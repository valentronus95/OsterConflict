#!/usr/bin/env python3
"""UE 5.8 compatibility shim for the bounded M700 motion pilot.

UE 5.8 creates a transient AnimSequence at 30 fps. AnimationDataController rejects
changing that sequence to 20 fps because 20 is neither a multiple nor a factor of
30. UE 5.8 also deprecates add_bone_track() in favor of add_bone_curve(); on the
current local engine the legacy call returns INDEX_NONE for the imported BOLT bone.

The underlying pilot remains authoritative for source identity, motion shape,
acceptance flags, evidence, and all safety gates. This shim changes only transient
pilot authoring compatibility: 60 fps cadence and the UE 5.8 bone-curve creation
API. It preserves the exact 1.10 s cycle as 66 frames.

This is proof-only. It does not save packages, change production profiles, accept
pilot travel, author bolt rotation, close item 16, or permit merge.
"""
from __future__ import annotations

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

import PASS45_M700_DERIVED_BOLT_TRANSLATION_UE58_PILOT as pilot

EXPECTED_LEGACY_FRAME_RATE = 20
EXPECTED_LEGACY_FRAME_COUNT = 22
COMPAT_FRAME_RATE = 60
COMPAT_FRAME_COUNT = 66
EXPECTED_CYCLE_DURATION = 1.10


def create_sequence_ue58(
    skeleton: object,
    mesh: object,
    ref_transform: object,
    contract: object,
) -> tuple[object, dict[str, object]]:
    unreal = pilot.unreal
    factory = unreal.AnimSequenceFactory()
    factory.set_editor_property("target_skeleton", skeleton)
    try:
        factory.set_editor_property("preview_skeletal_mesh", mesh)
    except Exception:
        pass
    sequence = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        pilot.PILOT_SEQUENCE_NAME,
        pilot.PILOT_DESTINATION,
        unreal.AnimSequence,
        factory,
    )
    if sequence is None or pilot.class_name(sequence) != "AnimSequence":
        pilot.fail("pilot_anim_sequence_creation_failed=1")

    data = pilot.controller(sequence)
    data.set_frame_rate(
        unreal.FrameRate(numerator=pilot.FRAME_RATE, denominator=1), False
    )
    data.set_number_of_frames(
        unreal.FrameNumber(value=pilot.FRAME_COUNT), False
    )

    add_bone_curve = getattr(data, "add_bone_curve", None)
    if not callable(add_bone_curve):
        pilot.fail("ue58_add_bone_curve_unavailable=1")
    if not bool(add_bone_curve(unreal.Name(pilot.BOLT_BONE), False)):
        pilot.fail("bolt_bone_curve_creation_failed=1")

    bind_t = pilot.copy_vector(ref_transform.translation)
    bind_r = pilot.copy_quat(ref_transform.rotation)
    bind_s = pilot.copy_vector(ref_transform.scale3d)
    positions = []
    rotations = []
    scales = []
    keys: list[dict[str, object]] = []
    for frame in range(pilot.FRAME_COUNT + 1):
        seconds = frame / float(pilot.FRAME_RATE)
        offset_y = pilot.interpolate_offset(contract, seconds)
        positions.append(
            unreal.Vector(
                float(bind_t.x),
                float(bind_t.y) + offset_y,
                float(bind_t.z),
            )
        )
        rotations.append(pilot.copy_quat(bind_r))
        scales.append(pilot.copy_vector(bind_s))
        keys.append(
            {
                "frame": frame,
                "time": round(seconds, 6),
                "offset_y": offset_y,
            }
        )
    if not data.set_bone_track_keys(
        unreal.Name(pilot.BOLT_BONE),
        positions,
        rotations,
        scales,
        False,
    ):
        pilot.fail("bolt_bone_track_key_write_failed=1")

    return sequence, {
        "track_index": None,
        "track_creation_api": "add_bone_curve",
        "frame_rate": pilot.FRAME_RATE,
        "frame_count": pilot.FRAME_COUNT,
        "key_count": pilot.FRAME_COUNT + 1,
        "bind_translation": [
            float(bind_t.x),
            float(bind_t.y),
            float(bind_t.z),
        ],
        "keys": keys,
    }


def main() -> None:
    if pilot.FRAME_RATE != EXPECTED_LEGACY_FRAME_RATE:
        raise RuntimeError(
            f"M700 pilot frame-rate contract drifted: expected {EXPECTED_LEGACY_FRAME_RATE}, got {pilot.FRAME_RATE}"
        )
    if pilot.FRAME_COUNT != EXPECTED_LEGACY_FRAME_COUNT:
        raise RuntimeError(
            f"M700 pilot frame-count contract drifted: expected {EXPECTED_LEGACY_FRAME_COUNT}, got {pilot.FRAME_COUNT}"
        )
    if abs((COMPAT_FRAME_COUNT / COMPAT_FRAME_RATE) - EXPECTED_CYCLE_DURATION) > 1e-9:
        raise RuntimeError(
            "M700 compatibility cadence no longer preserves the 1.10 s pilot duration"
        )

    pilot.FRAME_RATE = COMPAT_FRAME_RATE
    pilot.FRAME_COUNT = COMPAT_FRAME_COUNT
    pilot.create_sequence = create_sequence_ue58
    pilot.main()


if __name__ == "__main__":
    main()
