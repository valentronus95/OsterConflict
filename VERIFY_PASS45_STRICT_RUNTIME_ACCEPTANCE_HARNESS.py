#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
MAIN = ROOT / "RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd"
PLAYFLOW = ROOT / "RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd"
NORMAL = ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd"
MATERIAL = ROOT / "OsterConflict" / "RUN_PASS45_STRICT_MATERIAL_GATE.cmd"
EVIDENCE = ROOT / "VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py"
START = ROOT / "START_HERE.cmd"
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
playflow = read(PLAYFLOW)
normal = read(NORMAL)
material = read(MATERIAL)
evidence = read(EVIDENCE)
start = read(START)

# Keep one actual gameplay process. START_HERE option 2 enters the strict main wrapper, which delegates to the
# playflow/performance wrapper; only that wrapper calls RUN_R14_CURRENT_GAMEPLAY.cmd and only CURRENT_GAMEPLAY
# owns `start /wait` for Unreal gameplay.
req('call "%~dp0RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd"' in start,
    "START_HERE full runtime option bypasses the strict main acceptance wrapper")
req('set "OC_FORCE_ACCEPTANCE=1"' in main, "strict wrapper no longer enables acceptance mode")
req('call "%PLAYFLOW%"' in main, "strict wrapper no longer delegates to the playflow/performance acceptance wrapper")
req("RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd" in main, "playflow/performance wrapper identity missing from strict main wrapper")
req('call "%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"' in playflow,
    "playflow/performance wrapper no longer delegates to the canonical normal-game launcher")
req("start /wait" not in main and "start /wait" not in playflow,
    "a wrapper became a second gameplay process launcher")
req("start /wait" in normal, "canonical normal-game launcher no longer owns the gameplay process")
req("-fullscreen" in normal and 't.MaxFPS 60' in normal,
    "normal gameplay route lost Pass45 fullscreen/60 FPS recovery contract")

# START_HERE strict preparation may repair Stein authored assets once, but production vehicle intake belongs to
# CURRENT_GAMEPLAY strict stage so HMMWV/M2/BTR are not imported twice before one acceptance run.
strict_prepare = start[start.find(":prepare_materials_strict"):]
req("PASS45_REIMPORT_STEIN_WEAPON_MATERIALS_UE58.cmd" in strict_prepare,
    "START_HERE strict route no longer prepares Stein authored materials")
req("IMPORT_PRODUCTION_VEHICLES_UE58.cmd" not in strict_prepare,
    "START_HERE strict route duplicates production vehicle import before CURRENT_GAMEPLAY")

# Headless post-playtest gate consumes required-available material/dependency truth. Exact production weapon
# payload gaps may remain CONTENT GAP only when the explicit real fallback passes the same dependency checks.
for needle in (
    "-ValidateProductionWeapons",
    "-ValidateProductionWeaponsHeadless",
    "PASS45_REQUIRED_AVAILABLE_WEAPONS=PASS",
    "PASS45_AUTHORED_WEAPON_MATERIALS=PASS",
    "PASS45_WEAPON_DEPENDENCY_REPORT=PASS",
    "PASS45_EXACT_PRODUCTION_CONTENT_GAPS=",
    "PASS45_REQUIRED_AVAILABLE_WEAPON_VISUALS_VALIDATED_READY",
    "PASS45_PRODUCTION_VEHICLE_VISUALS_VALIDATED_READY",
    "PASS45_VEHICLEBASE_PRODUCTION_MATERIAL_BYPASS_READY",
):
    req(needle in material, f"strict material gate missing: {needle}")
for forbidden in (
    "PASS45_PRODUCTION_VEHICLE_MATERIAL_OVERRIDE_FAIL",
    "PASS45_PRODUCTION_VEHICLE_MATERIAL_GAP",
    "PASS45_PRODUCTION_VEHICLE_CONTENT_GAP",
    "PASS45_REQUIRED_AVAILABLE_WEAPON_RUNTIME_FAIL",
):
    req(f'findstr /C:"{forbidden}"' in material,
        f"strict material gate does not fail on: {forbidden}")
