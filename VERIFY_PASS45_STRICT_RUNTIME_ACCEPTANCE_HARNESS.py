#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
ENTRY = ROOT / "START_HERE.cmd"
NORMAL = ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd"
MATERIAL = ROOT / "OsterConflict" / "RUN_PASS45_STRICT_MATERIAL_GATE.cmd"
EVIDENCE = ROOT / "VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py"
errors = []


def read(path: Path) -> str:
    if not path.is_file():
        errors.append(f"missing file: {path.relative_to(ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


entry = read(ENTRY)
normal = read(NORMAL)
material = read(MATERIAL)
evidence = read(EVIDENCE)

req('Єдиний користувацький launcher/test entrypoint: START_HERE.cmd.' in entry,
    "START_HERE no longer declares the single user-facing launcher/test contract")
req('set "OC_FORCE_ACCEPTANCE=1"' in entry, "START_HERE no longer enables strict acceptance mode")
req('call "%CURRENT_GAMEPLAY%"' in entry, "START_HERE no longer delegates gameplay to RUN_R14_CURRENT_GAMEPLAY.cmd")
req('call "%MATERIAL_GATE%"' in entry, "START_HERE no longer runs the strict material gate")
req('VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py' in entry, "START_HERE no longer runs the canonical evidence verifier")
req('TRY_PRODUCTION_VEHICLES_UE58.cmd' not in entry, "obsolete TRY vehicle wrapper returned to START_HERE")
req('RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd' not in entry, "redundant strict acceptance wrapper returned")
req('RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd' not in entry, "redundant playflow acceptance wrapper returned")
req("-fullscreen" in normal and 't.MaxFPS 60' in normal,
    "normal gameplay route lost Pass45 fullscreen/60 FPS recovery contract")

for needle in (
    "-ValidateProductionWeapons",
    "-ValidateProductionWeaponsHeadless",
    "PASS45_AUTHORED_WEAPON_MATERIALS=PASS",
    "PASS45_WEAPON_DEPENDENCY_REPORT=PASS",
    "PASS45_PRODUCTION_WEAPON_VISUALS_VALIDATED_READY",
    "PASS45_PRODUCTION_VEHICLE_VISUALS_VALIDATED_READY",
    "PASS45_VEHICLEBASE_PRODUCTION_MATERIAL_BYPASS_READY",
):
    req(needle in material, f"strict material gate missing: {needle}")

for marker in (
    "PASS45_INITIAL_BASE_DEPLOYMENT_VALIDATED_ONCE",
    "PASS45_VEHICLE_ENTER_TRANSFORM_READY",
    "PASS45_VEHICLE_EXIT_TRANSFORM_READY",
    "PASS45_M2_GUNNER_PITCH_CONTRACT_READY",
    "PASS45_GUNNER_EXIT_TRANSFORM_READY",
    "PASS45_PRODUCTION_VEHICLE_VISUALS_VALIDATED_READY",
    "PASS14_PERF_30FPS_READY",
    "SUMMARY=11/11 production weapon classes PASS",
):
    req(marker in evidence, f"Pass45 evidence verifier missing required marker: {marker}")

for marker in (
    "PASS45_INITIAL_BASE_DEPLOYMENT_RECOVERY_FAIL",
    "PASS45_VEHICLE_ENTER_TRANSFORM_FAIL",
    "PASS45_VEHICLE_EXIT_TRANSFORM_FAIL",
    "PASS45_GUNNER_EXIT_TRANSFORM_FAIL",
    "PASS14_PERF_BELOW_TARGET",
    "placeholder=1",
    "RESULT=FAIL",
):
    req(marker in evidence, f"Pass45 evidence verifier does not reject failure marker: {marker}")

req("VISUAL_ACCEPTANCE=PENDING_MANUAL_OBSERVATION" in evidence,
    "evidence file no longer preserves visual acceptance as pending")
req("VISUAL ACCEPTANCE IS STILL PENDING" in entry,
    "START_HERE falsely implies automated logs complete visual acceptance")
req("PASS45_RUNTIME_AUTOMATED_EVIDENCE=PASS" in evidence,
    "evidence output lacks explicit automated-only PASS status")

if errors:
    print("PASS45 STRICT RUNTIME ACCEPTANCE HARNESS: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 STRICT RUNTIME ACCEPTANCE HARNESS: PASS")
print("- START_HERE.cmd is the only user-facing launcher/test entrypoint")
print("- RUN_R14_CURRENT_GAMEPLAY.cmd is the one internal gameplay route")
print("- strict material, interaction and performance evidence is centralized")
print("STATUS: SOURCE CONTRACT ONLY; factual local UE 5.8 playtest still required")
