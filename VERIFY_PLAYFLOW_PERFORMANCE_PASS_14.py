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

# Host setup state remains available, but Pass 29 may bypass the crash-prone live server-setup page.
for needle in ("StartHostedGameplay", "MaxPlayersEntry", "BotsEntry", "BotDifficultyEntry"):
    require(header, needle, "frontend host setup header")

pass29_static = 'PASS29_MAIN_START_DIRECT_HOST_QUEUED' in frontend
if pass29_static:
    for needle in (
        'if (Page == 0)', 'PASS29_MAIN_START_DIRECT_HOST_QUEUED',
        'PASS29_UNSAFE_FRONTEND_PAGE_TRANSITION_BLOCKED', 'PASS29_STATIC_FRONTEND_HOST_TRAVEL_EXECUTE',
        'void UOCR13FrontendMenuSubsystem::StartHostedGameplay()', 'PASS14_HOST_TRAVEL_BEGIN',
        '?listen?Mode=Conquest', '?PerfProfile=LowCPU?R13Gameplay=1',
    ):
        require(frontend, needle, "static safe server creation flow")
    for needle in ('PendingPage = 1;', 'PASS14_MAIN_START_OPENS_SERVER_SETUP'):
        forbid(frontend, needle, "Pass 29 must not restore crash-prone server-setup page")
else:
    for needle in (
        'if (Page == 0)', 'Page = 1;', 'PASS14_MAIN_START_OPENS_SERVER_SETUP',
        '"HostTitle", "СТВОРЕННЯ СЕРВЕРА"', '"CreateServer", "СТВОРИТИ СЕРВЕР"',
        'void UOCR13FrontendMenuSubsystem::StartHostedGameplay()', 'PASS14_HOST_TRAVEL_BEGIN',
        '?listen?Mode=Conquest', '?PerfProfile=LowCPU?R13Gameplay=1',
    ):
        require(frontend, needle, "explicit server creation flow")

forbid(frontend, "LocationTest=1", "normal frontend must not open technical LocationTest")
forbid(frontend, "AutoDeploy=1", "normal frontend must not bypass deployment")
forbid(frontend, "StartLocalGameplay", "obsolete direct local-start helper")

for needle in (
    'PC->GetNetMode() != NM_Standalone', 'PC->IsDeploymentPanelVisible()',
    'PC->UIToggleFrontend();', 'PASS14_FRONTEND_TRAVEL_HANDOFF_READY',
):
    require(frontend, needle, "post-travel frontend handoff")

for needle in (
    'if (State && !State->IsBotPlayer() && !State->IsLobbyReady())',
    'humans stay controller-only while choosing team/squad/role/spawn in Deployment UI',
):
    require(game_mode, needle, "deployment authority gate")

# Block 0 supersedes the old single GridStep/CellsPerBatch contract. Full and LowCPU cover the same compact
# 960x940m area; LowCPU is cheaper through a coarser grid and shorter cull distances, not by cropping Oster.
full_grid = re.search(r'constexpr\s+float\s+FullGridStepCm\s*=\s*([0-9.]+)f\s*;', foliage)
low_grid = re.search(r'constexpr\s+float\s+LowCPUGridStepCm\s*=\s*([0-9.]+)f\s*;', foliage)
full_batch = re.search(r'constexpr\s+int32\s+FullCellsPerBatch\s*=\s*(\d+)\s*;', foliage)
low_batch = re.search(r'constexpr\s+int32\s+LowCPUCellsPerBatch\s*=\s*(\d+)\s*;', foliage)
if not full_grid or float(full_grid.group(1)) < 1000.0:
    raise SystemExit("PASS14 VERIFY FAIL: Full foliage grid is denser than the Block0 recovery floor")
if not low_grid or float(low_grid.group(1)) < 1500.0:
    raise SystemExit("PASS14 VERIFY FAIL: LowCPU foliage grid is denser than the recovery floor")
if float(low_grid.group(1)) <= float(full_grid.group(1)):
    raise SystemExit("PASS14 VERIFY FAIL: LowCPU foliage grid must be coarser than Full")
if not full_batch or not 1 <= int(full_batch.group(1)) <= 32:
    raise SystemExit("PASS14 VERIFY FAIL: Full foliage batch exceeds the recovery CPU ceiling")
if not low_batch or not 1 <= int(low_batch.group(1)) <= 48:
    raise SystemExit("PASS14 VERIFY FAIL: LowCPU foliage batch exceeds the Block0 recovery CPU ceiling")
for needle in (
    'CompactMinX = -78000.0f', 'CompactMaxX =  18000.0f',
    'CompactMinY = -12000.0f', 'CompactMaxY =  82000.0f',
    'PopulationMinX = CompactMinX', 'PopulationMaxX = CompactMaxX',
    'PopulationMinY = CompactMinY', 'PopulationMaxY = CompactMaxY',
    'ActiveGridStep = bLowCPUProfile ? LowCPUGridStepCm : FullGridStepCm',
    'ActiveCellsPerBatch = bLowCPUProfile ? LowCPUCellsPerBatch : FullCellsPerBatch',
    'UHierarchicalInstancedStaticMeshComponent', 'SetCollisionEnabled(ECollisionEnabled::NoCollision)',
    'SetCastShadow(false)', 'PopulateBatch', 'DenseGrass_',
    'PASS45_BLOCK0_FULL_MAP_GRASS_SCOPE_READY', 'full_playable_bounds=1', 'museum_only=0',
):
    require(foliage, needle, "Block0 foliage recovery")
