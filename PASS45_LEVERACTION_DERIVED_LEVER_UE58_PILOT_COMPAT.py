#!/usr/bin/env python3
"""UE 5.8 frame-rate compatibility shim for the bounded Lever Action motion pilot.

UE 5.8 creates a transient AnimSequence at 30 fps. AnimationDataController rejects
changing that sequence to 20 fps because 20 is neither a multiple nor a factor of
30. The underlying pilot remains authoritative for source identity, motion shape,
acceptance flags, evidence, and all safety gates. This shim changes only the
transient controller sampling rate to a compatible 60 fps, preserving the exact
0.85 s cycle as 51 frames.

This is proof-only. It does not save packages, change production profiles, accept
the pilot angle, close item 16, or permit merge.
"""
from __future__ import annotations

import PASS45_LEVERACTION_DERIVED_LEVER_UE58_PILOT as pilot

EXPECTED_LEGACY_FRAME_RATE = 20
EXPECTED_LEGACY_FRAME_COUNT = 17
EXPECTED_LEGACY_KEY_COUNT = 18
COMPAT_FRAME_RATE = 60
COMPAT_FRAME_COUNT = 51
COMPAT_KEY_COUNT = 52
EXPECTED_CYCLE_DURATION = 0.85


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
        raise RuntimeError("Lever compatibility cadence no longer preserves the 0.85 s pilot duration")
    if COMPAT_KEY_COUNT != COMPAT_FRAME_COUNT + 1:
        raise RuntimeError("Lever compatibility key count no longer matches frame count + 1")

    pilot.FRAME_RATE = COMPAT_FRAME_RATE
    pilot.FRAME_COUNT = COMPAT_FRAME_COUNT
    pilot.KEY_COUNT = COMPAT_KEY_COUNT
    pilot.main()


if __name__ == "__main__":
    main()
