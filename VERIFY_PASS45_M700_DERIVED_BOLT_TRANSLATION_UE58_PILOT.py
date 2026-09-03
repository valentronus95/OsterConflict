#!/usr/bin/env python3
"""Static fail-closed contract for the isolated M700 UE 5.8 bolt translation pilot."""
from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parent


def read(rel: str) -> str:
    path = ROOT / rel
    if not path.is_file():
        raise SystemExit(f"PASS45 M700 DERIVED UE58 PILOT: FAIL\n[FAIL] missing file: {rel}")
    return path.read_text(encoding="utf-8", errors="replace")


errors: list[str] = []


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


pilot = read("PASS45_M700_DERIVED_BOLT_TRANSLATION_UE58_PILOT.py")
derived = read("PASS45_M700_DERIVED_BOLT_TRANSLATION_SOURCE.py")
derived_verify = read("VERIFY_PASS45_M700_DERIVED_BOLT_TRANSLATION_SOURCE.py")
motion_audit = read("_DOCS/PASS45_M700_SOURCE_MOTION_AUDIT_2026-09-02.json")
geometry_audit = read("_DOCS/PASS45_M700_BOLT_GEOMETRY_AUDIT_2026-09-02.json")
launcher = read("OsterConflict/TRY_PASS45_M700_DERIVED_BOLT_TRANSLATION_UE58_PILOT.cmd")
uproject = read("OsterConflict/OsterConflictPass45Commandlet.uproject")
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
    'EXPECTED_SOURCE_SHA256 = "b7e003e01be8441e452730bc06c38c5e9752e523ae1b401ed2a6cc6cdca16840"',
    'EXPECTED_SOURCE_BYTES = 638732',
    'BOLT_BONE = "BOLT"',
    'PILOT_DESTINATION = "/Game/PASS45/ImportPilots/M700DerivedBoltTranslation"',
    'PILOT_SEQUENCE_NAME = "AN_PASS45_M700_BoltTranslation_Pilot"',
    'FRAME_RATE = 20',
    'FRAME_COUNT = 22',
    'unreal.FbxImportUI()',
    '"import_as_skeletal": True',
    '"import_animations": False',
    'task.set_editor_property("save", False)',
    'task.set_editor_property("replace_existing", False)',
    'get_ref_pose_transform',
    'unreal.AnimSequenceFactory()',
    'factory.set_editor_property("target_skeleton", skeleton)',
    'data.add_bone_track(unreal.Name(BOLT_BONE), False)',
    'data.set_bone_track_keys(',
    'unreal.AnimationLibrary.get_bone_pose_for_time(',
    'M700_BOLT_TRANSLATION_DERIVED_UE58_MOTION_PROOF_ONLY',
    '"pilot_travel_accepted": False',
    '"source_authored_endpoint": False',
    '"bolt_stop_used_as_endpoint": False',
    '"rotation_channel_authored": False',
    '"rotation_calibration_pending": True',
    '"saved_packages": False',
    '"production_profile_changed": False',
    '"production_cutover": False',
    '"runtime_visual_acceptance": False',
    '"runtime_acceptance": False',
    '"item16_checked": False',
    'PASS45_M700_DERIVED_BOLT_TRANSLATION_UE58_PILOT_PASS',
    'source_authored_endpoint=0 bolt_stop_used_as_endpoint=0 pilot_travel_accepted=0',
    'rotation_calibration_pending=1 saved_packages=0 production_profile_changed=0',
):
    req(needle in pilot, f"M700 UE58 pilot contract missing: {needle}")

for forbidden in (
    '/Game/Production/Weapons/M700',
    'EditorAssetLibrary.save_asset(',
    'EditorAssetLibrary.save_loaded_asset(',
    'EditorAssetLibrary.save_directory(',
    'delete_asset(',
    'delete_directory(',
    'replace_existing", True',
    'runtime_acceptance=1',
    'runtime_visual_acceptance=1',
    'item16_checked=1',
):
    req(forbidden not in pilot, f"isolated M700 pilot regained forbidden mutation/acceptance: {forbidden}")

for needle in (
    'BOLT_NODE_NAME = "BOLT"',
    'BOLT_STOP_NODE_NAME = "BOLT_STOP"',
    'DERIVED_ANIMATION_NAME = "PASS45_M700_BoltTranslationPilot"',
    'CYCLE_DURATION = 1.10',
    'PILOT_TRAVEL_FRACTION_OF_Y_EXTENT = 0.20',
    '"bolt_stop_used_as_endpoint": False',
    '"source_authored_endpoint": False',
    '"pilot_travel_accepted": False',
    '"rotation_channel_authored": False',
    '"rotation_calibration_pending": True',
    '"production_cutover": False',
    '"runtime_acceptance": False',
    '"item16_checked": False',
):
    req(needle in derived, f"M700 derived-source prerequisite drifted: {needle}")

