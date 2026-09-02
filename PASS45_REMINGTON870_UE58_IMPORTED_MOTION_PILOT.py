#!/usr/bin/env python3
"""Extend the existing isolated Remington 870 UE 5.8 import pilot with imported-motion truth.

Run inside UnrealEditor-Cmd. This script deliberately reuses the existing
PASS45_REMINGTON870_UE58_IMPORT_PILOT module instead of duplicating donor
validation/import ownership. It proves whether named weapon-side bones remain
addressable after UE 5.8 import and whether they carry non-trivial, sibling-
relative animation. It cannot prove that Pmag_061 is physically the pump,
cannot relabel a reload clip as a standalone post-shot pump cycle, does not save
packages, and does not authorize production cutover or PASS45 item-16 acceptance.
"""
from __future__ import annotations

import importlib.util
from pathlib import Path

import unreal

BASE_PILOT = "PASS45_REMINGTON870_UE58_IMPORT_PILOT.py"
REQUIRED_IMPORTED_MOTION_BONES = ("PBody_058", "Pmag_061")
REQUIRED_SIBLING_BONES = ("PBody_058", "Pmag_061")
AUDIT_ONLY_BONES = ("Rif_059", "Trigger_060")
SAMPLE_FRACTIONS = (0.0, 0.25, 0.5, 0.75, 0.999)


def fail(message: str) -> None:
    unreal.log_error(f"PASS45_REMINGTON870_UE58_IMPORTED_MOTION_PILOT_FAIL {message}")
    raise RuntimeError(message)


def load_base_pilot():
    root = Path(__file__).resolve().parent
    path = root / BASE_PILOT
    if not path.is_file():
        fail(f"base_import_pilot_missing path={path}")
    spec = importlib.util.spec_from_file_location("pass45_remington870_ue58_import_pilot", path)
    if spec is None or spec.loader is None:
        fail("base_import_pilot_module_spec_failed=1")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def bone_exists_in_animation(animation, bone_name: str) -> bool:
    """Use UE 5.8's bone-addressability API as the authoritative imported-bone gate.

    UE 5.8.1 can report imported skeletal bones as addressable and sampleable while
    get_animation_track_names() does not expose those same bone names. The latter
    remains diagnostic only; acceptance requires addressability plus sampled motion.
    """
    if not hasattr(unreal, "AnimationLibrary"):
        fail("animation_library_unavailable=1")
    return bool(
        unreal.AnimationLibrary.does_bone_name_exist(
            animation,
            unreal.Name(bone_name),
        )
    )


def sample_bone_motion(animation, bone_name: str, play_length: float) -> tuple[bool, list[str]]:
    if not bone_exists_in_animation(animation, bone_name):
        return False, []

    if play_length <= 0.0:
        return False, []

    animation_library = unreal.AnimationLibrary
    unreal_name = unreal.Name(bone_name)
    poses = []
    rows: list[str] = []
    for fraction in SAMPLE_FRACTIONS:
        sample_time = min(play_length * fraction, max(play_length - 0.0001, 0.0))
        pose = animation_library.get_bone_pose_for_time(
            animation,
            unreal_name,
            sample_time,
            False,
        )
        poses.append(pose)
        rows.append(
            f"t={sample_time:.6f} translation={pose.translation} rotation={pose.rotation}"
        )

    first = poses[0]
    moved = any(
        not first.is_near_equal(
            pose,
            location_tolerance=0.0001,
            rotation_tolerance=0.0001,
            scale3d_tolerance=0.0001,
        )
        for pose in poses[1:]
    )
    return moved, rows


