#!/usr/bin/env python3
"""Static fail-closed contract for the derived Remington UE 5.8 assembly audit."""
from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parent


def read(rel: str) -> str:
    path = ROOT / rel
    if not path.is_file():
        raise SystemExit(
            "PASS45 REMINGTON870 DERIVED PUMP UE58 ASSEMBLY AUDIT: FAIL\n"
            f"[FAIL] missing file: {rel}"
        )
    return path.read_text(encoding="utf-8", errors="replace")


errors: list[str] = []


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


audit = read("PASS45_REMINGTON870_DERIVED_PUMP_UE58_ASSEMBLY_AUDIT.py")
pilot = read("PASS45_REMINGTON870_DERIVED_PUMP_UE58_PILOT.py")
launcher = read("OsterConflict/TRY_PASS45_REMINGTON870_DERIVED_PUMP_UE58_ASSEMBLY_AUDIT.cmd")
profiles = read("OsterConflict/Source/OsterConflict/Private/OCWeaponAnimationProfiles.cpp")
start_here = read("START_HERE.cmd")
production_wrapper = read("OsterConflict/PASS45_IMPORT_REMINGTON870_PRODUCTION_UE58.cmd")

for needle in (
    'BASE_PILOT = "PASS45_REMINGTON870_DERIVED_PUMP_UE58_PILOT.py"',
    'pilot.main()',
    'pilot.imported_objects()',
    'PUMP_BONE = "PASS45_PumpForeEnd"',
    'PUMP_ANIMATION = "PASS45_Remington870_PumpCycle"',
    'pilot.mesh_has_bone(mesh, PUMP_BONE)',
    'pilot.bone_exists_in_animation(animation, PUMP_BONE)',
    'pilot.sample_pump_motion(animation, play_length)',
    'pilot.skeleton_path(animation) in pump_skeletons',
    'SINGLE_SKELETAL_IMPORT_CANDIDATE',
    'MULTI_SKELETAL_IMPORT_REQUIRES_ASSEMBLY_PLAN',
    'MIXED_STATIC_SKELETAL_IMPORT_REQUIRES_ASSEMBLY_PLAN',
    'production_visual_completeness": "UNPROVEN"',
    'DERIVED_PUMP_UE58_ASSEMBLY_EVIDENCE_ONLY',
    'PASS45_REMINGTON870_DERIVED_PUMP_UE58_ASSEMBLY_AUDIT_PASS',
    'saved_packages=0 production_cutover=0',
    'runtime_acceptance=0 item16_checked=0',
):
    req(needle in audit, f"assembly-audit contract missing: {needle}")

for forbidden in (
    '/Game/Production/Weapons/Remington870',
    'save_asset(',
    'save_directory(',
    'delete_asset(',
    'delete_directory(',
    'rename_assets(',
    'replace_existing", True',
    'production_visual_completeness": "PROVEN"',
    'runtime_acceptance=1',
    'item16_checked=1',
):
    req(forbidden not in audit, f"assembly audit regained forbidden mutation/acceptance: {forbidden}")

for needle in (
    'DERIVED_PUMP_UE58_IMPORT_PROOF_ONLY',
    'PUMP_BONE = "PASS45_PumpForeEnd"',
    'PUMP_ANIMATION = "PASS45_Remington870_PumpCycle"',
    'task.set_editor_property("save", False)',
    'production_cutover=0 runtime_acceptance=0 item16_checked=0',
):
    req(needle in pilot, f"base derived UE58 pilot prerequisite drifted: {needle}")

for needle in (
    'OsterConflictPass45Commandlet.uproject',
    'PASS45_REMINGTON870_DERIVED_PUMP_UE58_ASSEMBLY_AUDIT.py',
    'UnrealEditor-Cmd.exe',
    '-run=pythonscript',
    '-unattended -nop4 -nosplash -nullrhi',
    'PASS45_REMINGTON870_DERIVED_PUMP_UE58_ASSEMBLY_AUDIT_PASS',
    'production_visual_completeness=UNPROVEN',
    'production_cutover=0 runtime_acceptance=0 item16_checked=0',
    'ASSEMBLY EVIDENCE ONLY',
):
    req(needle in launcher, f"assembly-audit launcher contract missing: {needle}")

for forbidden in (
    'git lfs pull',
    'git checkout',
    'git reset',
    'git clean',
    'OsterConflict.uproject" -run=pythonscript',
):
    req(forbidden not in launcher, f"assembly-audit launcher regained forbidden mutation/runtime host: {forbidden}")

# The source profile may now point at the canonical PumpCycle because the normal strict route is fail-closed:
# START_HERE runs the isolated pilot first, then production import/fresh-load, and only then allows gameplay.
# This static wiring is not runtime acceptance and item 16 remains open until the local UE 5.8 route proves it.
pump_object = "/Game/Production/Weapons/Remington870/AN_Remington870_PumpCycle.AN_Remington870_PumpCycle"
req(pump_object in profiles,
    "Remington gameplay profile is not wired to the canonical gated PumpCycle")
req('PASS45_IMPORT_REMINGTON870_PRODUCTION_UE58.cmd' in start_here,
    "START_HERE strict route no longer owns Remington production intake")
req('call "%PILOT%"' in production_wrapper,
    "Remington production wrapper no longer requires isolated UE 5.8 pilot before import")
req('Production import заборонено' in production_wrapper,
    "Remington production wrapper no longer fails closed when isolated pilot fails")

if errors:
    print("PASS45 REMINGTON870 DERIVED PUMP UE58 ASSEMBLY AUDIT: FAIL")
    for error in errors:
        print(f"[FAIL] {error}")
    raise SystemExit(1)

print(
    "PASS45 REMINGTON870 DERIVED PUMP UE58 ASSEMBLY AUDIT: PASS "
    "static_contract=1 base_pilot_reused=1 assembly_inventory=1 shared_skeleton_inventory=1 "
    "gated_profile_wiring=1 ue58_execution_pending=1 production_visual_completeness=UNPROVEN "
    "runtime_acceptance=0 item16_checked=0"
)
