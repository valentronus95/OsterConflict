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
ENTRY = ROOT / "START_HERE.cmd"
EVIDENCE = ROOT / "VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py"


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
entry = read(ENTRY)
evidence = read(EVIDENCE)

for needle in ("StartHostedGameplay", "MaxPlayersEntry", "BotsEntry", "BotDifficultyEntry"):
    require(header, needle, "frontend host setup header")

pass29_static = "PASS29_MAIN_START_DIRECT_HOST_QUEUED" in frontend
if pass29_static:
    for needle in (
        "PASS29_MAIN_START_DIRECT_HOST_QUEUED",
        "PASS29_STATIC_FRONTEND_HOST_TRAVEL_EXECUTE",
        "PASS14_HOST_TRAVEL_BEGIN",
        "?listen?Mode=Conquest",
        "?PerfProfile=LowCPU?R13Gameplay=1",
    ):
        require(frontend, needle, "static safe server creation flow")
else:
    for needle in ("PASS14_MAIN_START_OPENS_SERVER_SETUP", "PASS14_HOST_TRAVEL_BEGIN", "?listen?Mode=Conquest"):
        require(frontend, needle, "server creation flow")

forbid(frontend, "LocationTest=1", "normal frontend")
forbid(frontend, "AutoDeploy=1", "normal frontend")
for needle in ("PASS14_FRONTEND_TRAVEL_HANDOFF_READY", "PC->UIToggleFrontend();"):
    require(frontend, needle, "post-travel frontend handoff")
for needle in (
    "if (State && !State->IsBotPlayer() && !State->IsLobbyReady())",
    "humans stay controller-only while choosing team/squad/role/spawn in Deployment UI",
):
    require(game_mode, needle, "deployment authority gate")

grid = re.search(r"constexpr\s+float\s+GridStep\s*=\s*([0-9.]+)f\s*;", foliage)
batch = re.search(r"constexpr\s+int32\s+CellsPerBatch\s*=\s*(\d+)\s*;", foliage)
if not grid or float(grid.group(1)) < 1500.0:
    raise SystemExit("PASS14 VERIFY FAIL: foliage grid is too dense")
if not batch or not 1 <= int(batch.group(1)) <= 32:
    raise SystemExit("PASS14 VERIFY FAIL: foliage batch exceeds recovery CPU ceiling")
for needle in ("UHierarchicalInstancedStaticMeshComponent", "PopulateBatch", "DenseGrass_"):
    require(foliage, needle, "foliage recovery")

for needle in ("SetDynamicShadowCascades(4)", "SetRealTimeCaptureEnabled(false)", "PASS14_RENDER_BUDGET_READY"):
    require(env, needle, "daylight render budget")
for needle in ("UOCPerformanceSampleSubsystem : public UTickableWorldSubsystem", "WorstFrameSeconds"):
    require(perf_h, needle, "performance sampler header")
for needle in ("PASS14_PERF_SAMPLE", "PASS14_PERF_BELOW_TARGET", "PASS14_PERF_30FPS_READY", "AverageFps < 30.0f"):
    require(perf, needle, "performance sampler")

for needle in ('set "RECOVERY_PROJECT_DIR=%~dp0."', '-ProjectDir "%RECOVERY_PROJECT_DIR%"'):
    require(importer, needle, "production source recovery path")
for needle in ('call "%PRODUCTION_IMPORT%"', 'exit /b 20'):
    require(main_launcher, needle, "production importer fail-closed launcher")

for needle in (
    "Єдиний користувацький launcher/test entrypoint: START_HERE.cmd.",
    'set "OC_FORCE_ACCEPTANCE=1"',
    'call "%CURRENT_GAMEPLAY%"',
    'call "%MATERIAL_GATE%"',
    "VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py",
):
    require(entry, needle, "single START_HERE runtime test route")
for forbidden in ("RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd", "RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd", "TRY_PRODUCTION_VEHICLES_UE58.cmd"):
    forbid(entry, forbidden, "single START_HERE runtime test route")

for marker in (
    "PASS14_HOST_TRAVEL_BEGIN",
    "PASS14_FRONTEND_TRAVEL_HANDOFF_READY",
    "PASS14_PERF_SAMPLE",
    "PASS14_PERF_BELOW_TARGET",
    "PASS14_PERF_30FPS_READY",
):
    require(evidence, marker, "central runtime evidence verifier")

print("PLAYFLOW + PERFORMANCE PASS 14 SOURCE CONTRACT PASS")
print("- START_HERE.cmd is the single user-facing runtime test entrypoint")
print("- playflow/performance evidence is centralized in VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 runtime acceptance still required")
