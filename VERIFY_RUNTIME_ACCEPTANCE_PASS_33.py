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

# Pass 37 supersedes only evidence that latest user runtime disproved. Keep one canonical acceptance path.
for needle in (
    "PASS 29-37 RUNTIME ACCEPTANCE",
    'set "OC_FORCE_ACCEPTANCE=1"',
    'call "%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"',
    'set "LOG=%~dp0Logs\\R14_CURRENT_GAMEPLAY.log"',
    "SPAWN обов'язково виберіть BASE біля музею",
    "WASD + mouse",
    "Натисніть M один раз",
    "weapon pickups",
    "не менше 20 секунд",
):
    require(launcher, needle, "Pass 29-37 acceptance flow")

# Preserve proven frontend/travel/startup evidence.
for marker in (
    "PASS29_MAIN_START_DIRECT_HOST_QUEUED",
    "PASS29_STATIC_FRONTEND_HOST_TRAVEL_EXECUTE",
    "PASS14_HOST_TRAVEL_BEGIN",
    "PASS14_FRONTEND_TRAVEL_HANDOFF_READY",
    "PASS14_FOLIAGE_BUDGET_READY",
):
    require(launcher, marker, f"legacy runtime evidence {marker}")

# Current runtime acceptance must prove what the latest screenshots actually rejected: a real visible
# museum, the closer BASE, the map marker, released input, non-blank weapon presentation and bounded LowCPU foliage.
for marker in (
    "PASS30_MUSEUM_SPECULATIVE_INTERIOR_REMOVED",
    "PASS30_MUSEUM_WINDOW_FRAME_CLEAN_READY",
    "PASS31_GAMEPLAY_INPUT_READY",
    "PASS32_MUSEUM_LAYER_BUDGET_READY",
    "PASS35_TACTICAL_PLAYER_MARKER_FOREGROUND",
    "PASS36_LOWCPU_FOLIAGE_SCOPE_READY",
    "PASS36_LOWCPU_FOLIAGE_RUNTIME_READY",
    "PASS36_WEAPON_MATERIAL_AUDIT_READY",
    "PASS37_MUSEUM_VISIBLE_CORE_READY",
    "PASS37_MUSEUM_VISIBLE_BASES_READY",
    "PASS37_BASE_DEPLOYMENT_VISIBLE_MUSEUM_APPROACH",
    "PASS37_WEAPON_VISIBLE_PALETTE_READY",
    "PASS14_PERF_SAMPLE",
    "PASS14_PERF_30FPS_READY",
):
    require(launcher, marker, f"current runtime evidence {marker}")

# A marker alone is not enough for input. The emitted state must explicitly show that both stacks are clear.
require(launcher,
        'findstr /C:"PASS31_GAMEPLAY_INPUT_READY" "%LOG%" | findstr /C:"moveIgnored=0 lookIgnored=0" >nul',
        "released gameplay input state")

# The wrapper must fail closed on current known regressions. Pass 35 owner-count/base-distance failures are
# deliberately no longer authoritative: user runtime proved owner counts and the old 30-60 m band could be green
# while the Museum was not visible. Pass 37 visible components + 20-45 m deployment are the stronger contract.
for failure_marker in (
    "PASS29_UNSAFE_FRONTEND_PAGE_TRANSITION_BLOCKED",
    "PASS37_BASE_DEPLOYMENT_RECOVERY_FAIL",
    "PASS32_MUSEUM_LAYER_BUDGET_FAIL",
    "PASS37_MUSEUM_VISIBLE_CORE_FAIL",
    "PASS10_FOLIAGE_RUNTIME_FAIL",
    "PASS14_PERF_BELOW_TARGET",
):
    require(launcher, failure_marker, f"fail-closed marker {failure_marker}")

for exit_code in (
    "exit /b 33", "exit /b 34", "exit /b 35", "exit /b 36", "exit /b 37", "exit /b 38",
    "exit /b 40", "exit /b 42"
):
    require(launcher, exit_code, f"distinct acceptance failure {exit_code}")

# Never weaken performance acceptance just to make a bad runtime look green.
require(launcher, "30 FPS acceptance target", "explicit FPS floor")
forbid(launcher, "PASS14_PERF_30FPS_READY" + " >nul\nif not errorlevel 1", "30 FPS readiness must not be inverted")
require(launcher, "20-45 m museum approach", "Pass 37 closer Museum deployment band")

print("RUNTIME ACCEPTANCE PASS 33/35/36/37 SOURCE CONTRACT PASS")
print("- START_HERE full runtime test reaches the canonical normal-game launcher under acceptance mode")
print("- visible Museum structural core and a 20-45 m BASE deployment replace disproven owner-count/distance-only evidence")
print("- tactical player marker, released input and visible weapon palette are mandatory")
print("- LowCPU foliage must remain bounded and actual sampled gameplay must still reach >=30 FPS")
print("- known current frontend/spawn/layer/foliage/performance failure markers fail closed")
print("STATUS: SOURCE VERIFIED; the actual UE 5.8 run remains the runtime authority")
