#!/usr/bin/env python3
"""Static fail-closed contract for the isolated derived Remington UE 5.8 pilot."""
from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parent


def read(rel: str) -> str:
    path = ROOT / rel
    if not path.is_file():
        raise SystemExit(f"PASS45 REMINGTON870 DERIVED PUMP UE58 PILOT: FAIL\n[FAIL] missing file: {rel}")
    return path.read_text(encoding="utf-8", errors="replace")


errors: list[str] = []


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


pilot = read("PASS45_REMINGTON870_DERIVED_PUMP_UE58_PILOT.py")
derived = read("PASS45_REMINGTON870_DERIVED_PUMP_SOURCE.py")
derived_verify = read("VERIFY_PASS45_REMINGTON870_DERIVED_PUMP_SOURCE.py")
launcher = read("OsterConflict/TRY_PASS45_REMINGTON870_DERIVED_PUMP_UE58_PILOT.cmd")
uproject = read("OsterConflict/OsterConflict.uproject")
profiles = read("OsterConflict/Source/OsterConflict/Private/OCWeaponAnimationProfiles.cpp")
presentation = read("OsterConflict/Source/OsterConflict/Private/OCFirstPersonWeaponPresentationSubsystem.cpp")

for needle in (
    '"EngineAssociation": "5.8"',
    '"Name": "PythonScriptPlugin"',
    '"Name": "EditorScriptingUtilities"',
):
    req(needle in uproject, f"UE editor scripting prerequisite missing: {needle}")

for needle in (
    'EXPECTED_ENGINE_PREFIX = "5.8"',
    'EXPECTED_SOURCE_SHA256 = "147aa6a0b167ba3f0806ad19a1cb6cc8790a0d541079f54d2e0fa8cf829954a2"',
    'EXPECTED_SOURCE_BYTES = 20621580',
    'EXPECTED_FORE_END_VERTICES = 1170',
    'EXPECTED_SIDE_SADDLE_VERTICES = 3241',
    'EXPECTED_PUMP_DURATION = 0.55',
    'PUMP_BONE = "PASS45_PumpForeEnd"',
    'PUMP_ANIMATION = "PASS45_Remington870_PumpCycle"',
    'PILOT_DESTINATION = "/Game/PASS45/ImportPilots/Remington870DerivedPump"',
    'PASS45_REMINGTON870_DERIVED_PUMP_SOURCE as derived',
    'derived.build_derived(source.read_bytes())',
    'low_y_vertex_count',
    'high_y_vertex_count',
    'unreal.AssetImportTask()',
    'task.set_editor_property("automated", True)',
    'task.set_editor_property("save", False)',
    'task.set_editor_property("replace_existing", False)',
    'task.set_editor_property("replace_existing_settings", False)',
    'asset_tools.import_asset_tasks([task])',
    'mesh_has_bone(mesh, PUMP_BONE)',
    'AnimationLibrary.does_bone_name_exist',
    'AnimationLibrary.get_bone_pose_for_time',
    'max_delta > MIN_TRANSLATION_DELTA',
    'shared_skeleton_preserved',
    'standalone_pump_sequence_preserved',
    'source_partition_verified',
    'DERIVED_PUMP_UE58_IMPORT_PROOF_ONLY',
    'PASS45_REMINGTON870_DERIVED_PUMP_UE58_PILOT_PASS',
    'saved_packages=0 production_cutover=0 runtime_acceptance=0 item16_checked=0',
):
    req(needle in pilot, f"derived UE58 pilot contract missing: {needle}")

for forbidden in (
    '/Game/Production/Weapons/Remington870',
    'save_asset(',
    'save_directory(',
    'delete_asset(',
    'delete_directory(',
    'replace_existing", True',
    'runtime_acceptance=1',
    'item16_checked=1',
):
    req(forbidden not in pilot, f"isolated derived UE58 pilot regained forbidden mutation/acceptance: {forbidden}")

for needle in (
    'PASS45_PumpForeEnd',
    'PASS45_Remington870_PumpCycle',
    'EXPECTED_LOW_VERTEX_COUNT = 1170',
    'EXPECTED_HIGH_VERTEX_COUNT = 3241',
    'PUMP_STROKE_Y = -0.537790',
    'PUMP_DURATION = 0.55',
    'direct_pmag_as_pump_mapping',
    'production_cutover": False',
    'runtime_acceptance": False',
    'item16_checked": False',
):
    req(needle in derived, f"derived source prerequisite drifted: {needle}")

for needle in (
    'new_owned != derived.EXPECTED_LOW_VERTEX_COUNT',
    'old_owned != derived.EXPECTED_HIGH_VERTEX_COUNT',
    'derived pump animation introduced X/Z motion',
    'isolated_ue58_import_pending=1 production_cutover=0 runtime_acceptance=0 item16_checked=0',
):
    req(needle in derived_verify, f"derived source independent verifier prerequisite missing: {needle}")

for needle in (
    'OsterConflictPass45Commandlet.uproject',
    'PASS45_REMINGTON870_DERIVED_PUMP_UE58_PILOT.py',
    'remington_870_8siandude_ccby4.glb',
    'UnrealEditor-Cmd.exe',
    '-run=pythonscript',
    '-unattended -nop4 -nosplash -nullrhi',
    'No automatic working-tree mutation is performed here.',
    'PASS45_REMINGTON870_DERIVED_PUMP_UE58_PILOT_PASS',
    'pump_bone_addressable=1',
    'pump_motion_preserved=1',
    'shared_skeleton_preserved=1',
    'standalone_pump_sequence_preserved=1',
    'classification=DERIVED_PUMP_UE58_IMPORT_PROOF_ONLY',
    'production_cutover=0 runtime_acceptance=0 item16_checked=0',
):
    req(needle in launcher, f"derived UE58 launcher contract missing: {needle}")

for forbidden in (
    'git lfs pull',
    'git checkout',
    'git reset',
    'git clean',
    'OsterConflict.uproject" -run=pythonscript',
):
    req(forbidden not in launcher, f"derived UE58 launcher regained forbidden mutation/runtime host: {forbidden}")

# Until this isolated engine proof passes locally, the gameplay bridge must stay fail-closed.
req(
    'FName(TEXT("OC_SG1")), TEXT(""), TEXT(""), true, TEXT(""), true' in profiles,
    "Remington gameplay profile was cut over before derived UE58 isolated acceptance",
)
for needle in (
    'LoadObject<UAnimSequence>',
    'PlayWeaponAnimation(*Weapon, ManualActionSequence, State, ResetDelay)',
    'IsActionCycling()',
    'second_gameplay_timer=0',
    'procedural_fallback=0',
):
    req(needle in presentation, f"existing authored manual-action bridge/reuse-first invariant missing: {needle}")

if errors:
    print("PASS45 REMINGTON870 DERIVED PUMP UE58 PILOT: FAIL")
    for error in errors:
        print(f"[FAIL] {error}")
    raise SystemExit(1)

print(
    "PASS45 REMINGTON870 DERIVED PUMP UE58 PILOT: PASS "
    "static_contract=1 exact_donor_derivative=1 fore_end_partition=1170 side_saddle_partition=3241 "
    "bone_addressability_gate=1 sampled_motion_gate=1 shared_skeleton_gate=1 standalone_sequence_gate=1 "
    "ue58_execution_pending=1 production_cutover=0 runtime_acceptance=0 item16_checked=0"
)
