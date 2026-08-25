#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
MAIN = ROOT / "RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd"
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


main = read(MAIN)
normal = read(NORMAL)
material = read(MATERIAL)
evidence = read(EVIDENCE)

# Keep one normal gameplay launcher. The strict wrapper orchestrates evidence; it does not invent another gameplay route.
req('set "OC_FORCE_ACCEPTANCE=1"' in main, "strict wrapper no longer enables acceptance mode")
req('call "%CURRENT_GAMEPLAY%"' in main, "strict wrapper no longer delegates gameplay to RUN_R14_CURRENT_GAMEPLAY.cmd")
req("start /wait" not in main, "strict wrapper became a second gameplay launcher")
req("RUN_R14_CURRENT_GAMEPLAY.cmd" in main, "single normal-game launcher identity missing")
req("-fullscreen" in normal and 't.MaxFPS 60' in normal,
    "normal gameplay route lost Pass45 fullscreen/60 FPS recovery contract")

# Headless post-playtest gate must consume the exact authored material/dependency contracts merged in Pass45.
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
for forbidden in (
    "PASS45_PRODUCTION_VEHICLE_MATERIAL_OVERRIDE_FAIL",
    "PASS45_PRODUCTION_VEHICLE_MATERIAL_GAP",
    "PASS45_PRODUCTION_VEHICLE_CONTENT_GAP",
):
    req(f'findstr /C:"{forbidden}"' in material,
        f"strict material gate does not fail on: {forbidden}")

# Acceptance must force the actual interaction sequence that reproduces the rejected teleport/M2 bugs.
for marker in (
    "PASS45_INITIAL_BASE_DEPLOYMENT_VALIDATED_ONCE",
    "PASS45_INITIAL_BASE_DEPLOYMENT_RECOVERED_ONCE",
    "PASS45_VEHICLE_ENTER_TRANSFORM_READY",
    "PASS45_VEHICLE_EXIT_TRANSFORM_READY",
    "PASS45_M2_GUNNER_PITCH_CONTRACT_READY",
    "PASS45_GUNNER_EXIT_TRANSFORM_READY",
    "PASS45_PRODUCTION_VEHICLE_VISUALS_VALIDATED_READY",
    "PASS45_PRODUCTION_WEAPON_VISUALS_VALIDATED_READY",
    "SUMMARY=11/11 production weapon classes PASS",
):
    req(marker in evidence, f"Pass45 evidence verifier missing required marker: {marker}")
for marker in (
    "PASS45_INITIAL_BASE_DEPLOYMENT_RECOVERY_FAIL",
    "PASS45_VEHICLE_ENTER_TRANSFORM_FAIL",
    "PASS45_VEHICLE_EXIT_TRANSFORM_FAIL",
    "PASS45_GUNNER_EXIT_TRANSFORM_FAIL",
    "placeholder=1",
    "RESULT=FAIL",
):
    req(marker in evidence, f"Pass45 evidence verifier does not reject failure marker: {marker}")

# Automation cannot falsely promote log evidence to visual acceptance.
req("VISUAL_ACCEPTANCE=PENDING_MANUAL_OBSERVATION" in evidence,
    "evidence file no longer preserves visual acceptance as pending")
req("VISUAL ACCEPTANCE IS STILL PENDING" in main,
    "strict wrapper falsely implies automated logs complete visual acceptance")
req("PASS45_RUNTIME_AUTOMATED_EVIDENCE=PASS" in evidence,
    "evidence output lacks explicit automated-only PASS status")

if errors:
    print("PASS45 STRICT RUNTIME ACCEPTANCE HARNESS: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 STRICT RUNTIME ACCEPTANCE HARNESS: PASS")
print("- RUN_R14_CURRENT_GAMEPLAY.cmd remains the single normal gameplay launcher")
print("- strict wrapper adds post-playtest authored material/dependency validation")
print("- driver enter/exit and M2 gunner aim/exit evidence are mandatory")
print("- vehicle/weapon material gaps and transform failures are fatal")
print("- automated evidence cannot mark visual acceptance complete")
print("STATUS: SOURCE CONTRACT ONLY; factual local UE 5.8 playtest still required")
