#!/usr/bin/env python3
from pass45_runtime_route_contract import require, validate_runtime_route

route = validate_runtime_route()
normal = route["normal"]
material = route["material"]
evidence = route["evidence"]
finalizer = route["finalizer"]
batch = route["batch"]

require(normal, "-fullscreen", "fullscreen runtime contract")
require(normal, "t.MaxFPS 60", "60 FPS cap contract")
for marker in ("-ValidateProductionWeapons", "-ValidateProductionWeaponsHeadless", "PASS45_AUTHORED_WEAPON_MATERIALS=PASS", "PASS45_WEAPON_DEPENDENCY_REPORT=PASS", "PASS45_PRODUCTION_WEAPON_VISUALS_VALIDATED_READY", "PASS45_PRODUCTION_VEHICLE_VISUALS_VALIDATED_READY"):
    require(material, marker, "strict material gate")
for marker in ("PASS45_INITIAL_BASE_DEPLOYMENT_VALIDATED_ONCE", "PASS45_VEHICLE_ENTER_TRANSFORM_READY", "PASS45_VEHICLE_EXIT_TRANSFORM_READY", "PASS45_M2_GUNNER_PITCH_CONTRACT_READY", "PASS45_GUNNER_EXIT_TRANSFORM_READY", "PASS14_PERF_30FPS_READY", "SUMMARY=11/11 production weapon classes PASS", "PASS45_RUNTIME_AUTOMATED_EVIDENCE=PASS", "VISUAL_ACCEPTANCE=PENDING_MANUAL_OBSERVATION"):
    require(evidence, marker, "strict runtime evidence")
for marker in ("PASS45_INITIAL_BASE_DEPLOYMENT_RECOVERY_FAIL", "PASS45_VEHICLE_ENTER_TRANSFORM_FAIL", "PASS45_VEHICLE_EXIT_TRANSFORM_FAIL", "PASS45_GUNNER_EXIT_TRANSFORM_FAIL", "PASS14_PERF_BELOW_TARGET", "placeholder=1", "RESULT=FAIL"):
    require(evidence, marker, "fail-closed evidence")
for marker in ('preflight_only = "--preflight" in args', 'accept_visual = "--accept-visual" in args', "run_preflight()", "write_manual_acceptance(head)"):
    require(finalizer, marker, "manual finalizer")
for marker in ('$env:OC_FORCE_ACCEPTANCE = "1"', 'Invoke-Stage "Strict authored material/dependency gate"', "VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py", '@($Finalizer, "--preflight")', '@($Finalizer, "--accept-visual")'):
    require(batch, marker, "packet strict acceptance owner")

print("PASS45 STRICT RUNTIME ACCEPTANCE HARNESS: PASS")
print("- START_HERE delegates; packet runner owns strict acceptance and finalization")
print("- automated evidence cannot silently become manual visual acceptance")
print("STATUS: SOURCE CONTRACT ONLY; factual local UE 5.8 playtest still required")
