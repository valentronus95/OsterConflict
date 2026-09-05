#!/usr/bin/env python3
from pass45_runtime_route_contract import require, forbid, validate_runtime_route

route = validate_runtime_route()
evidence = route["evidence"]
finalizer = route["finalizer"]
batch = route["batch"]

for marker in (
    "PASS29_MAIN_START_DIRECT_HOST_QUEUED", "PASS29_STATIC_FRONTEND_HOST_TRAVEL_EXECUTE",
    "PASS14_HOST_TRAVEL_BEGIN", "PASS14_FRONTEND_TRAVEL_HANDOFF_READY",
    "PASS44_LOCAL_BOT_AUTOFILL_DISABLED_READY", "PASS44_ACTUAL_PAWN_MUSEUM_BASE_READY",
    "PASS45_MUSEUM_SINGLE_VISIBLE_OWNER_READY", "PASS45_MUSEUM_LAYER_VALIDATION_READY",
    "PASS45_PRODUCTION_VEHICLE_VISUALS_VALIDATED_READY", "PASS45_VEHICLE_ENTER_TRANSFORM_READY",
    "PASS45_VEHICLE_EXIT_TRANSFORM_READY", "PASS45_M2_GUNNER_PITCH_CONTRACT_READY",
    "PASS45_GUNNER_EXIT_TRANSFORM_READY", "PASS31_GAMEPLAY_INPUT_READY",
    "PASS36_LOWCPU_FOLIAGE_RUNTIME_READY", "PASS36_WEAPON_MATERIAL_AUDIT_READY",
    "PASS38_WEAPON_FALLBACK_SCAN_STOPPED", "PASS14_PERF_SAMPLE", "PASS14_PERF_30FPS_READY",
    "PASS7_MUSEUM_BASES_READY", "PASS19_PLAYABLE_WEAPON_SET_READY",
):
    require(evidence, marker, f"canonical runtime evidence {marker}")
for marker in (
    "PASS45_INITIAL_BASE_DEPLOYMENT_RECOVERY_FAIL", "PASS44_ACTUAL_PAWN_MUSEUM_BASE_FAIL",
    "PASS45_MUSEUM_SINGLE_VISIBLE_OWNER_FAIL", "PASS45_MUSEUM_LAYER_VALIDATION_FAIL",
    "PASS45_PRODUCTION_VEHICLE_CONTENT_GAP", "PASS45_VEHICLE_ENTER_TRANSFORM_FAIL",
    "PASS45_VEHICLE_EXIT_TRANSFORM_FAIL", "PASS45_GUNNER_EXIT_TRANSFORM_FAIL",
    "PASS15_EMERGENCY_PERF_PROFILE_APPLIED", "PASS14_PERF_BELOW_TARGET",
    "PASS19_PLAYABLE_WEAPON_SET_FAIL",
):
    require(evidence, marker, f"fail-closed evidence {marker}")
for marker in ("PASS45_INITIAL_BASE_DEPLOYMENT_VALIDATED_ONCE", "PASS45_INITIAL_BASE_DEPLOYMENT_RECOVERED_ONCE", "require_any("):
    require(evidence, marker, "initial BASE alternatives")
for marker in ('preflight_only = "--preflight" in args', 'accept_visual = "--accept-visual" in args', "verify_current_automated_status(current, head)", 'category_counts.get("M16_M4")', "write_manual_acceptance(head)"):
    require(finalizer, marker, "manual finalization")
for marker in ("Single gameplay runtime", "runtime evidence verifier failed", "FORMAL_ACCEPTANCE=BLOCKED_DIRTY_OR_NONEXACT_SOURCE", '@($Finalizer, "--preflight")', '@($Finalizer, "--accept-visual")'):
    require(batch, marker, "packet acceptance order")
for retired in ("RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd", "FINALIZE_ASSET_ACCEPTANCE_AND_CLEANUP.cmd"):
    forbid(route["start"], retired, "retired launcher")

print("RUNTIME ACCEPTANCE PASS 33 / PASS45 CURRENT CONTRACT PASS")
print("- packet runner owns import/runtime/material/evidence/finalizer sequence")
print("- automated evidence remains fail-closed and manual acceptance stays separate")
print("STATUS: SOURCE VERIFIED; actual UE 5.8 run remains the runtime authority")
