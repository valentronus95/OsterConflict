#!/usr/bin/env python3
"""Extend the existing isolated Remington 870 UE 5.8 import pilot with imported-motion truth.

Run inside UnrealEditor-Cmd. This script deliberately reuses the existing
PASS45_REMINGTON870_UE58_IMPORT_PILOT module instead of duplicating donor
validation/import ownership. It proves only whether named weapon-side tracks
survive the UE 5.8 import with non-trivial animation. It cannot prove that
Pmag_061 is physically the pump, cannot relabel a reload clip as a standalone
post-shot pump cycle, does not save packages, and does not authorize production
cutover or PASS45 item-16 acceptance.
"""
from __future__ import annotations

import importlib.util
from pathlib import Path

import unreal

BASE_PILOT = "PASS45_REMINGTON870_UE58_IMPORT_PILOT.py"
REQUIRED_IMPORTED_MOTION_BONES = ("PBody_058", "Pmag_061")
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


def sample_bone_motion(animation, bone_name: str, play_length: float) -> tuple[bool, list[str]]:
    if not hasattr(unreal, "AnimationLibrary"):
        fail("animation_library_unavailable=1")

    animation_library = unreal.AnimationLibrary
    unreal_name = unreal.Name(bone_name)
    if not animation_library.does_bone_name_exist(animation, unreal_name):
        return False, []

    if play_length <= 0.0:
        return False, []

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

    required_present: dict[str, bool] = {name: False for name in REQUIRED_IMPORTED_MOTION_BONES}
    required_moved: dict[str, bool] = {name: False for name in REQUIRED_IMPORTED_MOTION_BONES}

    all_bones = REQUIRED_IMPORTED_MOTION_BONES + AUDIT_ONLY_BONES
    for animation in animations:
        play_length = base.animation_play_length(animation)
        track_names = {
            str(name)
            for name in unreal.AnimationLibrary.get_animation_track_names(animation)
        }
        unreal.log(
            "PASS45_REMINGTON870_UE58_IMPORTED_MOTION_SEQUENCE "
            f"path={animation.get_path_name()} play_length={play_length:.6f} "
            f"track_count={len(track_names)}"
        )

        for bone_name in all_bones:
            track_present = bone_name in track_names
            moved, pose_rows = sample_bone_motion(animation, bone_name, play_length)
            if bone_name in required_present:
                required_present[bone_name] = required_present[bone_name] or track_present
                required_moved[bone_name] = required_moved[bone_name] or moved

            unreal.log(
                "PASS45_REMINGTON870_UE58_IMPORTED_MOTION_BONE "
                f"sequence={animation.get_path_name()} bone={bone_name} "
                f"track_present={int(track_present)} moved={int(moved)} "
                f"samples={len(pose_rows)}"
            )
            for row in pose_rows:
                unreal.log(
                    "PASS45_REMINGTON870_UE58_IMPORTED_MOTION_SAMPLE "
                    f"sequence={animation.get_path_name()} bone={bone_name} {row}"
                )

    missing_tracks = [name for name, present in required_present.items() if not present]
    if missing_tracks:
        fail(
            "required_weapon_side_tracks_not_preserved=1 "
            f"bones={','.join(sorted(missing_tracks))}"
        )

    static_tracks = [name for name, moved in required_moved.items() if not moved]
    if static_tracks:
        fail(
            "required_weapon_side_motion_not_preserved=1 "
            f"bones={','.join(sorted(static_tracks))}"
        )

    unreal.log(
        "PASS45_REMINGTON870_UE58_IMPORTED_MOTION_PILOT_PASS "
        "pbody_track_preserved=1 pbody_motion_preserved=1 "
        "pmag_track_preserved=1 pmag_motion_preserved=1 "
        "pump_node_identity=UNPROVEN standalone_pump_clip=UNPROVEN "
        "visual_inspection_required=1 saved_packages=0 production_cutover=0 "
        "runtime_acceptance=0 item16_checked=0"
    )


if __name__ == "__main__":
    main()
