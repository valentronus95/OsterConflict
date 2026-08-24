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

require(start, '2. ПОВНИЙ RUNTIME-ТЕСТ', "START_HERE full-test label")
require(start, 'call "%~dp0RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd"', "START_HERE full-test route")

for needle in (
    "PASS 29-40 RUNTIME ACCEPTANCE",
    'set "OC_FORCE_ACCEPTANCE=1"',
    'call "%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"',
    'set "LOG=%~dp0Logs\\R14_CURRENT_GAMEPLAY.log"',
    "SPAWN обов'язково виберіть BASE біля музею",
    "WASD + mouse",
    "Натисніть M один раз",
    "weapon pickups",
    "не менше 20 секунд",
):
    require(launcher, needle, "Pass 29-40 acceptance flow")

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
    "PASS38_MUSEUM_REBUILD_BUDGET_READY",
    "PASS38_WEAPON_FALLBACK_SCAN_STOPPED",
    "PASS38_WEAPON_PALETTE_SCAN_STOPPED",
    "PASS39_GRAPHICS_QUALITY_PROFILE_READY",
    "PASS39_MINIMAP_UPDATE_BUDGET_READY",
    "PASS39_FP_LOCAL_PAWN_FAST_PATH_READY",
    "PASS39_PERF_SAMPLER_IDLE_READY",
    "PASS40_UI_STABILIZER_BUDGET_READY",
    "PASS40_DEPLOYMENT_PRESENTATION_BUDGET_READY",
    "PASS14_PERF_SAMPLE",
    "PASS14_PERF_30FPS_READY",
):
    require(launcher, marker, f"current runtime evidence {marker}")

require(launcher,
        'findstr /C:"PASS31_GAMEPLAY_INPUT_READY" "%LOG%" | findstr /C:"moveIgnored=0 lookIgnored=0" >nul',
        "released gameplay input state")

for failure_marker in (
    "PASS29_UNSAFE_FRONTEND_PAGE_TRANSITION_BLOCKED",
    "PASS37_BASE_DEPLOYMENT_RECOVERY_FAIL",
    "PASS32_MUSEUM_LAYER_BUDGET_FAIL",
    "PASS37_MUSEUM_VISIBLE_CORE_FAIL",
    "PASS38_MUSEUM_REBUILD_BUDGET_FAIL",
    "PASS38_WEAPON_FALLBACK_SCAN_BOUNDED_STOP",
    "PASS38_WEAPON_PALETTE_SCAN_BOUNDED_STOP",
    "PASS15_EMERGENCY_PERF_PROFILE_APPLIED",
    "PASS10_FOLIAGE_RUNTIME_FAIL",
    "PASS14_PERF_BELOW_TARGET",
):
    require(launcher, failure_marker, f"fail-closed marker {failure_marker}")

for exit_code in (
    "exit /b 33", "exit /b 34", "exit /b 35", "exit /b 36", "exit /b 37", "exit /b 38",
    "exit /b 40", "exit /b 42", "exit /b 43", "exit /b 44", "exit /b 45", "exit /b 46"
):
    require(launcher, exit_code, f"distinct acceptance failure {exit_code}")

require(launcher, "30 FPS acceptance target", "explicit FPS floor")
forbid(launcher, "PASS14_PERF_30FPS_READY" + " >nul\nif not errorlevel 1", "30 FPS readiness must not be inverted")
require(launcher, "20-45 m museum approach", "closer Museum deployment band")

print("RUNTIME ACCEPTANCE PASS 33/35/36/37/38/39/40 SOURCE CONTRACT PASS")
print("- START_HERE full runtime test reaches the canonical normal-game launcher under acceptance mode")
print("- visible Museum structural core and a 20-45 m BASE deployment remain mandatory")
print("- Pass 38 requires bounded museum recovery and bounded weapon startup scans")
print("- Pass 39 requires stable graphics quality, budgeted minimap/local-pawn presentation and an idle completed sampler")
print("- Pass 40 requires bounded viewport/deployment presentation work instead of per-frame global root scans and Slate rewrites")
print("- stale hidden emergency graphics downgrade is a hard failure")
print("- LowCPU foliage must remain bounded and actual sampled gameplay must still reach >=30 FPS")
print("STATUS: SOURCE VERIFIED; the actual UE 5.8 run remains the runtime authority")