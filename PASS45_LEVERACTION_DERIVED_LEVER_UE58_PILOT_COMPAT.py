#!/usr/bin/env python3
"""UE 5.8 compatibility shim for the bounded Lever Action motion pilot.

UE 5.8 creates the transient AnimSequence on a 30 fps sampling grid. The legacy
pilot uses a 20 fps / 0.85 s cycle, so the compatibility shim authors at 60 fps.
A 0.85 s sequence envelope is not itself legal for UE's 30 fps resampling path:
0.85 * 30 = 25.5 source frames. The 2026-09-03 factual local run proved that
SetNumberOfFrames(51 @ 60 fps) reaches AnimSequence resampling with a 0.5
sub-frame remainder and then asserts in animation compression.

The recovery keeps the actual Lever motion exactly 0.85 s: frame 51 at 60 fps is
still the exact motion endpoint. The transient sequence envelope is padded by one
additional 60 fps frame to 52 frames / 0.866666... s, which maps to exactly 26
frames on the initial 30 fps grid. The final padded key remains at the returned
bind pose because the authoritative motion function clamps at the 0.85 s cycle
endpoint.

UE 5.8 also deprecates add_bone_track() in favor of add_bone_curve(). The shim
keeps the previously added asset-compilation barriers as a separate teardown
safety guard, but the frame-grid correction happens before those barriers because
the rejected run crashed during SetNumberOfFrames/compression.

The underlying pilot remains authoritative for source identity, motion shape,
acceptance flags, evidence, and all safety gates. This is proof-only. It does not
save packages, change production profiles, accept the pilot angle, close item 16,
or permit merge.
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
INITIAL_TRANSIENT_FRAME_RATE = 30
COMPAT_FRAME_RATE = 60
EXPECTED_CYCLE_DURATION = 0.85
EXPECTED_CYCLE_END_FRAME = 51
COMPAT_PAD_FRAME_COUNT = 1
COMPAT_FRAME_COUNT = EXPECTED_CYCLE_END_FRAME + COMPAT_PAD_FRAME_COUNT
COMPAT_KEY_COUNT = COMPAT_FRAME_COUNT + 1


def finish_asset_compilation_ue58(stage: str) -> None:
    unreal = pilot.unreal
    library = getattr(unreal, "AutomationUtilsBlueprintLibrary", None)
    if library is None:
        pilot.fail(
            "ue58_automation_utils_unavailable=1 "
            f"stage={stage} finish_all_asset_compilation_required=1"
        )
    finish_all = getattr(library, "finish_all_asset_compilation", None)
    if not callable(finish_all):
        pilot.fail(
            "ue58_finish_all_asset_compilation_unavailable=1 "
            f"stage={stage}"
        )
    unreal.log(
        "PASS45_LEVERACTION_UE58_ASSET_COMPILATION_BARRIER_BEGIN "
        f"stage={stage}"
    )
    finish_all()
    unreal.log(
        "PASS45_LEVERACTION_UE58_ASSET_COMPILATION_BARRIER_END "
        f"stage={stage}"
    )


def configure_sequence_ue58(sequence: object, ref_transform: object) -> dict[str, object]:
    unreal = pilot.unreal
    controller = pilot.animation_controller(sequence)
    controller.set_frame_rate(
        unreal.FrameRate(numerator=pilot.FRAME_RATE, denominator=1), False
    )

    resampled_source_frames = (
        pilot.FRAME_COUNT * INITIAL_TRANSIENT_FRAME_RATE
    ) // pilot.FRAME_RATE
    unreal.log(
        "PASS45_LEVERACTION_UE58_RESAMPLE_GRID_READY "
        f"initial_fps={INITIAL_TRANSIENT_FRAME_RATE} "
        f"compat_fps={pilot.FRAME_RATE} "
        f"compat_frames={pilot.FRAME_COUNT} "
        f"source_frames={resampled_source_frames} "
        f"motion_end_frame={EXPECTED_CYCLE_END_FRAME} "
        f"tail_pad_frames={COMPAT_PAD_FRAME_COUNT}"
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

    # SetBoneTrackKeys broadcasts TrackChanged and may start compression/DDC work.
    # Drain that work before the base pilot samples the sequence.
    finish_asset_compilation_ue58("after_set_bone_track_keys_before_sampling")

    return {
        "track_index": None,
        "track_creation_api": "add_bone_curve",
        "frame_rate": pilot.FRAME_RATE,
        "frame_count": pilot.FRAME_COUNT,
        "key_count": pilot.KEY_COUNT,
        "initial_transient_frame_rate": INITIAL_TRANSIENT_FRAME_RATE,
        "resampled_source_frames": resampled_source_frames,
        "motion_duration_seconds": EXPECTED_CYCLE_DURATION,
        "motion_end_frame": EXPECTED_CYCLE_END_FRAME,
        "tail_pad_frames": COMPAT_PAD_FRAME_COUNT,
        "sequence_duration_seconds": pilot.FRAME_COUNT / float(pilot.FRAME_RATE),
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
        "asset_compilation_barrier_before_sampling": True,
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
    if abs(
        (EXPECTED_CYCLE_END_FRAME / COMPAT_FRAME_RATE) - EXPECTED_CYCLE_DURATION
    ) > 1e-9:
        raise RuntimeError(
            "Lever compatibility motion endpoint no longer preserves the exact 0.85 s cycle"
        )
    if COMPAT_FRAME_COUNT != EXPECTED_CYCLE_END_FRAME + COMPAT_PAD_FRAME_COUNT:
        raise RuntimeError("Lever compatibility tail-pad contract drifted")
    if COMPAT_KEY_COUNT != COMPAT_FRAME_COUNT + 1:
        raise RuntimeError(
            "Lever compatibility key count no longer matches frame count + 1"
        )
    if (
        COMPAT_FRAME_COUNT * INITIAL_TRANSIENT_FRAME_RATE
    ) % COMPAT_FRAME_RATE != 0:
        raise RuntimeError(
            "Lever compatibility sequence envelope no longer maps to an integral 30 fps resampling frame"
        )

    pilot.FRAME_RATE = COMPAT_FRAME_RATE
    pilot.FRAME_COUNT = COMPAT_FRAME_COUNT
    pilot.KEY_COUNT = COMPAT_KEY_COUNT
    pilot.configure_sequence = configure_sequence_ue58
    pilot.main()

    # Do not let the PythonScriptCommandlet tear down transient imported/animation
    # objects while UE foreground workers still own compilation/DDC follow-up work.
    finish_asset_compilation_ue58("post_pilot_before_commandlet_exit")


if __name__ == "__main__":
    main()