req("R14_PRODUCTION_WEAPONS=PASS" not in material,
    "strict material gate resurrected impossible all-exact production weapon sentinel")

# P0 black-world recovery is part of strict acceptance, not an optional side validation. Automated evidence must
# prove the physical daylight owner started and the semantic Ground/Roads/Sidewalks MID contract survived the
# Pass12 12s/16s/20s stability window. A geometry/material FAIL must be fatal.
for marker in (
    "PASS45_DAYLIGHT_EXPOSURE_CONTRACT_READY",
    "PASS12_WORLD_GEOMETRY_STABLE",
    "PASS45_WORLD_MATERIAL_STABLE",
    "PASS12_WORLD_GEOMETRY_STABILITY_FAIL",
    "BLACK_WORLD_AUTOMATED_CONTRACT=PASS",
):
    req(marker in evidence, f"Pass45 black-world evidence verifier missing marker/contract: {marker}")

# Acceptance must force the actual interaction sequence that reproduces the rejected teleport/M2 bugs and must
# require material truth for the rack actually rendered in gameplay.
for marker in (
    "PASS45_INITIAL_BASE_DEPLOYMENT_VALIDATED_ONCE",
    "PASS45_INITIAL_BASE_DEPLOYMENT_RECOVERED_ONCE",
    "PASS45_VEHICLE_ENTER_TRANSFORM_READY",
    "PASS45_VEHICLE_EXIT_TRANSFORM_READY",
    "PASS45_M2_GUNNER_PITCH_CONTRACT_READY",
    "PASS45_GUNNER_EXIT_TRANSFORM_READY",
    "PASS45_REQUIRED_AVAILABLE_WEAPONS_READY",
    "PASS36_WEAPON_MATERIAL_AUDIT_READY",
    "PASS45_PRODUCTION_VEHICLE_VISUALS_VALIDATED_READY",
    "PASS45_REQUIRED_AVAILABLE_WEAPON_VISUALS_VALIDATED_READY",
    "required available weapon visuals PASS",
    "textureGaps=0",
    "textureDependency=PASS",
):
    req(marker in evidence, f"Pass45 evidence verifier missing required marker: {marker}")
for marker in (
    "PASS45_INITIAL_BASE_DEPLOYMENT_RECOVERY_FAIL",
    "PASS45_VEHICLE_ENTER_TRANSFORM_FAIL",
    "PASS45_VEHICLE_EXIT_TRANSFORM_FAIL",
    "PASS45_GUNNER_EXIT_TRANSFORM_FAIL",
    "PASS45_REQUIRED_AVAILABLE_WEAPON_RUNTIME_FAIL",
    "PASS44_WEAPON_RACK_AUTHORED_MATERIAL_GAP",
    "placeholder=1",
    "textureDependency=GAP",
    "RESULT=FAIL",
):
    req(marker in evidence, f"Pass45 evidence verifier does not reject failure marker: {marker}")
req("SUMMARY=11/11 production weapon classes PASS" not in evidence,
    "evidence verifier still requires impossible all-exact production weapon summary")

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
print("- START_HERE full test -> strict main wrapper -> playflow/performance -> one canonical gameplay process")
print("- production vehicle import is not duplicated by START_HERE strict preparation")
print("- P0 black-world automated evidence requires physical daylight plus stable semantic world materials")
print("- strict post-run gate validates required available weapon materials/dependencies while preserving exact CONTENT GAP truth")
print("- driver enter/exit and M2 gunner aim/exit evidence are mandatory")
print("- world/material, vehicle/weapon material and transform failures are fatal")
print("- automated evidence cannot mark visual acceptance complete")
print("STATUS: SOURCE CONTRACT ONLY; factual local UE 5.8 playtest still required")