def sample_sibling_relative_motion(
    animation,
    body_bone: str,
    sibling_bone: str,
    play_length: float,
) -> tuple[bool, list[str]]:
    """Compare two imported sibling bones in the same sampled pose space."""
    if not bone_exists_in_animation(animation, body_bone):
        return False, []
    if not bone_exists_in_animation(animation, sibling_bone):
        return False, []
    if play_length <= 0.0:
        return False, []

    animation_library = unreal.AnimationLibrary
    body_name = unreal.Name(body_bone)
    sibling_name = unreal.Name(sibling_bone)
    relative_poses = []
    rows: list[str] = []
    for fraction in SAMPLE_FRACTIONS:
        sample_time = min(play_length * fraction, max(play_length - 0.0001, 0.0))
        body_pose = animation_library.get_bone_pose_for_time(
            animation,
            body_name,
            sample_time,
            False,
        )
        sibling_pose = animation_library.get_bone_pose_for_time(
            animation,
            sibling_name,
            sample_time,
            False,
        )
        relative_pose = sibling_pose.make_relative(body_pose)
        relative_poses.append(relative_pose)
        rows.append(
            f"t={sample_time:.6f} translation={relative_pose.translation} "
            f"rotation={relative_pose.rotation}"
        )

    first = relative_poses[0]
    moved = any(
        not first.is_near_equal(
            pose,
            location_tolerance=0.0001,
            rotation_tolerance=0.0001,
            scale3d_tolerance=0.0001,
        )
        for pose in relative_poses[1:]
    )
    return moved, rows


def imported_sibling_parent_preserved(imported: list[object]) -> tuple[bool, list[str]]:
    """Fail closed unless at least one imported skeletal mesh preserves a shared parent."""
    rows: list[str] = []
    for mesh in imported:
        if mesh.get_class().get_name() != "SkeletalMesh":
            continue
        get_parent = getattr(mesh, "get_bone_parent", None)
        if not callable(get_parent):
            rows.append(f"mesh={mesh.get_path_name()} get_bone_parent=UNAVAILABLE")
            continue

        body_parent = str(get_parent(unreal.Name(REQUIRED_SIBLING_BONES[0])))
        sibling_parent = str(get_parent(unreal.Name(REQUIRED_SIBLING_BONES[1])))
        shared = (
            body_parent not in ("", "None")
            and sibling_parent not in ("", "None")
            and body_parent == sibling_parent
        )
        rows.append(
            f"mesh={mesh.get_path_name()} body_parent={body_parent or 'NONE'} "
            f"sibling_parent={sibling_parent or 'NONE'} shared_parent={int(shared)}"
        )
        if shared:
            return True, rows
    return False, rows


