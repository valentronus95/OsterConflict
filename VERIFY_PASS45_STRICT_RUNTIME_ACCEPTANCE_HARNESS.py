#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
START = ROOT / "START_HERE.cmd"
BATCH_CMD = ROOT / "OsterConflict" / "PASS45_BATCH_RUNTIME.cmd"
BATCH_PY = ROOT / "OsterConflict" / "Scripts" / "pass45_batch_runtime.py"
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


start = read(START)
batch_cmd = read(BATCH_CMD)
batch_py = read(BATCH_PY)
material = read(MATERIAL)
evidence = read(EVIDENCE)

# Current contract: START_HERE option 2 owns one batch-first diagnostic/acceptance route.
# Independent preflight stages all run and report before the single gameplay process is allowed to start.
req('call "%~dp0OsterConflict\\PASS45_BATCH_RUNTIME.cmd"' in start,
    "START_HERE option 2 does not enter the batch runtime wrapper")
req(":prepare_materials_strict" not in start,
    "retired fail-fast START_HERE material chain returned")
req('call "%~dp0RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd"' not in start,
    "retired nested strict-main option-2 route returned")
req("pass45_batch_runtime.py" in batch_cmd,
    "batch command wrapper no longer delegates to the Python orchestrator")

for needle in (
    "IMPORT_ALL_LOCAL_INBOX_UE58.cmd",
    "PASS45_REIMPORT_STEIN_WEAPON_MATERIALS_UE58.cmd",
    "PASS45_IMPORT_MANUAL_ACTION_AUDIO_UE58.cmd",
    "PASS45_IMPORT_REMINGTON870_PRODUCTION_UE58.cmd",
    "IMPORT_PRODUCTION_VEHICLES_UE58.cmd",
    "verify_required_weapon_assets.py",
    "RUN_PASS45_STRICT_MATERIAL_GATE.cmd",
    "VERIFY_PASS45_GATE_K_RUNTIME_LOG.py",
    "VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py",
    "VERIFY_PASS45_MANUAL_ACTION_RUNTIME.py",
    "VERIFY_PASS45_GRENADE_THROW_ANIMATION_RUNTIME.py",
    "VERIFY_PASS45_GRENADE_FLASH_RUNTIME.py",
    "/Game/Maps/OsterConflict_Runtime",
    '"-d3d11", "-sm5", "-nohdr"',
    "PASS45_BATCH_RUNTIME_REPORT.txt",
    "PREFLIGHT_FAIL",
    "RUNTIME_OR_POSTCHECK_FAIL",
    "DIAGNOSTIC_PASS_FORMAL_ACCEPTANCE_BLOCKED",
    "AUTOMATED_PASS_VISUAL_ACCEPTANCE_PENDING",
):
    req(needle in batch_py, f"batch runtime contract missing: {needle}")

req(batch_py.count("subprocess.run(runtime_cmd") == 1,
    "batch route must own exactly one gameplay process")
for destructive in ("git reset", "git clean", "git stash", "checkout --", "restore --"):
    req(destructive not in batch_py.lower(),
        f"batch route must preserve user local Changes: {destructive}")
req("tracked_changes_before=" in batch_py and "tracked_changes_after=" in batch_py,
    "batch report no longer records local tracked Changes as formal blockers")

# Headless authored material/dependency truth remains mandatory. Missing exact payload may only use an
# explicit real authored fallback and must remain an explicit CONTENT GAP.
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
        f"strict material gate does not reject: {forbidden}")
req("R14_PRODUCTION_WEAPONS=PASS" not in material,
    "strict material gate resurrected impossible all-exact production readiness")

# P0 black-world / semantic world truth remains factual runtime evidence.
for marker in (
    "PASS45_DAYLIGHT_EXPOSURE_CONTRACT_READY",
    "PASS12_WORLD_GEOMETRY_STABLE",
    "PASS45_WORLD_MATERIAL_STABLE",
    "PASS12_WORLD_GEOMETRY_STABILITY_FAIL",
    "BLACK_WORLD_AUTOMATED_CONTRACT=PASS",
):
    req(marker in evidence, f"black-world evidence contract missing: {marker}")

# Landmark identity/separation remains mandatory.
for marker in (
    "PASS45_LANDMARK_SEPARATION_VALIDATION_READY",
    "PASS45_LANDMARK_IDENTITY_VALIDATION_READY",
    "PASS45_SILPO_IDENTITY_VALIDATION_READY",
    "PASS45_LANDMARK_SEPARATION_VALIDATION_FAIL",
    "PASS45_LANDMARK_IDENTITY_VALIDATION_FAIL",
    "PASS45_SILPO_IDENTITY_VALIDATION_FAIL",
    "LANDMARK_IDENTITY_AUTOMATED_CONTRACT=PASS",
    "SILPO_IDENTITY_AUTOMATED_CONTRACT=PASS",
):
    req(marker in evidence, f"landmark identity evidence contract missing: {marker}")

