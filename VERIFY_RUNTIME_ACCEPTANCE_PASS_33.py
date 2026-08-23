#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
LAUNCHER = ROOT / "RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd"
START = ROOT / "START_HERE.cmd"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS33 VERIFY FAIL: missing {path.name}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        raise SystemExit(f"PASS33 VERIFY FAIL: {label}: missing {needle!r}")


def forbid(text: str, needle: str, label: str) -> None:
    if needle in text:
        raise SystemExit(f"PASS33 VERIFY FAIL: {label}: forbidden {needle!r}")


launcher = read(LAUNCHER)
start = read(START)

# START_HERE option 2 remains the single user-facing full-runtime entry point.
require(start, '2. ПОВНИЙ RUNTIME-ТЕСТ', "START_HERE full-test label")
require(start, 'call "%~dp0RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd"', "START_HERE full-test route")

# The compatibility verifier keeps the original Pass 33 contract and now also proves the later Pass 35/36
# evidence is part of the same one-click runtime path.
for needle in (
    "PASS 29-36 RUNTIME ACCEPTANCE",
    'set "OC_FORCE_ACCEPTANCE=1"',
    'call "%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"',
    'set "LOG=%~dp0Logs\\R14_CURRENT_GAMEPLAY.log"',
    "SPAWN обов'язково виберіть BASE біля музею",
    "WASD + mouse",
    "Натисніть M один раз",
    "weapon pickups",
    "не менше 16 секунд",
):
    require(launcher, needle, "Pass 29-36 acceptance flow")

# Preserve the proven frontend/travel/foliage evidence before evaluating newer museum/input/material gates.
for marker in (
    "PASS29_MAIN_START_DIRECT_HOST_QUEUED",
    "PASS29_STATIC_FRONTEND_HOST_TRAVEL_EXECUTE",
    "PASS14_HOST_TRAVEL_BEGIN",
    "PASS14_FRONTEND_TRAVEL_HANDOFF_READY",
    "PASS14_FOLIAGE_BUDGET_READY",
):
    require(launcher, marker, f"legacy runtime evidence {marker}")

for marker in (
    "PASS30_MUSEUM_SPECULATIVE_INTERIOR_REMOVED",
    "PASS30_MUSEUM_WINDOW_FRAME_CLEAN_READY",
    "PASS30_BASE_DEPLOYMENT_OUTSIDE_MUSEUM",
    "PASS31_GAMEPLAY_INPUT_READY",
    "PASS32_MUSEUM_LAYER_BUDGET_READY",
    "PASS35_MUSEUM_CORE_READY",
    "PASS35_MUSEUM_BASE_DISTANCE_READY",
    "PASS35_TACTICAL_PLAYER_MARKER_FOREGROUND",
    "PASS36_LOWCPU_FOLIAGE_SCOPE_READY",
    "PASS36_LOWCPU_FOLIAGE_RUNTIME_READY",
    "PASS36_WEAPON_MATERIAL_AUDIT_READY",
    "PASS14_PERF_SAMPLE",
    "PASS14_PERF_30FPS_READY",
):
    require(launcher, marker, f"runtime evidence {marker}")

# A marker alone is not enough for input. The emitted state must explicitly show that both stacks are clear.
require(launcher,
        'findstr /C:"PASS31_GAMEPLAY_INPUT_READY" "%LOG%" | findstr /C:"moveIgnored=0 lookIgnored=0" >nul',
        "released gameplay input state")

# The wrapper must fail closed if any known museum/spawn/foliage/performance failure is recorded.
for failure_marker in (
    "PASS29_UNSAFE_FRONTEND_PAGE_TRANSITION_BLOCKED",
    "PASS30_BASE_DEPLOYMENT_RECOVERY_FAIL",
    "PASS32_MUSEUM_LAYER_BUDGET_FAIL",
    "PASS35_MUSEUM_CORE_FAIL",
    "PASS35_MUSEUM_BASE_DISTANCE_FAIL",
    "PASS10_FOLIAGE_RUNTIME_FAIL",
    "PASS14_PERF_BELOW_TARGET",
):
    require(launcher, failure_marker, f"fail-closed marker {failure_marker}")

for exit_code in (
    "exit /b 33", "exit /b 34", "exit /b 35", "exit /b 36", "exit /b 37", "exit /b 38",
    "exit /b 40", "exit /b 41", "exit /b 42"
):
    require(launcher, exit_code, f"distinct acceptance failure {exit_code}")

# Never weaken performance acceptance just to make a bad runtime look green. Humanity has enough dashboards like that.
require(launcher, "30 FPS acceptance target", "explicit FPS floor")
forbid(launcher, "PASS14_PERF_30FPS_READY" + " >nul\nif not errorlevel 1", "30 FPS readiness must not be inverted")

print("RUNTIME ACCEPTANCE PASS 33/35/36 SOURCE CONTRACT PASS")
print("- START_HERE full runtime test reaches the canonical normal-game launcher under acceptance mode")
print("- exterior near-museum BASE, visible museum core/map marker, released gameplay input and weapon material audit are mandatory")
print("- LowCPU foliage must remain bounded and actual sampled gameplay must still reach >=30 FPS")
print("- known frontend/spawn/layer/foliage/performance failure markers fail closed")
print("STATUS: SOURCE VERIFIED; the actual UE 5.8 run remains the runtime authority")
