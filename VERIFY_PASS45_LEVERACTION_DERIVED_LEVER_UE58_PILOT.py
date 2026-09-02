#!/usr/bin/env python3
"""Static fail-closed contract for the isolated Lever Action UE 5.8 motion pilot."""
from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parent


def read(rel: str) -> str:
    path = ROOT / rel
    if not path.is_file():
        raise SystemExit(f"PASS45 LEVER ACTION DERIVED UE58 PILOT: FAIL\n[FAIL] missing file: {rel}")
    return path.read_text(encoding="utf-8", errors="replace")


errors: list[str] = []


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


pilot = read("PASS45_LEVERACTION_DERIVED_LEVER_UE58_PILOT.py")
derived = read("PASS45_LEVERACTION_DERIVED_LEVER_SOURCE.py")
derived_verify = read("VERIFY_PASS45_LEVERACTION_DERIVED_LEVER_SOURCE.py")
audit = read("_DOCS/PASS45_LEVERACTION_SOURCE_MOTION_AUDIT_2026-09-02.json")
launcher = read("OsterConflict/TRY_PASS45_LEVERACTION_DERIVED_LEVER_UE58_PILOT.cmd")
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
    'EXPECTED_SOURCE_SHA256 = "b2bf25bd47e9c4f6404897f67ad2a76a02971365fb7a689761936891d4591c69"',
    'EXPECTED_SOURCE_BYTES = 570332',
    'LEVER_BONE = "LEVER"',
    'PILOT_DESTINATION = "/Game/PASS45/ImportPilots/LeverActionDerivedCycle"',
    'PILOT_SEQUENCE_NAME = "AN_PASS45_LeverAction_Cycle_Pilot"',
    'CYCLE_DURATION = 0.85',
    'FRAME_RATE = 20',
    'FRAME_COUNT = 17',
    'PILOT_MAX_ANGLE_DEG = -45.0',
    'unreal.FbxImportUI()',
    '"import_as_skeletal": True',
    '"import_animations": False',
    'task.set_editor_property("save", False)',
    'task.set_editor_property("replace_existing", False)',
    'task.set_editor_property("replace_existing_settings", False)',
    'component.get_bone_index(unreal.Name(bone_name))',
    'get_ref_pose_transform',
    'unreal.AnimSequenceFactory()',
    'factory.set_editor_property("target_skeleton", skeleton)',
    'controller.set_frame_rate(frame_rate, False)',
    'controller.set_number_of_frames(unreal.FrameNumber(value=FRAME_COUNT), False)',
    'controller.add_bone_track(unreal.Name(LEVER_BONE), False)',
    'controller.set_bone_track_keys(',
    'unreal.AnimationLibrary.get_bone_pose_for_time(',
    'LEVERACTION_DERIVED_UE58_MOTION_PROOF_ONLY',
    '"source_authored_animation": False',
    '"source_authored_endpoint": False',
    '"pilot_angle_accepted": False',
    '"saved_packages": False',
    '"production_profile_changed": False',
    '"production_cutover": False',
    '"runtime_visual_acceptance": False',
    '"runtime_acceptance": False',
    '"item16_checked": False',
    'PASS45_LEVERACTION_DERIVED_LEVER_UE58_PILOT_PASS',
    'source_authored_endpoint=0 pilot_angle_accepted=0 saved_packages=0',
    'production_profile_changed=0 production_cutover=0',
    'runtime_visual_acceptance=0 runtime_acceptance=0 item16_checked=0',
):
    req(needle in pilot, f"Lever UE58 pilot contract missing: {needle}")

for forbidden in (
    '/Game/Production/Weapons/Lever',
    'EditorAssetLibrary.save_asset(',
    'EditorAssetLibrary.save_loaded_asset(',
    'EditorAssetLibrary.save_directory(',
    'EditorLoadingAndSavingUtils.save',
    'delete_asset(',
    'delete_directory(',
    'replace_existing", True',
    'runtime_acceptance=1',
    'runtime_visual_acceptance=1',
    'item16_checked=1',
):
    req(forbidden not in pilot, f"isolated Lever UE58 pilot regained forbidden mutation/acceptance: {forbidden}")

