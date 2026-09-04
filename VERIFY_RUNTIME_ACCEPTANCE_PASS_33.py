#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
START = ROOT / "START_HERE.cmd"
EVIDENCE = ROOT / "VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py"
FINALIZER = ROOT / "OsterConflict" / "Scripts" / "finalize_asset_acceptance.py"


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


start = read(START)
evidence = read(EVIDENCE)
finalizer = read(FINALIZER)

# START_HERE is the only user-facing launcher. Full runtime testing must stay on the current
# gameplay -> live asset proof -> strict material gate -> canonical automated evidence -> manual finalization route.
for needle in (
    "2. ПОВНИЙ RUNTIME-ТЕСТ",
    'set "CURRENT_GAMEPLAY=%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"',
    'set "ALL_ASSET_IMPORT=%~dp0OsterConflict\\IMPORT_ALL_LOCAL_INBOX_UE58.cmd"',
    'set "MATERIAL_GATE=%~dp0OsterConflict\\RUN_PASS45_STRICT_MATERIAL_GATE.cmd"',
    'set "EVIDENCE_VERIFY=%~dp0VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py"',
    'set "ASSET_FINALIZER=%~dp0OsterConflict\\Scripts\\finalize_asset_acceptance.py"',
    "call :full_runtime_test",
    "call :ingest_all_assets",
    'set "OC_FORCE_ACCEPTANCE=1"',
    'set "OC_VALIDATE_LOCAL_INBOX=1"',
    'call "%CURRENT_GAMEPLAY%"',
    'PASS45_LOCAL_INBOX_RUNTIME=PASS',
    'PASS45_LOCAL_WORLD_RUNTIME=PASS',
    'call "%MATERIAL_GATE%"',
    '%PY_CMD% "%EVIDENCE_VERIFY%" "%GAMEPLAY_LOG%" "%MATERIAL_LOG%" "%WEAPON_REPORT%"',
    "PASS45 AUTOMATED RUNTIME EVIDENCE GATES PASSED",
    '"%ASSET_FINALIZER%" --preflight',
    "FINALIZE PENDING",
    "Visual acceptance не записано",
    "choice /C YN",
    '"%ASSET_FINALIZER%" --accept-visual',
):
    require(start, needle, "current START_HERE full runtime route")

preflight_pos = start.find('"%ASSET_FINALIZER%" --preflight')
choice_pos = start.find('choice /C YN', preflight_pos)
accept_pos = start.find('"%ASSET_FINALIZER%" --accept-visual', choice_pos)
if -1 in (preflight_pos, choice_pos, accept_pos) or not preflight_pos < choice_pos < accept_pos:
    raise SystemExit("PASS33 VERIFY FAIL: visual acceptance is not gated after automated evidence + preflight + explicit Y/N")

forbid(
    start,
    "RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd",
    "deleted per-pass acceptance launcher must not return",
)
forbid(
    start,
    "FINALIZE_ASSET_ACCEPTANCE_AND_CLEANUP.cmd",
    "second user-facing finalization launcher must not return",
)

# The canonical Python verifier owns automated runtime evidence semantics now.
for marker in (
    "PASS29_MAIN_START_DIRECT_HOST_QUEUED",
    "PASS29_STATIC_FRONTEND_HOST_TRAVEL_EXECUTE",
    "PASS14_HOST_TRAVEL_BEGIN",
    "PASS14_FRONTEND_TRAVEL_HANDOFF_READY",
    "PASS44_LOCAL_BOT_AUTOFILL_DISABLED_READY",
    "PASS44_PRIMARY_WORLD_COMPACT_AUTHORING_READY",
    "PASS44_RUNTIME_GAMEPLAY_SEEDS_COMPACT_READY",
    "PASS44_COMBAT_VEHICLE_SEEDS_COMPACT_READY",
    "PASS44_COMPACT_PLAYABLE_AREA_READY",
    "PASS44_TACTICAL_MAP_COMPACT_BOUNDS_READY",
    "PASS44_ACTUAL_PAWN_MUSEUM_BASE_READY",
    "PASS45_LANDMARK_STARTUP_COORDINATED_READY",
    "PASS45_MUSEUM_R137_VISIBLE_OWNER_PRESERVED",
    "PASS45_MUSEUM_R138_COLLISION_ONLY_READY",
    "PASS45_MUSEUM_SINGLE_VISIBLE_OWNER_READY",
    "PASS45_MUSEUM_LAYER_VALIDATION_READY",
    "PASS42_BASE_RACK_GROUNDED_READY",
    "PASS45_VEHICLEBASE_PRODUCTION_MATERIAL_BYPASS_READY",
    "PASS45_PRODUCTION_VEHICLE_VISUALS_VALIDATED_READY",
    "PASS45_HMMWV_PROPORTIONAL_VISUAL_READY",
    "PASS45_BTR4_PROPORTIONAL_VISUAL_READY",
    "PASS45_M2_MOUNT_ALIGNMENT_READY",
    "PASS45_VEHICLE_ENTER_TRANSFORM_READY",
    "PASS45_VEHICLE_EXIT_TRANSFORM_READY",
    "PASS45_M2_GUNNER_PITCH_CONTRACT_READY",
    "PASS45_GUNNER_EXIT_TRANSFORM_READY",
    "PASS31_GAMEPLAY_INPUT_READY",
    "PASS41_INPUT_RECOVERY_POLL_BUDGET_READY",
    "PASS36_LOWCPU_FOLIAGE_RUNTIME_READY",
    "PASS36_WEAPON_MATERIAL_AUDIT_READY",
    "PASS38_WEAPON_FALLBACK_SCAN_STOPPED",
    "PASS40_UI_STABILIZER_BUDGET_READY",
    "PASS40_DEPLOYMENT_PRESENTATION_BUDGET_READY",
    "PASS14_PERF_SAMPLE",
    "PASS14_PERF_30FPS_READY",
    "PASS7_MUSEUM_BASES_READY",
):
    require(evidence, marker, f"canonical runtime evidence {marker}")