def main() -> None:
    base = load_base_pilot()
    base.main()

    imported = base.imported_objects_from_destination()
    animations = [obj for obj in imported if obj.get_class().get_name() == "AnimSequence"]
    if len(animations) < base.EXPECTED_DONOR_ANIMATIONS:
        fail(
            "imported_animation_set_unavailable_after_base_pilot=1 "
            f"expected_at_least={base.EXPECTED_DONOR_ANIMATIONS} actual={len(animations)}"
        )

    sibling_parent_preserved, parent_rows = imported_sibling_parent_preserved(imported)
    for row in parent_rows:
        unreal.log(f"PASS45_REMINGTON870_UE58_IMPORTED_MOTION_HIERARCHY {row}")
    if not sibling_parent_preserved:
        fail(
            "required_sibling_parent_not_preserved=1 "
            f"bones={','.join(REQUIRED_SIBLING_BONES)}"
        )

    required_present: dict[str, bool] = {name: False for name in REQUIRED_IMPORTED_MOTION_BONES}
    required_moved: dict[str, bool] = {name: False for name in REQUIRED_IMPORTED_MOTION_BONES}
    relative_motion_sequences: list[str] = []

    all_bones = REQUIRED_IMPORTED_MOTION_BONES + AUDIT_ONLY_BONES
    for animation in animations:
        play_length = base.animation_play_length(animation)
        reported_track_names = {
            str(name)
            for name in unreal.AnimationLibrary.get_animation_track_names(animation)
        }
        unreal.log(
            "PASS45_REMINGTON870_UE58_IMPORTED_MOTION_SEQUENCE "
            f"path={animation.get_path_name()} play_length={play_length:.6f} "
            f"reported_track_count={len(reported_track_names)}"
        )

        for bone_name in all_bones:
            reported_track_present = bone_name in reported_track_names
            bone_addressable = bone_exists_in_animation(animation, bone_name)
            moved, pose_rows = sample_bone_motion(animation, bone_name, play_length)
            if bone_name in required_present:
                required_present[bone_name] = required_present[bone_name] or bone_addressable
                required_moved[bone_name] = required_moved[bone_name] or moved

            unreal.log(
                "PASS45_REMINGTON870_UE58_IMPORTED_MOTION_BONE "
                f"sequence={animation.get_path_name()} bone={bone_name} "
                f"reported_track_present={int(reported_track_present)} "
                f"bone_addressable={int(bone_addressable)} moved={int(moved)} "
                f"samples={len(pose_rows)}"
            )
            for row in pose_rows:
                unreal.log(
                    "PASS45_REMINGTON870_UE58_IMPORTED_MOTION_SAMPLE "
                    f"sequence={animation.get_path_name()} bone={bone_name} {row}"
                )

        required_bones_in_sequence = all(
            bone_exists_in_animation(animation, bone_name)
            for bone_name in REQUIRED_SIBLING_BONES
        )
        relative_moved = False
        relative_rows: list[str] = []
        if required_bones_in_sequence:
            relative_moved, relative_rows = sample_sibling_relative_motion(
                animation,
                REQUIRED_SIBLING_BONES[0],
                REQUIRED_SIBLING_BONES[1],
                play_length,
            )
            if relative_moved:
                relative_motion_sequences.append(animation.get_path_name())

        unreal.log(
            "PASS45_REMINGTON870_UE58_IMPORTED_MOTION_RELATIVE "
            f"sequence={animation.get_path_name()} "
            f"bones_addressable={int(required_bones_in_sequence)} "
            f"moved={int(relative_moved)} samples={len(relative_rows)}"
        )
        for row in relative_rows:
            unreal.log(
                "PASS45_REMINGTON870_UE58_IMPORTED_MOTION_RELATIVE_SAMPLE "
                f"sequence={animation.get_path_name()} "
                f"body={REQUIRED_SIBLING_BONES[0]} sibling={REQUIRED_SIBLING_BONES[1]} {row}"
            )

    missing_bones = [name for name, present in required_present.items() if not present]
    if missing_bones:
        fail(
            "required_weapon_side_bones_not_addressable=1 "
            f"bones={','.join(sorted(missing_bones))}"
        )

    static_tracks = [name for name, moved in required_moved.items() if not moved]
    if static_tracks:
        fail(
            "required_weapon_side_motion_not_preserved=1 "
            f"bones={','.join(sorted(static_tracks))}"
        )

    if not relative_motion_sequences:
        fail(
            "required_sibling_relative_motion_not_preserved=1 "
            f"body={REQUIRED_SIBLING_BONES[0]} sibling={REQUIRED_SIBLING_BONES[1]}"
        )

    unreal.log(
        "PASS45_REMINGTON870_UE58_IMPORTED_MOTION_PILOT_PASS "
        "pbody_track_preserved=1 pbody_motion_preserved=1 "
        "pmag_track_preserved=1 pmag_motion_preserved=1 "
        "sibling_parent_preserved=1 relative_sibling_motion_preserved=1 "
        "track_evidence=bone_addressability_plus_pose_motion "
        f"relative_motion_sequences={len(relative_motion_sequences)} "
        "pump_node_identity=UNPROVEN standalone_pump_clip=UNPROVEN "
        "visual_inspection_required=1 saved_packages=0 production_cutover=0 "
        "runtime_acceptance=0 item16_checked=0"
    )


if __name__ == "__main__":
    main()