for forbidden in (
    'constexpr float GridStep = 900.0f',
    'constexpr int32 CellsPerBatch = 88',
    'LowCPUHalfExtentCm',
    'full_sector_population=0',
):
    forbid(foliage, forbidden, "retired foliage recovery contract")

for needle in (
    'SetDynamicShadowCascades(4)', 'SetDynamicShadowDistanceMovableLight(18000.0f)',
    'SetRealTimeCaptureEnabled(false)', 'PASS14_RENDER_BUDGET_READY',
):
    require(env, needle, "daylight render budget")
forbid(env, 'SetRealTimeCaptureEnabled(true)', "continuous skylight capture")
forbid(env, 'SetDynamicShadowDistanceMovableLight(30000.0f)', "old 300 m movable shadow range")

# Pass 15 may add an adaptive probe/emergency profile, but Pass 14 evidence markers remain for compatibility.
for needle in (
    'UOCPerformanceSampleSubsystem : public UTickableWorldSubsystem',
    'WarmupSeconds', 'SampleSeconds', 'WorstFrameSeconds',
    'bRecoveryRuntimeContractLogged', 'ValidatePass45RecoveryRuntimeContract',
):
    require(perf_h, needle, "performance sampler header")
for needle in (
    'PC->GetPawn() == nullptr', 'PASS14_PERF_SAMPLE', 'PASS14_PERF_BELOW_TARGET',
    'PASS14_PERF_30FPS_READY', 'AverageFps < 30.0f',
):
    require(perf, needle, "performance sampler compatibility")

# Gate C/H: the launcher request is insufficient. Once a real gameplay pawn is possessed, UE itself must read
# the live t.MaxFPS CVar and live GameViewportClient fullscreen state and emit fail-visible evidence.
for needle in (
    '#include "HAL/IConsoleManager.h"', '#include "Engine/GameViewportClient.h"',
    'FindConsoleVariable(TEXT("t.MaxFPS"))', 'MaxFpsVariable->GetFloat()',
    'FMath::IsNearlyEqual(RuntimeMaxFps, 60.0f, 0.5f)',
    'ViewportClient->IsFullScreenViewport()',
    'PASS45_THERMAL_CAP_RUNTIME_READY', 'PASS45_THERMAL_CAP_RUNTIME_FAIL',
    'PASS45_FULLSCREEN_RUNTIME_READY', 'PASS45_FULLSCREEN_RUNTIME_FAIL',
    'ValidatePass45RecoveryRuntimeContract();',
):
    require(perf, needle, "Pass45 live thermal/display runtime contract")

for needle in ('set "RECOVERY_PROJECT_DIR=%~dp0."', '-ProjectDir "%RECOVERY_PROJECT_DIR%"'):
    require(importer, needle, "production source recovery path")

for needle in ('call "%PRODUCTION_IMPORT%"', 'if errorlevel 1 (', 'exit /b 20'):
    require(main_launcher, needle, "production importer fail-closed launcher")
for needle in ('-fullscreen', 't.MaxFPS 60'):
    require(main_launcher, needle, "Pass45 recovery display/thermal request")
forbid(main_launcher, '-windowed', "normal route must not force windowed mode")

runtime_markers = [
    'RUN_R14_CURRENT_GAMEPLAY.cmd', 'PASS14_HOST_TRAVEL_BEGIN',
    'PASS14_FRONTEND_TRAVEL_HANDOFF_READY', 'PASS14_PERF_SAMPLE',
    'PASS14_PERF_BELOW_TARGET', 'PASS14_PERF_30FPS_READY', 'R14_CURRENT_GAMEPLAY.log',
]
if pass29_static:
    runtime_markers.append('PASS29_STATIC_FRONTEND_HOST_TRAVEL_EXECUTE')
else:
    runtime_markers.append('PASS14_MAIN_START_OPENS_SERVER_SETUP')
for needle in runtime_markers:
    require(runtime_launcher, needle, "Pass 14 runtime launcher")

print("PLAYFLOW + PERFORMANCE PASS 14 SOURCE CONTRACT PASS")
if pass29_static:
    print("- Pass 29 static START replaces the disproven live server-setup page while preserving hosted travel and Deployment ownership")
else:
    print("- explicit server setup and Deployment ownership remain intact")
print("- Block0 Full/LowCPU foliage share the compact Oster bounds; LowCPU uses a coarser grid and shorter culls instead of a spatial crop")
print("- Full batch stays <=32 cells and LowCPU <=48 cells while generation remains incremental at 50 ms cadence")
print("- Pass 14 FPS evidence markers remain compatible with adaptive recovery")
print("- Pass45 Gate C/H distinguishes launcher request from live UE t.MaxFPS/fullscreen viewport evidence")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 runtime acceptance still required")
