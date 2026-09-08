#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
MAIN = ROOT / "RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd"
LAUNCHER = ROOT / "RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd"
START = ROOT / "START_HERE.cmd"
BATCH_CMD = ROOT / "OsterConflict" / "PASS45_BATCH_RUNTIME.cmd"
BATCH_ENTRY = ROOT / "OsterConflict" / "Scripts" / "pass45_batch_runtime_progress_entry.py"
MANUAL_ACTION = ROOT / "VERIFY_PASS45_MANUAL_ACTION_RUNTIME.py"


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


main = read(MAIN)
launcher = read(LAUNCHER)
start = read(START)
batch_cmd = read(BATCH_CMD)
batch_entry = read(BATCH_ENTRY)
manual_action = read(MANUAL_ACTION)

# Full runtime remains batch-first so one gameplay process supplies one coherent evidence set.
require(start, '2. ПОВНИЙ RUNTIME-ТЕСТ', "START_HERE full-test label")
require(start, 'call "%~dp0OsterConflict\\PASS45_BATCH_RUNTIME.cmd"', "START_HERE batch full-test route")
forbid(start, 'call "%~dp0RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd"', "obsolete direct strict-main full-test route")
forbid(start, 'call "%~dp0RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd"', "obsolete direct playflow full-test bypass")
require(batch_cmd, 'pass45_batch_runtime_progress_entry.py', "batch command -> progress/runtime entry")
require(batch_entry, 'pass45_batch_runtime_progress as progress', "batch progress entry")
require(batch_entry, 'pass45_batch_runtime_runtimefix.py', "batch runtime-fix orchestrator")

# Focused strict-main route remains source-valid for diagnosis, not as a competing source of truth.
require(main, 'RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd', "strict main -> playflow identity")
require(main, 'call "%PLAYFLOW%"', "strict main -> playflow call")
require(main, 'RUN_PASS45_STRICT_MATERIAL_GATE.cmd', "strict post-game material gate")
require(main, 'VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py', "strict post-game evidence verifier")
require(main, 'VERIFY_PASS45_MANUAL_ACTION_RUNTIME.py', "strict item16 manual-action verifier")
require(main, '%PY_CMD% "%MANUAL_ACTION_VERIFY%" "%GAMEPLAY_LOG%"', "manual-action exact-run log gate")
require(main, 'VISUAL ACCEPTANCE IS STILL PENDING', "manual visual acceptance remains pending")

for needle in (
    "OC_SNP1",
    "OC_SG1",
    "R13_LEVER4570",
    "PASS45_MANUAL_ACTION_CYCLE_READY",
    "PASS45_WEAPON_AUDIO_FALLBACK_READY",
    "PASS45_MANUAL_ACTION_AUTHORED_SOURCE_BRIDGE_READY",
    "PASS45_WEAPON_AUDIO_CONTENT_GAP",
    "PASS45_MANUAL_ACTION_AUTHORED_CONTENT_GAP",
    "PASS45_MANUAL_ACTION_AUTHORED_SOURCE_BRIDGE_FAIL",
):
    require(manual_action, needle, f"manual-action runtime gate contract {needle}")

# Current user/runtime authority: spawn must not be denied by visual preparation failures, local/listen matches
# restore filler bots only after the human host exists, and wrong-identity weapon substitution is rejected.
for needle in (
    "OSTER CONFLICT - PASS 45 CURRENT RUNTIME ACCEPTANCE",
    'set "OC_FORCE_ACCEPTANCE=1"',
    'call "%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"',
    'set "LOG=%~dp0Logs\\R14_CURRENT_GAMEPLAY.log"',
    "Реальний pawn має опинитися біля Museum",
    "compact central Oster",
    "Normal local/listen run після появи human host має відновити filler bots",
    "Exact production/local weapon identity is required",
    "11 weapon pickups",
    "HMMWV/M2/BTR",
    "не менше 20 секунд",
):
    require(launcher, needle, "current Pass45 acceptance flow")