# Retired generic residential/private-fence presentation must not return.
for marker in (
    "PASS45_REFERENCE_DRIVEN_RESIDENTIAL_RUNTIME_READY",
    "PASS45_REFERENCE_DRIVEN_RESIDENTIAL_RUNTIME_FAIL",
    "REFERENCE_DRIVEN_RESIDENTIAL_RUNTIME_CONTRACT=PASS",
):
    req(marker in evidence, f"Gate E evidence contract missing: {marker}")

# Renderer/thermal evidence must come from actual UE runtime after possession.
for marker in (
    "PASS45_THERMAL_CAP_RUNTIME_READY",
    "PASS45_THERMAL_CAP_RUNTIME_FAIL",
    "PASS45_FULLSCREEN_RUNTIME_READY",
    "PASS45_FULLSCREEN_RUNTIME_FAIL",
    "THERMAL_CAP_RUNTIME_CONTRACT=PASS",
    "FULLSCREEN_RUNTIME_CONTRACT=PASS",
):
    req(marker in evidence, f"thermal/fullscreen evidence contract missing: {marker}")

# Item 16 stays factual: Remington gameplay must reach the authored pump sequence and mechanical audio path.
for marker in (
    "PASS45_MANUAL_ACTION_AUTHORED_SOURCE_BRIDGE_READY",
    "weapon=OC_SG1",
    "/Game/Production/Weapons/Remington870/AN_Remington870_PumpCycle.AN_Remington870_PumpCycle",
    "PASS45_MANUAL_ACTION_AUTHORED_SOURCE_BRIDGE_FAIL weapon=OC_SG1",
    "PASS45_MANUAL_ACTION_AUTHORED_CONTENT_GAP weapon=OC_SG1",
    "PASS45_WEAPON_AUDIO_CONTENT_GAP weapon=OC_SG1 event=manual_action",
    "REMINGTON870_AUTHORED_PUMP_RUNTIME_BRIDGE=PASS",
):
    req(marker in evidence, f"Remington runtime evidence contract missing: {marker}")

# Grenades remain factual authored runtime content, not source-only wiring.
for marker in (
    "PASS45_GRENADE_PRODUCTION_VISUAL_READY",
    "PASS45_GRENADE_THROW_COMMIT_READY",
    "PASS45_GRENADE_THROW_PRESENTATION_BRIDGE_READY",
    "PASS45_GRENADE_THROW_AUDIO_RUNTIME_READY",
    "PASS45_SMOKE_VFX_RUNTIME_READY",
    "SMOKE_AUTHORED_VFX=PASS",
):
    req(marker in evidence, f"ordnance evidence contract missing: {marker}")
for marker in (
    "PASS45_GRENADE_PRODUCTION_VISUAL_FAIL",
    "PASS45_GRENADE_SAFE_SPAWN_REJECTED",
    "PASS45_GRENADE_SPAWN_FAIL",
    "PASS45_GRENADE_THROW_AUDIO_CONTENT_GAP",
    "PASS45_SMOKE_VFX_LOAD_FAIL",
    "PASS45_SMOKE_VFX_CONTENT_GAP",
    "PASS45_SMOKE_GAMEPLAY_VOLUME_FAIL",
):
    req(marker in evidence, f"ordnance failure rejection missing: {marker}")

# Runtime interaction/material gates remain mandatory after the one integrated playtest.
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
    req(marker in evidence, f"runtime evidence verifier missing: {marker}")
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
    req(marker in evidence, f"runtime failure rejection missing: {marker}")
req("SUMMARY=11/11 production weapon classes PASS" not in evidence,
    "evidence verifier still requires fictional all-exact production readiness")

# Automated evidence never claims that the user visually accepted the result.
req("VISUAL_ACCEPTANCE=PENDING_MANUAL_OBSERVATION" in evidence,
    "evidence no longer preserves visual acceptance as manual")
req("PASS45_RUNTIME_AUTOMATED_EVIDENCE=PASS" in evidence,
    "automated evidence PASS marker is missing")
req("AUTOMATED_PASS_VISUAL_ACCEPTANCE_PENDING" in batch_py,
    "batch route can no longer distinguish automated PASS from manual visual acceptance")

if errors:
    print("PASS45 STRICT RUNTIME ACCEPTANCE HARNESS: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 STRICT RUNTIME ACCEPTANCE HARNESS: PASS")
print("- START_HERE option 2 -> one batch-first orchestrator -> all independent preflight gates -> one gameplay process")
print("- local/Fab assets, Stein/audio/Remington, HMMWV/M2/BTR, required weapon assets and material truth are collected together")
print("- user tracked Changes are preserved and reported as formal blockers, never reset/stashed/cleaned")
print("- post-runtime Gate K, interactions, manual actions and grenade evidence are all evaluated")
print("- automated evidence cannot mark visual acceptance complete")
print("STATUS: SOURCE CONTRACT ONLY; factual local UE 5.8 batch run remains authoritative")