for needle in (
    'bolt_stop_animated=0',
    'source_authored_endpoint=0',
    'pilot_travel_accepted=0',
    'rotation_calibration_pending=1',
    'production_cutover=0 runtime_acceptance=0 item16_checked=0',
):
    req(needle in derived_verify, f"M700 derived verifier prerequisite missing: {needle}")

for needle in (
    '"animation_count": 0',
    '"BOLT": {',
    '"weighted_vertex_count": 1317',
    '"BOLT_STOP": {',
    '"weighted_vertex_count": 60',
    '"runtime_acceptance": false',
    '"item16_checked": false',
):
    req(needle in motion_audit, f"M700 motion audit prerequisite drifted: {needle}")

for needle in (
    '"classification": "DISTINCT_WEIGHTED_COMPONENTS_NO_SHARED_VERTICES"',
    '"shared_weighted_vertex_count": 0',
    '"target_weighted_vertices_disjoint": true',
    '"source_authored_stop_delta_safe_as_bolt_travel": false',
    '"BOLT_STOP_IS_WEIGHTED_GEOMETRY_COMPONENT_NOT_PROVEN_TRAVEL_ENDPOINT"',
):
    req(needle in geometry_audit, f"M700 geometry audit prerequisite drifted: {needle}")

for needle in (
    'OsterConflictPass45Commandlet.uproject',
    'PASS45_M700_DERIVED_BOLT_TRANSLATION_UE58_PILOT.py',
    'SKM_M700.fbx',
    'UnrealEditor-Cmd.exe',
    '-run=pythonscript',
    '-unattended -nop4 -nosplash -nullrhi',
    'No automatic working-tree mutation is performed here.',
    'PASS45_M700_DERIVED_BOLT_TRANSLATION_UE58_PILOT_PASS',
    'bolt_stop_used_as_endpoint=0',
    'pilot_travel_accepted=0',
    'rotation_calibration_pending=1',
    'saved_packages=0',
    'production_profile_changed=0',
    'runtime_acceptance=0',
    'item16_checked=0',
):
    req(needle in launcher, f"M700 UE58 launcher contract missing: {needle}")

for forbidden in (
    'git lfs pull',
    'git checkout',
    'git reset',
    'git clean',
    'save_asset',
    'save_directory',
):
    req(forbidden not in launcher, f"M700 UE58 launcher regained forbidden mutation: {forbidden}")

req('FName(TEXT("OC_SNP1"))' in profiles, "M700 gameplay profile disappeared")
req(
    '{ FName(TEXT("OC_SNP1")), TEXT(""), TEXT(""), true, TEXT(""), true }' in profiles,
    "M700 production manual-action slot is no longer fail-closed/empty",
)
for forbidden in (
    '/Game/PASS45/ImportPilots/M700DerivedBoltTranslation',
    'AN_PASS45_M700_BoltTranslation_Pilot',
    'PASS45_M700_BoltTranslationPilot',
):
    req(forbidden not in profiles, f"pilot/derived M700 path leaked into production profile: {forbidden}")

for needle in (
    'LoadObject<UAnimSequence>',
    'PlayWeaponAnimation(*Weapon, ManualActionSequence, State, ResetDelay)',
    'IsActionCycling()',
    'second_gameplay_timer=0',
    'procedural_fallback=0',
):
    req(needle in presentation, f"existing authored manual-action bridge invariant missing: {needle}")

if errors:
    print("PASS45 M700 DERIVED UE58 PILOT: FAIL")
    for error in errors:
        print(f"[FAIL] {error}")
    raise SystemExit(1)

print(
    "PASS45 M700 DERIVED UE58 PILOT: PASS "
    "static_contract=1 exact_stein_source=1 bolt_weighted_vertices=1317 bolt_stop_weighted_vertices=60 "
    "disjoint_geometry=1 bolt_stop_endpoint_rejected=1 ue_ref_pose_gate=1 controller_bone_track_gate=1 "
    "sampled_motion_gate=1 production_profile_unchanged=1 saved_packages=0 production_cutover=0 "
    "runtime_visual_acceptance=0 runtime_acceptance=0 item16_checked=0"
)