for marker in (
    "PASS45_SECONDARY_MENU_HOST_SETUP_QUEUED",
    "PASS14_MAIN_START_OPENS_SERVER_SETUP",
    "PASS14_HOST_TRAVEL_BEGIN",
    "PASS45_SECONDARY_MENU_HOST_TRAVEL_EXECUTE",
    "PASS14_FRONTEND_TRAVEL_HANDOFF_READY",
    "GAME_RECOVERY_BOT_FILL_RESTORED",
    "PASS44_PRIMARY_WORLD_COMPACT_AUTHORING_READY",
    "PASS44_RUNTIME_GAMEPLAY_SEEDS_COMPACT_READY",
    "PASS44_BASE_ROLE_COORDINATE_INDEPENDENT_READY",
    "PASS44_COMBAT_VEHICLE_SEEDS_COMPACT_READY",
    "PASS44_COMPACT_PLAYABLE_AREA_READY",
    "PASS44_TACTICAL_MAP_COMPACT_BOUNDS_READY",
    "PASS44_ACTUAL_PAWN_MUSEUM_BASE_READY",
    "GAME_RECOVERY_WORLD_READY",
    "PASS45_MUSEUM_R137_VISIBLE_OWNER_PRESERVED",
    "PASS45_MUSEUM_R138_COLLISION_ONLY_READY",
    "PASS45_MUSEUM_SINGLE_VISIBLE_OWNER_READY",
    "PASS45_MUSEUM_LAYER_VALIDATION_READY",
    "PASS14_FOLIAGE_BUDGET_READY",
    "PASS30_MUSEUM_WINDOW_FRAME_CLEAN_READY",
    "PASS37_MUSEUM_VISIBLE_BASES_READY",
    "PASS42_BASE_RACK_GROUNDED_READY",
    "PASS45_REQUIRED_AVAILABLE_WEAPONS_READY",
    "PASS45_VEHICLEBASE_PRODUCTION_MATERIAL_BYPASS_READY",
    "PASS45_PRODUCTION_VEHICLE_VISUALS_VALIDATED_READY",
    "PASS45_HMMWV_PROPORTIONAL_VISUAL_READY",
    "PASS45_BTR4_PROPORTIONAL_VISUAL_READY",
    "PASS45_M2_MOUNT_ALIGNMENT_READY",
    "PASS45_M2_GUNNER_PITCH_CONTRACT_READY",
    "PASS45_VEHICLE_ENTER_TRANSFORM_READY",
    "PASS45_VEHICLE_EXIT_TRANSFORM_READY",
    "PASS45_GUNNER_EXIT_TRANSFORM_READY",
    "PASS35_TACTICAL_PLAYER_MARKER_FOREGROUND",
    "PASS31_GAMEPLAY_INPUT_READY",
    "PASS41_INPUT_RECOVERY_POLL_BUDGET_READY",
    "PASS36_LOWCPU_FOLIAGE_SCOPE_READY",
    "PASS36_LOWCPU_FOLIAGE_RUNTIME_READY",
    "PASS36_WEAPON_MATERIAL_AUDIT_READY",
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

# Fail visibly on real runtime/content gaps. Bounded weapon scan exhaustion is a failure, not a success token.
for failure_marker in (
    "PASS44_ACTUAL_PAWN_MUSEUM_BASE_FAIL",
    "PASS44_COMPACT_PLAYABLE_AREA_FAIL",
    "PASS37_BASE_DEPLOYMENT_RECOVERY_FAIL",
    "PASS45_MUSEUM_SINGLE_VISIBLE_OWNER_FAIL",
    "PASS45_MUSEUM_R138_COLLISION_ONLY_FAIL",
    "PASS45_MUSEUM_LAYER_VALIDATION_FAIL",
    "PASS45_LANDMARK_SEPARATION_VALIDATION_FAIL",
    "PASS42_BASE_RACK_GROUNDING_INCOMPLETE",
    "PASS45_REQUIRED_AVAILABLE_WEAPON_RUNTIME_FAIL",
    "PASS44_WEAPON_RACK_AUTHORED_MATERIAL_GAP",
    "PASS45_PRODUCTION_VEHICLE_MATERIAL_OVERRIDE_FAIL",
    "PASS45_PRODUCTION_VEHICLE_MATERIAL_GAP",
    "PASS45_PRODUCTION_VEHICLE_CONTENT_GAP",
    "PASS45_VEHICLE_ENTER_TRANSFORM_FAIL",
    "PASS45_VEHICLE_EXIT_TRANSFORM_FAIL",
    "PASS45_GUNNER_EXIT_TRANSFORM_FAIL",
    "PASS38_WEAPON_FALLBACK_SCAN_BOUNDED_STOP",
    "PASS15_EMERGENCY_PERF_PROFILE_APPLIED",
    "PASS10_FOLIAGE_RUNTIME_FAIL",
    "PASS14_PERF_BELOW_TARGET",
):
    require(launcher, failure_marker, f"fail-visible marker {failure_marker}")

for exit_code in (
    "exit /b 33", "exit /b 34", "exit /b 35", "exit /b 36", "exit /b 37", "exit /b 38",
    "exit /b 39", "exit /b 42", "exit /b 44", "exit /b 46", "exit /b 47", "exit /b 48",
    "exit /b 49", "exit /b 50", "exit /b 51", "exit /b 52", "exit /b 53", "exit /b 54",
):
    require(launcher, exit_code, f"distinct acceptance failure {exit_code}")

require(launcher, "30 FPS acceptance target", "explicit FPS floor")

for forbidden in (
    "Normal local run не повинен сам запускати filler bots",
    "PASS44_LOCAL_BOT_AUTOFILL_DISABLED_READY",
    "PASS38_WEAPON_FALLBACK_SCAN_STOPPED",
    "PASS37_WEAPON_VISIBLE_PALETTE_READY",
    "PASS42_PRODUCTION_MATERIALS_RESTORED",
    "PASS42_PRODUCTION_VEHICLE_VISUALS_READY",
    "PASS30_MUSEUM_SPECULATIVE_INTERIOR_REMOVED",
    "PASS32_MUSEUM_LAYER_BUDGET_READY",
    "PASS32_MUSEUM_LAYER_BUDGET_FAIL",
    "PASS38_MUSEUM_REBUILD_BUDGET_READY",
    "PASS38_MUSEUM_REBUILD_BUDGET_FAIL",
    "PASS38_WEAPON_PALETTE_SCAN_STOPPED",
    "20-45 m museum approach",
):
    forbid(launcher, forbidden, f"retired compatibility marker {forbidden}")

print("RUNTIME ACCEPTANCE PASS 33 / PASS45 CURRENT CONTRACT PASS")
print("- START_HERE option 2 remains batch-first and keeps one coherent runtime evidence set")
print("- actual Museum pawn, compact Oster bounds, post-host filler bots and >=30 FPS remain mandatory")
print("- visual world-preparation acceptance cannot be used as permission to deny the human a pawn")
print("- exact weapon identity is required; generic look-alike fallback stays retired")
print("- authored weapon/vehicle material and manual-action content gaps fail visibly; no runtime disguise is accepted")
print("STATUS: SOURCE CONTRACT ONLY; actual UE 5.8 run remains the runtime authority")
