#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent


def read(rel: str) -> str:
    path = ROOT / rel
    if not path.is_file():
        raise SystemExit(f"PASS45 REMINGTON870 UE58 IMPORT PILOT: FAIL\n[FAIL] missing file: {rel}")
    return path.read_text(encoding="utf-8", errors="replace")


errors: list[str] = []


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


pilot = read("PASS45_REMINGTON870_UE58_IMPORT_PILOT.py")
launcher = read("OsterConflict/TRY_PASS45_REMINGTON870_UE58_IMPORT_PILOT.cmd")
manifest = read("SOURCE_ASSETS/PASS45/Remington870/MANIFEST.json")
uproject = read("OsterConflict/OsterConflict.uproject")
animation_profiles = read("OsterConflict/Source/OsterConflict/Private/OCWeaponAnimationProfiles.cpp")
presentation = read("OsterConflict/Source/OsterConflict/Private/OCFirstPersonWeaponPresentationSubsystem.cpp")

for needle in (
    '"EngineAssociation": "5.8"',
    '"Name": "PythonScriptPlugin"',
    '"Name": "EditorScriptingUtilities"',
):
    req(needle in uproject, f"UE editor import prerequisite missing from project: {needle}")

for needle in (
    'EXPECTED_ENGINE_PREFIX = "5.8"',
    'EXPECTED_SOURCE_BYTES = 20621580',
    'EXPECTED_SOURCE_SHA256 = "147aa6a0b167ba3f0806ad19a1cb6cc8790a0d541079f54d2e0fa8cf829954a2"',
    'EXPECTED_DONOR_ANIMATIONS = 5',
    'EXPECTED_DONOR_SKINS = 4',
    'EXPECTED_DONOR_NODES = 109',
    'EXPECTED_DONOR_MESHES = 6',
    '2: ("fire", 71)',
    '3: ("easy_reload", 71)',
    '4: ("full_reload", 72)',
    'SOURCE_ASSETS/PASS45/Remington870/remington_870_8siandude_ccby4.glb',
    'SOURCE_ASSETS/PASS45/Remington870/MANIFEST.json',
    'PILOT_DESTINATION = "/Game/PASS45/ImportPilots/Remington870_147aa6a0"',
    'struct.unpack("<4sII", header)',
    'magic != b"glTF"',
    'version != 2',
    'donor_structure_drift=1',
    'donor_action_channel_drift=1',
    'manifest_action_channel_drift=1',
    'unreal.SystemLibrary.get_engine_version()',
    'version https://git-lfs.github.com/spec/v1',
    'run_git_lfs_pull_before_ue_import=1',
    'unreal.AssetImportTask()',
    'task.set_editor_property("automated", True)',
    'task.set_editor_property("async", False)',
    'task.set_editor_property("save", False)',
    'task.set_editor_property("replace_existing", False)',
    'task.set_editor_property("replace_existing_settings", False)',
    'unreal.AssetToolsHelpers.get_asset_tools()',
    'asset_tools.import_asset_tasks([task])',
    'skeletal_meshes: list[object] = []',
    'animation_assets: list[object] = []',
    'len(animation_assets) < EXPECTED_DONOR_ANIMATIONS',
    'donor_animation_set_not_preserved=1',
    'shared_skeleton_paths = mesh_skeleton_paths & animation_skeleton_paths',
    'shared_skeleton_missing=1',
    'positive_length_animations < EXPECTED_DONOR_ANIMATIONS',
    'nonempty_animation_set_not_preserved=1',
    'PASS45_REMINGTON870_UE58_IMPORT_PILOT_ANIMATION',
    'donor_action_channels=71/71/72',
    'imported_animation_set_preserved=1',
    'destructive_cleanup_refused=1',
    'PASS45_REMINGTON870_UE58_IMPORT_PILOT_PASS',
    'saved_packages=0 production_cutover=0 runtime_acceptance=0 item16_checked=0',
):
    req(needle in pilot, f"fail-closed UE58 Remington import-pilot contract missing: {needle}")

for forbidden in (
    '/Game/Production/Weapons/Remington870',
    'save_directory(',
    'save_asset(',
    'delete_directory(',
    'delete_asset(',
    'replace_existing", True',
    'runtime_acceptance=1',
    'item16_checked=1',
):
    req(forbidden not in pilot, f"isolated import pilot regained forbidden mutation/acceptance behavior: {forbidden}")

for needle in (
    'OsterConflictPass45Commandlet.uproject',
    'PASS45_REMINGTON870_UE58_IMPORT_PILOT.py',
    'remington_870_8siandude_ccby4.glb',
    'UnrealEditor-Cmd.exe',
    '-run=pythonscript',
    '-unattended -nop4 -nosplash -nullrhi',
    'No automatic working-tree mutation is performed here.',
    'PASS45_REMINGTON870_UE58_IMPORT_PILOT_PASS',
    'donor_action_channels=71/71/72',
    'imported_animation_set_preserved=1',
    'production_cutover=0 runtime_acceptance=0 item16_checked=0',
    'IMPORT PILOT ONLY',
):
    req(needle in launcher, f"local UE58 Remington pilot launcher contract missing: {needle}")

for forbidden in (
    'git lfs pull',
    'git checkout',
    'git reset',
    'git clean',
    'OsterConflict.uproject" -run=pythonscript',
):
    req(forbidden not in launcher, f"Remington pilot launcher regained forbidden working-tree/runtime-host behavior: {forbidden}")

for needle in (
    '"status": "APPROVED_FOR_UE_IMPORT"',
    '"source_bytes": 20621580',
    '"source_sha256": "147aa6a0b167ba3f0806ad19a1cb6cc8790a0d541079f54d2e0fa8cf829954a2"',
    '"donor_animation_count": 5',
    '"donor_skin_count": 4',
    '"donor_node_count": 109',
    '"donor_mesh_count": 6',
    '"easy_reload_index_3": 71',
    '"fire_index_2": 71',
    '"full_reload_index_4": 72',
    '"rigged_or_articulated": true',
    '"animation_capable": true',
    '"runtime_ready": false',
    '"ue58_import_pending": true',
    '"item16_checked": false',
):
    req(needle in manifest, f"Remington manifest no longer matches isolated import-pilot assumptions: {needle}")

# Reuse-first: the pilot must feed the existing authored UAnimSequence bridge, not create a second gameplay/action owner.
for needle in (
    'FName(TEXT("OC_SG1")), TEXT(""), TEXT(""), true, TEXT(""), true',
    'LoadObject<UAnimSequence>',
    'PlayWeaponAnimation(*Weapon, ManualActionSequence, State, ResetDelay)',
    'IsActionCycling()',
    'second_gameplay_timer=0',
    'procedural_fallback=0',
):
    req(needle in animation_profiles + presentation,
        f"existing authored manual-action bridge/reuse-first invariant missing: {needle}")

if errors:
    print("PASS45 REMINGTON870 UE58 IMPORT PILOT: FAIL")
    for error in errors:
        print(f"[FAIL] {error}")
    raise SystemExit(1)

print(
    "PASS45 REMINGTON870 UE58 IMPORT PILOT: PASS "
    "static_contract=1 donor_action_set_pinned=1 shared_skeleton_gate=1 local_launcher_guarded=1 "
    "ue58_execution_pending=1 production_cutover=0 runtime_acceptance=0 item16_checked=0"
)
