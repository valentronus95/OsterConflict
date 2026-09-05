#!/usr/bin/env python3
"""Single source of truth for PASS45 item-16 M700 / Lever calibration donor identity.

These constants identify the exact raw donor payloads used by current calibration
pilots, calibration review, approval validation and production-authoring binding.
Changing this file is calibration-critical and therefore invalidates any older
approved calibration through the ancestry guard.
"""
from __future__ import annotations

M700_SOURCE = "OsterConflict/Content/Raw/R13/Weapons/SteinClassicWeapons/WeaponsPack/M700/SKM_M700.fbx"
M700_SOURCE_SHA256 = "b7e003e01be8441e452730bc06c38c5e9752e523ae1b401ed2a6cc6cdca16840"
LEVER_SOURCE = "OsterConflict/Content/Raw/R13/Weapons/SteinClassicWeapons/WeaponsPack/LeverAction/SKM_LeverAction.fbx"
LEVER_SOURCE_SHA256 = "b2bf25bd47e9c4f6404897f67ad2a76a02971365fb7a689761936891d4591c69"

CALIBRATION_SOURCE_IDENTITIES = {
    "m700": {
        "source": M700_SOURCE,
        "source_sha256": M700_SOURCE_SHA256,
    },
    "lever_action": {
        "source": LEVER_SOURCE,
        "source_sha256": LEVER_SOURCE_SHA256,
    },
}
