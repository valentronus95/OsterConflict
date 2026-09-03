#!/usr/bin/env python3
"""UE 5.8 compatibility shim for the bounded Lever Action motion pilot.

UE 5.8 creates a transient AnimSequence at 30 fps. AnimationDataController rejects
changing that sequence to 20 fps because 20 is neither a multiple nor a factor of
30. UE 5.8 also deprecates add_bone_track() in favor of add_bone_curve().

The underlying pilot remains authoritative for source identity, motion shape,
acceptance flags, evidence, and all safety gates. This shim changes only transient
pilot authoring compatibility: 60 fps cadence and the UE 5.8 bone-curve creation
API. It preserves the exact 0.85 s cycle as 51 frames.

This is proof-only. It does not save packages, change production profiles, accept
the pilot angle, close item 16, or permit merge.
"""
from __future__ import annotations

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

import PASS45_LEVERACTION_DERIVED_LEVER_UE58_PILOT as pilot

EXPECTED_LEGACY_FRAME_RATE = 20
EXPECTED_LEGACY_FRAME_COUNT = 17
EXPECTED_LEGACY_KEY_COUNT = 18
COMPAT_FRAME_RATE = 60
COMPAT_FRAME_COUNT = 51
COMPAT_KEY_COUNT = 52
EXPECTED_CYCLE_DURATION = 0.85


def configure_sequence_ue58(sequence: object, ref_transform: object) -> dict[str, object]:
    unreal = pilot.unreal
    controller = pilot.animation_controller(sequence)
    controller.set_frame_rate(
        unreal.FrameRate(numerator=pilot.FRAME_RATE, denominator=1), False
    )
    controller.set_number_of_frames(
        unreal.FrameNumber(value=pilot.FRAME_COUNT), False
    )

    add_bone_curve = getattr(controller, "add_bone_curve", None)
    if not callable(add_bone_curve):
        pilot.fail("ue58_add_bone_curve_unavailable=1")
    if not bool(add_bone_curve(unreal.Name(pilot.LEVER_BONE), False)):
        pilot.fail("lever_bone_curve_creation_failed=1")

    bind_translation = pilot.copy_vector(ref_transform.translation)
    bind_rotation = pilot.normalized_quat(pilot.copy_quat(ref_transform.rotation))
    bind_scale = pilot.copy_vector(ref_transform.scale3d)

    positions = []
    rotations = []
    scales = []
    key_rows: list[dict[str, object]] = []
    for frame in range(pilot.KEY_COUNT):
        seconds = frame / float(pilot.FRAME_RATE)
        angle = pilot.angle_for_time(seconds)
        delta = pilot.quat_x(angle)
        local_rotation = pilot.normalized_quat(
            pilot.quat_mul(bind_rotation, delta)
        )
        positions.append(pilot.copy_vector(bind_translation))
        rotations.append(local_rotation)
        scales.append(pilot.copy_vector(bind_scale))
        key_rows.append(
            {
                "frame": frame,
                "time": round(seconds, 6),
                "pilot_angle_deg": round(angle, 6),
            }
        )

    if not controller.set_bone_track_keys(
        unreal.Name(pilot.LEVER_BONE),
        positions,
        rotations,
        scales,
        False,
    ):
        pilot.fail("lever_bone_track_key_write_failed=1")

    return {
        "track_index": None,
        "track_creation_api": "add_bone_curve",
        "frame_rate": pilot.FRAME_RATE,
        "frame_count": pilot.FRAME_COUNT,
        "key_count": pilot.KEY_COUNT,
        "bind_translation": [
            float(bind_translation.x),
            float(bind_translation.y),
            float(bind_translation.z),
        ],
        "bind_rotation": [
            float(bind_rotation.x),
            float(bind_rotation.y),
            float(bind_rotation.z),
            float(bind_rotation.w),
        ],
        "bind_scale": [
            float(bind_scale.x),
            float(bind_scale.y),
            float(bind_scale.z),
        ],
        "keys": key_rows,
    }


def main() -> None:
    if pilot.FRAME_RATE != EXPECTED_LEGACY_FRAME_RATE:
        raise RuntimeError(
            f"Lever pilot frame-rate contract drifted: expected {EXPECTED_LEGACY_FRAME_RATE}, got {pilot.FRAME_RATE}"
        )
    if pilot.FRAME_COUNT != EXPECTED_LEGACY_FRAME_COUNT:
        raise RuntimeError(
            f"Lever pilot frame-count contract drifted: expected {EXPECTED_LEGACY_FRAME_COUNT}, got {pilot.FRAME_COUNT}"
        )
    if pilot.KEY_COUNT != EXPECTED_LEGACY_KEY_COUNT:
        raise RuntimeError(
            f"Lever pilot key-count contract drifted: expected {EXPECTED_LEGACY_KEY_COUNT}, got {pilot.KEY_COUNT}"
        )
    if abs((COMPAT_FRAME_COUNT / COMPAT_FRAME_RATE) - EXPECTED_CYCLE_DURATION) > 1e-9:
        raise RuntimeError(
            "Lever compatibility cadence no longer preserves the 0.85 s pilot duration"
        )
    if COMPAT_KEY_COUNT != COMPAT_FRAME_COUNT + 1:
        raise RuntimeError(
            "Lever compatibility key count no longer matches frame count + 1"
        )

    pilot.FRAME_RATE = COMPAT_FRAME_RATE
    pilot.FRAME_COUNT = COMPAT_FRAME_COUNT
    pilot.KEY_COUNT = COMPAT_KEY_COUNT
    pilot.configure_sequence = configure_sequence_ue58
    pilot.main()


if __name__ == "__main__":
    main()