for needle in (
    'EXPECTED_SOURCE_SHA256 = "b2bf25bd47e9c4f6404897f67ad2a76a02971365fb7a689761936891d4591c69"',
    'LEVER_NODE_NAME = "LEVER"',
    'DERIVED_ANIMATION_NAME = "PASS45_LeverAction_Cycle"',
    'CYCLE_DURATION = 0.85',
    'PILOT_MAX_ANGLE_DEG = -45.0',
    '"source_authored_motion": False',
    '"source_authored_endpoint": False',
    '"pilot_angle_accepted": False',
    '"second_gameplay_timer": False',
    '"production_cutover": False',
    '"runtime_acceptance": False',
    '"item16_checked": False',
):
    req(needle in derived, f"Lever derived-source prerequisite drifted: {needle}")

for needle in (
    'EXPECTED_WEIGHTED_VERTICES = 964',
    'shared.get("LEVER+HAMMER"',
    'shared.get("LEVER+BOLT"',
    'source-authored endpoint: false',
    'production_cutover=0 runtime_acceptance=0 item16_checked=0',
):
    req(needle in derived_verify, f"Lever derived independent verifier prerequisite missing: {needle}")

for needle in (
    '"animation_count": 0',
    '"lever_joint_has_weighted_geometry": true',
    '"source_authored_lever_angle_or_endpoint": false',
    '"LEVER": {',
    '"weighted_vertex_count": 964',
    '"LEVER+HAMMER": 0',
    '"LEVER+BOLT": 0',
    '"runtime_acceptance": false',
    '"item16_checked": false',
):
    req(needle in audit, f"Lever source-audit prerequisite drifted: {needle}")

for needle in (
    'OsterConflictPass45Commandlet.uproject',
    'PASS45_LEVERACTION_DERIVED_LEVER_UE58_PILOT.py',
    'SKM_LeverAction.fbx',
    'UnrealEditor-Cmd.exe',
    '-run=pythonscript',
    '-unattended -nop4 -nosplash -nullrhi',
    'No automatic working-tree mutation is performed here.',
    'PASS45_LEVERACTION_DERIVED_LEVER_UE58_PILOT_PASS',
    'source_authored_endpoint=0',
    'pilot_angle_accepted=0',
    'saved_packages=0',
    'production_profile_changed=0',
    'production_cutover=0',
    'runtime_visual_acceptance=0',
    'runtime_acceptance=0',
    'item16_checked=0',
):
    req(needle in launcher, f"Lever UE58 launcher contract missing: {needle}")

for forbidden in (
    'git lfs pull',
    'git checkout',
    'git reset',
    'git clean',
    'save_asset',
    'save_directory',
):
    req(forbidden not in launcher, f"Lever UE58 launcher regained forbidden mutation: {forbidden}")

# Pilot path must remain completely outside the production weapon profile until direct UE proof is reviewed.
req(
    'R13_LEVER4570' in profiles,
    "LeverAction gameplay profile disappeared",
)
req(
    '{ FName(TEXT("R13_LEVER4570")), TEXT(""), TEXT(""), true, TEXT(""), true }' in profiles,
    "LeverAction production manual-action slot is no longer fail-closed/empty",
)
for forbidden in (
    '/Game/PASS45/ImportPilots/LeverActionDerivedCycle',
    'AN_PASS45_LeverAction_Cycle_Pilot',
    'PASS45_LeverAction_Cycle',
):
    req(forbidden not in profiles, f"pilot/derived Lever path leaked into production profile: {forbidden}")

for needle in (
    'LoadObject<UAnimSequence>',
    'PlayWeaponAnimation(*Weapon, ManualActionSequence, State, ResetDelay)',
    'IsActionCycling()',
    'second_gameplay_timer=0',
    'procedural_fallback=0',
):
    req(needle in presentation, f"existing authored manual-action bridge/reuse-first invariant missing: {needle}")

if errors:
    print("PASS45 LEVER ACTION DERIVED UE58 PILOT: FAIL")
    for error in errors:
        print(f"[FAIL] {error}")
    raise SystemExit(1)

print(
    "PASS45 LEVER ACTION DERIVED UE58 PILOT: PASS "
    "static_contract=1 exact_stein_source=1 lever_weighted_vertices=964 pairwise_shared_vertices=0 "
    "ue_ref_pose_gate=1 controller_bone_track_gate=1 sampled_motion_gate=1 "
    "production_profile_unchanged=1 saved_packages=0 production_cutover=0 "
    "runtime_visual_acceptance=0 runtime_acceptance=0 item16_checked=0"
)
