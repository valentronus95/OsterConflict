#!/usr/bin/env python3
"""Static fail-closed guard for the PASS45 Remington 870 production pump route."""
from pathlib import Path

ROOT = Path(__file__).resolve().parent


def read(rel: str) -> str:
    path = ROOT / rel
    if not path.is_file():
        raise SystemExit(
            f"PASS45 REMINGTON870 PRODUCTION WIRING: FAIL\n[FAIL] missing file: {rel}"
        )
    return path.read_text(encoding="utf-8", errors="replace")


errors: list[str] = []


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


importer = read("PASS45_REMINGTON870_PRODUCTION_UE58_IMPORT.py")
fresh = read("OsterConflict/Scripts/verify_remington870_production_fresh_load.py")
wrapper = read("OsterConflict/PASS45_IMPORT_REMINGTON870_PRODUCTION_UE58.cmd")
variants = read("OsterConflict/Source/OsterConflict/Private/OCWeaponVariants.cpp")
profiles = read("OsterConflict/Source/OsterConflict/Private/OCWeaponAnimationProfiles.cpp")
validator = read("OsterConflict/Source/OsterConflict/Private/OCProductionWeaponRuntimeValidationSubsystem.cpp")
presentation = read("OsterConflict/Source/OsterConflict/Private/OCFirstPersonWeaponPresentationSubsystem.cpp")
start_here = read("START_HERE.cmd")

SKELETAL_OBJECT = "/Game/Production/Weapons/Remington870/SKM_Remington870.SKM_Remington870"
PUMP_OBJECT = "/Game/Production/Weapons/Remington870/AN_Remington870_PumpCycle.AN_Remington870_PumpCycle"
OLD_STATIC_OBJECT = "/Game/Production/Weapons/Remington870/SM_Remington870.SM_Remington870"

for needle in (
    'IMPORT_CONTRACT_REVISION = "PASS45_REMINGTON870_DERIVED_PUMP_PROD_R2"',
    'EXPECTED_SOURCE_SHA256 = "147aa6a0b167ba3f0806ad19a1cb6cc8790a0d541079f54d2e0fa8cf829954a2"',
    'EXPECTED_FORE_END_VERTICES = 1170',
    'EXPECTED_SIDE_SADDLE_VERTICES = 3241',
    'PUMP_BONE = "PASS45_PumpForeEnd"',
    'PUMP_ANIMATION_SOURCE_NAME = "PASS45_Remington870_PumpCycle"',
    'doc["animations"] = pump',
    'import_static_meshes", False',
    'import_skeletal_meshes", True',
    'InterchangeCombineSkeletalMeshesBehavior.ALL',
    'InterchangeForceMeshType.IFMT_SKELETAL_MESH',
    'create_physics_asset", False',
    'if static_meshes:',
    'len(skeletal_meshes) != 1',
    'len(pump_animations) != 1',
    'FULL_WEAPON_FORCED_TO_SINGLE_SKELETAL=1',
    'PUMP_MOTION_PRESERVED=1',
    'SHARED_SKELETON_PRESERVED=1',
    'runtime_acceptance=0',
    'item16_checked=0',
):
    req(needle in importer, f"production importer contract missing: {needle}")

for forbidden in (
    'runtime_acceptance=1',
    'item16_checked=1',
    'delete_directory(',
    'delete_asset(',
    'replace_existing", True',
):
    req(forbidden not in importer, f"production importer regained forbidden mutation/acceptance: {forbidden}")