# Either factual initial BASE terminal result is valid; ordinary vehicle possession is not a new deployment.
for marker in (
    "PASS45_INITIAL_BASE_DEPLOYMENT_VALIDATED_ONCE",
    "PASS45_INITIAL_BASE_DEPLOYMENT_RECOVERED_ONCE",
):
    require(evidence, marker, f"initial BASE evidence option {marker}")
require(evidence, "require_any(", "initial BASE alternatives must remain explicit")
require(evidence, '"moveIgnored=0"', "released gameplay move input proof")
require(evidence, '"lookIgnored=0"', "released gameplay look input proof")

for failure_marker in (
    "PASS45_INITIAL_BASE_DEPLOYMENT_RECOVERY_FAIL",
    "PASS44_ACTUAL_PAWN_MUSEUM_BASE_FAIL",
    "PASS44_COMPACT_PLAYABLE_AREA_FAIL",
    "PASS37_BASE_DEPLOYMENT_RECOVERY_FAIL",
    "PASS45_MUSEUM_SINGLE_VISIBLE_OWNER_FAIL",
    "PASS45_MUSEUM_R138_COLLISION_ONLY_FAIL",
    "PASS45_MUSEUM_LAYER_VALIDATION_FAIL",
    "PASS45_LANDMARK_SEPARATION_VALIDATION_FAIL",
    "PASS42_BASE_RACK_GROUNDING_INCOMPLETE",
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
    require(evidence, failure_marker, f"fail-closed runtime marker {failure_marker}")

# Local asset/world live proof gets distinct failure codes before material/evidence verification.
for exit_code in ("exit /b 35", "exit /b 36", "exit /b 37", "exit /b 38"):
    require(start, exit_code, f"live asset/world failure {exit_code}")
for propagated in (
    "exit /b %GAME_RC%",
    "exit /b %MATERIAL_RC%",
    "exit /b %EVIDENCE_RC%",
):
    require(start, propagated, f"current gate failure propagation {propagated}")

# Authored production materials and exact weapon texture dependencies are hard automated gates.
for marker in (
    "PASS45_PRODUCTION_WEAPON_VISUALS_VALIDATED_READY",
    "SUMMARY=11/11 production weapon classes PASS",
    "materialGaps=0",
    "unexpectedOverrides=0",
    "textureCount=",
    "VISUAL_ACCEPTANCE=PENDING_MANUAL_OBSERVATION",
):
    require(evidence, marker, f"current material/runtime evidence {marker}")
for marker in (
    "PASS45_PRODUCTION_WEAPON_CONTENT_GAP",
    "placeholder=1",
    "RESULT=FAIL",
):
    require(evidence, marker, f"fail-closed production evidence {marker}")

# Final manual acceptance must remain outside the automated evidence verifier.
for marker in (
    'preflight_only = "--preflight" in args',
    'accept_visual = "--accept-visual" in args',
    "verify_current_automated_status(current, head)",
    'category_counts.get("M16_M4")',
    "write_manual_acceptance(head)",
):
    require(finalizer, marker, f"manual finalization contract {marker}")

# Retired historical repair/palette/rebuild semantics must not be restored just to satisfy Pass33.
for retired in (
    "PASS37_WEAPON_VISIBLE_PALETTE_READY",
    "PASS42_PRODUCTION_MATERIALS_RESTORED",
    "PASS42_PRODUCTION_VEHICLE_VISUALS_READY",
    "PASS30_MUSEUM_SPECULATIVE_INTERIOR_REMOVED",
    "PASS32_MUSEUM_LAYER_BUDGET_READY",
    "PASS32_MUSEUM_LAYER_BUDGET_FAIL",
    "PASS38_MUSEUM_REBUILD_BUDGET_READY",
    "PASS38_MUSEUM_REBUILD_BUDGET_FAIL",
    "PASS38_WEAPON_PALETTE_SCAN_STOPPED",
):
    forbid(evidence, retired, f"retired compatibility marker {retired}")

print("RUNTIME ACCEPTANCE PASS 33 / PASS45 CURRENT CONTRACT PASS")
print("- START_HERE owns the full runtime-test route; deleted per-pass acceptance CMD stays absent")
print("- local inbox/world live runtime proof is mandatory before material/evidence acceptance")
print("- canonical Python evidence verifier owns Museum, vehicle, weapon, input and >=30 FPS automated gates")
print("- automated evidence leaves visual acceptance pending; finalizer preflight + explicit Y/N own the manual visual/cleanup stage")
print("- production material/texture gaps remain fail-closed")
print("STATUS: SOURCE VERIFIED; actual UE 5.8 run remains the runtime authority")
