#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
MOTION_PILOT = ROOT / "PASS45_REMINGTON870_UE58_IMPORTED_MOTION_PILOT.py"
BASE_PILOT = ROOT / "PASS45_REMINGTON870_UE58_IMPORT_PILOT.py"
LAUNCHER = ROOT / "OsterConflict/TRY_PASS45_REMINGTON870_UE58_IMPORTED_MOTION_PILOT.cmd"
DONOR_AUDIT = ROOT / "_DOCS/PASS45_REMINGTON870_DONOR_MOTION_AUDIT_2026-09-01.md"

errors: list[str] = []


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS45 REMINGTON870 UE58 IMPORTED MOTION PILOT: FAIL\n[FAIL] missing file: {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


motion = read(MOTION_PILOT)
base = read(BASE_PILOT)
launcher = read(LAUNCHER)
audit = read(DONOR_AUDIT)

for needle in (
    'BASE_PILOT = "PASS45_REMINGTON870_UE58_IMPORT_PILOT.py"',
    'REQUIRED_IMPORTED_MOTION_BONES = ("PBody_058", "Pmag_061")',
    'AUDIT_ONLY_BONES = ("Rif_059", "Trigger_060")',
    'base.main()',
    'base.imported_objects_from_destination()',
    'unreal.AnimationLibrary.get_animation_track_names(animation)',
    'animation_library.does_bone_name_exist(animation, unreal_name)',
    'animation_library.get_bone_pose_for_time(',
    'first.is_near_equal(',
    'required_weapon_side_tracks_not_preserved=1',
    'required_weapon_side_motion_not_preserved=1',
    'PASS45_REMINGTON870_UE58_IMPORTED_MOTION_PILOT_PASS',
    'pump_node_identity=UNPROVEN standalone_pump_clip=UNPROVEN',
    'visual_inspection_required=1 saved_packages=0 production_cutover=0',
    'runtime_acceptance=0 item16_checked=0',
):
    req(needle in motion, f"imported-motion fail-closed contract missing: {needle}")

# Reuse-first: the motion proof must consume the existing import pilot rather than
# creating a parallel donor-validation/import owner.
for forbidden in (
    'unreal.AssetImportTask()',
    'import_asset_tasks([',
    '/Game/Production/Weapons/Remington870',
    'save_asset(',
    'save_directory(',
    'delete_asset(',
    'delete_directory(',
    'runtime_acceptance=1',
    'item16_checked=1',
):
    req(forbidden not in motion, f"motion pilot regained forbidden duplicate/mutation/acceptance behavior: {forbidden}")

for needle in (
    'PILOT_DESTINATION = "/Game/PASS45/ImportPilots/Remington870_147aa6a0"',
    'task.set_editor_property("save", False)',
    'task.set_editor_property("replace_existing", False)',
    'PASS45_REMINGTON870_UE58_IMPORT_PILOT_PASS',
    'saved_packages=0 production_cutover=0 runtime_acceptance=0 item16_checked=0',
):
    req(needle in base, f"base isolated import pilot invariant missing: {needle}")

for needle in (
    'OsterConflictPass45Commandlet.uproject',
    'PASS45_REMINGTON870_UE58_IMPORTED_MOTION_PILOT.py',
    'UnrealEditor-Cmd.exe',
    '-run=pythonscript',
    '-unattended -nop4 -nosplash -nullrhi',
    'No automatic working-tree mutation is performed here.',
    'PASS45_REMINGTON870_UE58_IMPORT_PILOT_PASS',
    'PASS45_REMINGTON870_UE58_IMPORTED_MOTION_PILOT_PASS',
    'pbody_track_preserved=1 pbody_motion_preserved=1',
    'pmag_track_preserved=1 pmag_motion_preserved=1',
    'pump_node_identity=UNPROVEN standalone_pump_clip=UNPROVEN',
    'runtime_acceptance=0 item16_checked=0',
):
    req(needle in launcher, f"local imported-motion launcher contract missing: {needle}")

for forbidden in (
    'git lfs pull',
    'git checkout',
    'git reset',
    'git clean',
    'OsterConflict.uproject" -run=pythonscript',
):
    req(forbidden not in launcher, f"imported-motion launcher regained forbidden working-tree/runtime-host behavior: {forbidden}")

for needle in (
    'ARTICULATED_RELOAD_MOTION_PROVEN / PUMP_NODE_IDENTITY_UNPROVEN / STANDALONE_PUMP_CLIP_UNPROVEN',
    '`PBody_058`',
    '`Pmag_061`',
    'UE 5.8 preserves the needed hierarchy/track semantics after import',
    'existing local UE 5.8 isolated import/visual inspection',
):
    req(needle in audit, f"donor-motion audit invariant missing: {needle}")

if errors:
    print("PASS45 REMINGTON870 UE58 IMPORTED MOTION PILOT: FAIL")
    for error in errors:
        print(f"[FAIL] {error}")
    raise SystemExit(1)

print(
    "PASS45 REMINGTON870 UE58 IMPORTED MOTION PILOT: PASS "
    "reuse_existing_import_pilot=1 named_track_gate=1 imported_pose_motion_gate=1 "
    "local_launcher_guarded=1 pump_identity_unproven=1 visual_inspection_required=1 "
    "ue58_execution_pending=1 production_cutover=0 runtime_acceptance=0 item16_checked=0"
)
