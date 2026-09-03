#!/usr/bin/env python3
"""UE 5.8 frame-rate compatibility shim for the bounded M700 motion pilot.

UE 5.8 creates a transient AnimSequence at 30 fps. AnimationDataController rejects
changing that sequence to 20 fps because 20 is neither a multiple nor a factor of
30. The underlying pilot remains authoritative for source identity, motion shape,
acceptance flags, evidence, and all safety gates. This shim changes only the
transient controller sampling rate to a compatible 60 fps, preserving the exact
1.10 s cycle as 66 frames.

This is proof-only. It does not save packages, change production profiles, accept
pilot travel, author bolt rotation, close item 16, or permit merge.
"""
from __future__ import annotations

import PASS45_M700_DERIVED_BOLT_TRANSLATION_UE58_PILOT as pilot

EXPECTED_LEGACY_FRAME_RATE = 20
EXPECTED_LEGACY_FRAME_COUNT = 22
COMPAT_FRAME_RATE = 60
COMPAT_FRAME_COUNT = 66
EXPECTED_CYCLE_DURATION = 1.10


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
        raise RuntimeError("M700 compatibility cadence no longer preserves the 1.10 s pilot duration")

    pilot.FRAME_RATE = COMPAT_FRAME_RATE
    pilot.FRAME_COUNT = COMPAT_FRAME_COUNT
    pilot.main()


if __name__ == "__main__":
    main()
