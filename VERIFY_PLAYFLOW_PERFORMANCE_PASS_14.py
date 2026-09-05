#!/usr/bin/env python3
from pathlib import Path
import re
from pass45_runtime_route_contract import ROOT, read, require, validate_runtime_route

SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"

def src(rel: str) -> str:
    return (SRC / rel).read_text(encoding="utf-8", errors="replace")

header = src("Public/OCR13FrontendMenuSubsystem.h")
frontend = src("Private/OCR13FrontendMenuSubsystem.cpp")
game_mode = src("Private/OCGameMode.cpp")
foliage = src("Private/OCDenseGroundFoliageSubsystem.cpp")
env = src("Private/OCVisualEnvironment.cpp")
perf_h = src("Public/OCPerformanceSampleSubsystem.h")
perf = src("Private/OCPerformanceSampleSubsystem.cpp")
importer = read("OsterConflict/IMPORT_PRODUCTION_VEHICLES_UE58.cmd")
route = validate_runtime_route()

for needle in ("StartHostedGameplay", "MaxPlayersEntry", "BotsEntry", "BotDifficultyEntry"):
    require(header, needle, "frontend host setup")
for needle in ("PASS29_MAIN_START_DIRECT_HOST_QUEUED", "PASS29_STATIC_FRONTEND_HOST_TRAVEL_EXECUTE", "PASS14_HOST_TRAVEL_BEGIN", "?listen?Mode=Conquest", "?PerfProfile=LowCPU?R13Gameplay=1"):
    require(frontend, needle, "static host flow")
for needle in ("PASS14_FRONTEND_TRAVEL_HANDOFF_READY", "PC->UIToggleFrontend();"):
    require(frontend, needle, "frontend handoff")
for needle in ("if (State && !State->IsBotPlayer() && !State->IsLobbyReady())", "humans stay controller-only while choosing team/squad/role/spawn in Deployment UI"):
    require(game_mode, needle, "deployment authority")

grid = re.search(r"constexpr\s+float\s+GridStep\s*=\s*([0-9.]+)f\s*;", foliage)
batch = re.search(r"constexpr\s+int32\s+CellsPerBatch\s*=\s*(\d+)\s*;", foliage)
if not grid or float(grid.group(1)) < 1500.0 or not batch or not 1 <= int(batch.group(1)) <= 32:
    raise SystemExit("PASS14 VERIFY FAIL: foliage CPU budget regressed")
for needle in ("SetDynamicShadowCascades(4)", "SetRealTimeCaptureEnabled(false)", "PASS14_RENDER_BUDGET_READY"):
    require(env, needle, "render budget")
for needle in ("UOCPerformanceSampleSubsystem : public UTickableWorldSubsystem", "WorstFrameSeconds"):
    require(perf_h, needle, "performance sampler header")
for needle in ("PASS14_PERF_SAMPLE", "PASS14_PERF_BELOW_TARGET", "PASS14_PERF_30FPS_READY", "AverageFps < 30.0f"):
    require(perf, needle, "performance sampler")
for needle in ('set "RECOVERY_PROJECT_DIR=%~dp0."', '-ProjectDir "%RECOVERY_PROJECT_DIR%"'):
    require(importer, needle, "production recovery path")
for needle in ("PASS14_HOST_TRAVEL_BEGIN", "PASS14_FRONTEND_TRAVEL_HANDOFF_READY", "PASS14_PERF_SAMPLE", "PASS14_PERF_BELOW_TARGET", "PASS14_PERF_30FPS_READY"):
    require(route["evidence"], needle, "central evidence")

print("PLAYFLOW + PERFORMANCE PASS 14 SOURCE CONTRACT PASS")
print("- START_HERE only delegates; the packet runner owns strict acceptance")
print("- playflow/performance evidence remains centralized and fail-closed")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 runtime acceptance still required")
