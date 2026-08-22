#!/usr/bin/env python3
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"

FRONTEND_H = SRC / "Public" / "OCR13FrontendMenuSubsystem.h"
FRONTEND = SRC / "Private" / "OCR13FrontendMenuSubsystem.cpp"
GAME_MODE = SRC / "Private" / "OCGameMode.cpp"
FOLIAGE = SRC / "Private" / "OCDenseGroundFoliageSubsystem.cpp"
ENV = SRC / "Private" / "OCVisualEnvironment.cpp"
PERF_H = SRC / "Public" / "OCPerformanceSampleSubsystem.h"
PERF = SRC / "Private" / "OCPerformanceSampleSubsystem.cpp"
IMPORTER = ROOT / "OsterConflict" / "IMPORT_PRODUCTION_VEHICLES_UE58.cmd"
MAIN_LAUNCHER = ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd"
RUNTIME_LAUNCHER = ROOT / "RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS14 VERIFY FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(text: str, needle: str, where: str) -> None:
    if needle not in text:
        raise SystemExit(f"PASS14 VERIFY FAIL: {where}: missing {needle!r}")


def forbid(text: str, needle: str, where: str) -> None:
    if needle in text:
        raise SystemExit(f"PASS14 VERIFY FAIL: {where}: forbidden {needle!r}")


header = read(FRONTEND_H)
frontend = read(FRONTEND)
game_mode = read(GAME_MODE)
foliage = read(FOLIAGE)
env = read(ENV)
perf_h = read(PERF_H)
perf = read(PERF)
importer = read(IMPORTER)
main_launcher = read(MAIN_LAUNCHER)
runtime_launcher = read(RUNTIME_LAUNCHER)

# Main START is navigation, not an implicit local/test match. Hosting has its own explicit confirmation.
for needle in (
    "StartHostedGameplay",
    "MaxPlayersEntry",
    "BotsEntry",
    "BotDifficultyEntry",
):
    require(header, needle, "frontend host setup header")
for needle in (
    'if (Page == 0)',
    'Page = 1;',
    'PASS14_MAIN_START_OPENS_SERVER_SETUP',
    '"HostTitle", "СТВОРЕННЯ СЕРВЕРА"',
    '"CreateServer", "СТВОРИТИ СЕРВЕР"',
    'void UOCR13FrontendMenuSubsystem::StartHostedGameplay()',
    'PASS14_HOST_TRAVEL_BEGIN',
    '?listen?Mode=Conquest',
    '?PerfProfile=LowCPU?R13Gameplay=1',
):
    require(frontend, needle, "explicit server creation flow")
forbid(frontend, "LocationTest=1", "normal frontend must not open technical LocationTest")
forbid(frontend, "AutoDeploy=1", "normal frontend must not bypass deployment")
forbid(frontend, "StartLocalGameplay", "obsolete direct local-start helper")

# -Frontend remains a startup-shell flag only. Listen/client travel must hand control to Deployment.
for needle in (
    'PC->GetNetMode() != NM_Standalone',
    'PC->IsDeploymentPanelVisible()',
    'PC->UIToggleFrontend();',
    'PASS14_FRONTEND_TRAVEL_HANDOFF_READY',
):
    require(frontend, needle, "post-travel frontend handoff")

# GameMode must still keep a human controller pawn-less until the deployment is committed.
for needle in (
    'if (State && !State->IsBotPlayer() && !State->IsLobbyReady())',
    'humans stay controller-only while choosing team/squad/role/spawn in Deployment UI',
):
    require(game_mode, needle, "deployment authority gate")

# Emergency foliage budget after the 4-7 FPS playtest.
grid = re.search(r'constexpr\s+float\s+GridStep\s*=\s*([0-9.]+)f\s*;', foliage)
batch = re.search(r'constexpr\s+int32\s+CellsPerBatch\s*=\s*(\d+)\s*;', foliage)
if not grid or float(grid.group(1)) < 1500.0:
    raise SystemExit("PASS14 VERIFY FAIL: foliage grid is still too dense for the emergency budget")
if not batch or int(batch.group(1)) > 32:
    raise SystemExit("PASS14 VERIFY FAIL: foliage batch still exceeds the emergency CPU budget")