for needle in (
    'IMPORT_CONTRACT_REVISION = "PASS45_REMINGTON870_DERIVED_PUMP_PROD_R2"',
    'SKELETAL_ASSET = "/Game/Production/Weapons/Remington870/SKM_Remington870"',
    'PUMP_ANIMATION_ASSET = "/Game/Production/Weapons/Remington870/AN_Remington870_PumpCycle"',
    'PUMP_BONE = "PASS45_PumpForeEnd"',
    'FULL_WEAPON_SINGLE_SKELETAL=1',
    'PUMP_BONE_ADDRESSABLE=1',
    'PUMP_MOTION_PRESERVED=1',
    'SHARED_SKELETON_PRESERVED=1',
    'PRODUCTION_FRESH_LOAD_READY=1',
    'runtime_acceptance=0',
    'item16_checked=0',
):
    req(needle in fresh, f"fresh-load contract missing: {needle}")

for needle in (
    'TRY_PASS45_REMINGTON870_DERIVED_PUMP_UE58_PILOT.cmd',
    'PASS45_REMINGTON870_PRODUCTION_UE58_IMPORT.py',
    'verify_remington870_production_fresh_load.py',
    'REVISION=PASS45_REMINGTON870_DERIVED_PUMP_PROD_R2',
    'call "%PILOT%"',
    'Production import заборонено',
    'FULL_WEAPON_FORCED_TO_SINGLE_SKELETAL=1',
    'FULL_WEAPON_SINGLE_SKELETAL=1',
    'PRODUCTION_FRESH_LOAD_READY=1',
):
    req(needle in wrapper, f"production wrapper contract missing: {needle}")

req(
    'ApplySkeletalProductionWeapon(this, WeaponRoot,\n        TEXT("/Game/Production/Weapons/Remington870/SKM_Remington870.SKM_Remington870")' in variants,
    "Remington runtime owner is not using the canonical skeletal production weapon",
)
req(OLD_STATIC_OBJECT not in variants, "old static Remington runtime owner returned")
req(
    'PASS45_REMINGTON870_PRODUCTION_SKELETAL_READY' in variants,
    "Remington skeletal runtime source marker missing",
)

req(PUMP_OBJECT in profiles, "Remington manual-action profile is not wired to PumpCycle")
req(
    'FName(TEXT("OC_SG1"))' in profiles and 'TEXT(""), true }' in profiles,
    "Remington manual-action remains required/fail-visible",
)

req(SKELETAL_OBJECT in validator, "runtime validator does not require skeletal Remington")
req(
    'TEXT("Remington 870")' in validator and 'EExpectedWeaponMeshKind::Skeletal' in validator,
    "runtime validator does not classify Remington as skeletal",
)
req(OLD_STATIC_OBJECT not in validator, "runtime validator still accepts obsolete static Remington")

for needle in (
    'FindProductionSkeletalWeaponVisual',
    'LoadObject<UAnimSequence>',
    'ManualActionSequence->GetSkeleton() != Weapon->GetSkeletalMeshAsset()->GetSkeleton()',
    'PlayWeaponAnimation(*Weapon, ManualActionSequence, State, ResetDelay)',
    'IsActionCycling()',
    'PASS45_MANUAL_ACTION_AUTHORED_SOURCE_BRIDGE_READY',
    'second_gameplay_timer=0',
    'procedural_fallback=0',
):
    req(needle in presentation, f"existing manual-action bridge invariant missing: {needle}")

for needle in (
    'PASS45_IMPORT_REMINGTON870_PRODUCTION_UE58.cmd',
    'REMINGTON_STRICT_RC',
    'Remington 870 skeletal pump production intake',
    'exit /b 27',
):
    req(needle in start_here, f"START_HERE strict route missing Remington intake: {needle}")

if errors:
    print("PASS45 REMINGTON870 PRODUCTION WIRING: FAIL")
    for error in errors:
        print(f"[FAIL] {error}")
    raise SystemExit(1)

print(
    "PASS45 REMINGTON870 PRODUCTION WIRING: PASS "
    "exact_donor_derivative=1 full_weapon_single_skeletal=1 pilot_first=1 fresh_load_gate=1 "
    "runtime_owner_skeletal=1 pump_profile_wired=1 validator_skeletal=1 start_here_full_route=1 "
    "runtime_acceptance=0 item16_checked=0"
)