for needle in (
    'RandomStream.RandRange(2, 3)',
    'TEXT("DenseGrass_%d"), Index)), 16000',
    'TEXT("DenseGroundPlants"), 12000)',
    'TEXT("DenseFlowers"), 10000)',
    'RandomStream.FRand() < 0.12f',
    'RandomStream.FRand() < 0.025f',
    'PASS14_FOLIAGE_BUDGET_READY',
):
    require(foliage, needle, "foliage budget")
forbid(foliage, 'constexpr float GridStep = 900.0f', "old 9 m foliage grid")
forbid(foliage, 'constexpr int32 CellsPerBatch = 88', "old 88-cell foliage batch")

# Daylight stays visually compatible but removes continuous skylight recapture and excessive distant CSM work.
for needle in (
    'SetDynamicShadowCascades(4)',
    'SetDynamicShadowDistanceMovableLight(18000.0f)',
    'SetRealTimeCaptureEnabled(false)',
    'PASS14_RENDER_BUDGET_READY',
):
    require(env, needle, "daylight render budget")
forbid(env, 'SetRealTimeCaptureEnabled(true)', "continuous skylight capture")
forbid(env, 'SetDynamicShadowDistanceMovableLight(30000.0f)', "old 300 m movable shadow range")

# Runtime FPS evidence starts only after a local gameplay pawn exists and records a stable 10s sample.
for needle in (
    'UOCPerformanceSampleSubsystem : public UTickableWorldSubsystem',
    'WarmupSeconds',
    'SampleSeconds',
    'WorstFrameSeconds',
):
    require(perf_h, needle, "performance sampler header")
for needle in (
    'PC->GetPawn() == nullptr',
    'WarmupSeconds < 5.0f',
    'SampleSeconds < 10.0f',
    'PASS14_PERF_SAMPLE',
    'PASS14_PERF_BELOW_TARGET',
    'PASS14_PERF_30FPS_READY',
    'AverageFps < 30.0f',
):
    require(perf, needle, "performance sampler")

# The user's production-source failure came from a quoted path ending in backslash. Use a quote-safe . form.
for needle in (
    'set "RECOVERY_PROJECT_DIR=%~dp0."',
    '-ProjectDir "%RECOVERY_PROJECT_DIR%"',
):
    require(importer, needle, "production source recovery path")

# Normal launcher must fail closed if the production importer still fails.
for needle in (
    'call "%PRODUCTION_IMPORT%"',
    'if errorlevel 1 (',
    'exit /b 20',
):
    require(main_launcher, needle, "production importer fail-closed launcher")

# Dedicated runtime acceptance checks the user-visible flow and measured performance.
for needle in (
    'RUN_R14_CURRENT_GAMEPLAY.cmd',
    'PASS14_MAIN_START_OPENS_SERVER_SETUP',
    'PASS14_HOST_TRAVEL_BEGIN',
    'PASS14_FRONTEND_TRAVEL_HANDOFF_READY',
    'PASS14_FOLIAGE_BUDGET_READY',
    'PASS14_PERF_SAMPLE',
    'PASS14_PERF_BELOW_TARGET',
    'PASS14_PERF_30FPS_READY',
    'R14_CURRENT_GAMEPLAY.log',
):
    require(runtime_launcher, needle, "Pass 14 runtime launcher")

print("PLAYFLOW + PERFORMANCE PASS 14 SOURCE CONTRACT PASS")
print("- main START opens explicit server setup instead of LocationTest/listen gameplay")
print("- host creation exposes MaxPlayers/Bots/BotDifficulty and normal Deployment remains authoritative")
print("- post-travel -Frontend resurrection is suppressed before Deployment")
print("- dense foliage CPU/instance/draw-distance budgets were reduced after the 4-7 FPS playtest")
print("- realtime skylight capture and 300 m movable-shadow range are retired")
print("- a 5s warmup + 10s gameplay sample records average/worst-frame FPS and a 30 FPS readiness marker")
print("- production model source recovery now receives a quote-safe project directory")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 compile and measured runtime FPS still required")
