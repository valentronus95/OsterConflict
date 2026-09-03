#!/usr/bin/env python3
"""Static contract for PASS45 item-16 UE 5.8 transient animation compatibility."""
from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parent
errors: list[str] = []


def read(rel: str) -> str:
    path = ROOT / rel
    if not path.is_file():
        raise SystemExit(f"PASS45 ITEM16 UE58 COMPAT: FAIL\n[FAIL] missing file: {rel}")
    return path.read_text(encoding="utf-8", errors="replace")


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


m700 = read("PASS45_M700_DERIVED_BOLT_TRANSLATION_UE58_PILOT_COMPAT.py")
lever = read("PASS45_LEVERACTION_DERIVED_LEVER_UE58_PILOT_COMPAT.py")
m700_launcher = read("OsterConflict/TRY_PASS45_M700_DERIVED_BOLT_TRANSLATION_UE58_PILOT.cmd")
lever_launcher = read("OsterConflict/TRY_PASS45_LEVERACTION_DERIVED_LEVER_UE58_PILOT.cmd")
chain = read("OsterConflict/RUN_PASS45_ITEM16_LOCAL_UE58_EVIDENCE.cmd")

# UE 5.8 cadence contract. The transient sequence starts at 30 fps in the local
# engine, while the bounded proof shapes were authored around exact 20 fps
# durations. The compatibility shims preserve the exact duration at 60 fps.
for needle in (
    "EXPECTED_LEGACY_FRAME_RATE = 20",
    "EXPECTED_LEGACY_FRAME_COUNT = 22",
    "COMPAT_FRAME_RATE = 60",
    "COMPAT_FRAME_COUNT = 66",
    "EXPECTED_CYCLE_DURATION = 1.10",
    "pilot.FRAME_RATE = COMPAT_FRAME_RATE",
    "pilot.FRAME_COUNT = COMPAT_FRAME_COUNT",
    "pilot.main()",
):
    req(needle in m700, f"M700 compatibility contract missing: {needle}")

for needle in (
    "EXPECTED_LEGACY_FRAME_RATE = 20",
    "EXPECTED_LEGACY_FRAME_COUNT = 17",
    "EXPECTED_LEGACY_KEY_COUNT = 18",
    "COMPAT_FRAME_RATE = 60",
    "COMPAT_FRAME_COUNT = 51",
    "COMPAT_KEY_COUNT = 52",
    "EXPECTED_CYCLE_DURATION = 0.85",
    "pilot.FRAME_RATE = COMPAT_FRAME_RATE",
    "pilot.FRAME_COUNT = COMPAT_FRAME_COUNT",
    "pilot.KEY_COUNT = COMPAT_KEY_COUNT",
    "pilot.main()",
):
    req(needle in lever, f"Lever compatibility contract missing: {needle}")

# UE 5.8 animation-data API regression guard.
# The 2026-09-03 factual local run on cf75b86c failed because the legacy
# AddBoneTrack path returned INDEX_NONE for the imported M700 BOLT track. UE 5.8
# deprecates AddBoneTrack in favor of AddBoneCurve. Both proof shims must override
# the legacy base authoring functions and then write keys to the created curve.
for needle in (
    "def create_sequence_ue58(",
    'getattr(data, "add_bone_curve", None)',
    "add_bone_curve(unreal.Name(pilot.BOLT_BONE), False)",
    "data.set_bone_track_keys(",
    '"track_creation_api": "add_bone_curve"',
    "pilot.create_sequence = create_sequence_ue58",
):
    req(needle in m700, f"M700 UE58 bone-curve recovery missing: {needle}")

for needle in (
    "def configure_sequence_ue58(",
    'getattr(controller, "add_bone_curve", None)',
    "add_bone_curve(unreal.Name(pilot.LEVER_BONE), False)",
    "controller.set_bone_track_keys(",
    '"track_creation_api": "add_bone_curve"',
    "pilot.configure_sequence = configure_sequence_ue58",
):
    req(needle in lever, f"Lever UE58 bone-curve recovery missing: {needle}")

req(".add_bone_track(" not in m700, "M700 compatibility shim directly calls deprecated add_bone_track()")
req(".add_bone_track(" not in lever, "Lever compatibility shim directly calls deprecated add_bone_track()")

req("PASS45_M700_DERIVED_BOLT_TRANSLATION_UE58_PILOT.py" in m700_launcher, "M700 canonical pilot identity disappeared from launcher")
req("PASS45_M700_DERIVED_BOLT_TRANSLATION_UE58_PILOT_COMPAT.py" in m700_launcher, "M700 launcher does not use compatibility shim")
req("PASS45_LEVERACTION_DERIVED_LEVER_UE58_PILOT.py" in lever_launcher, "Lever canonical pilot identity disappeared from launcher")
req("PASS45_LEVERACTION_DERIVED_LEVER_UE58_PILOT_COMPAT.py" in lever_launcher, "Lever launcher does not use compatibility shim")
req("compatible 60 fps only" in m700_launcher, "M700 launcher does not disclose compatibility cadence")
req("compatible 60 fps only" in lever_launcher, "Lever launcher does not disclose compatibility cadence")

for launcher, label in ((m700_launcher, "M700"), (lever_launcher, "Lever")):
    for forbidden in (
        "git checkout",
        "git reset",
        "git clean",
        "git pull",
        "git push",
        "save_asset",
        "save_directory",
        "runtime_acceptance=1",
        "item16_checked=1",
    ):
        req(forbidden.lower() not in launcher.lower(), f"{label} launcher regained forbidden token: {forbidden}")

for shim, label in ((m700, "M700"), (lever, "Lever")):
    for forbidden in (
        "/Game/Production/",
        "save_asset(",
        "save_directory(",
        "delete_asset(",
        "delete_directory(",
        "subprocess.",
        "os.system",
        "runtime_acceptance = True",
        "item16_checked = True",
    ):
        req(forbidden.lower() not in shim.lower(), f"{label} compatibility shim contains forbidden mutation/acceptance token: {forbidden}")

req("TRY_PASS45_M700_DERIVED_BOLT_TRANSLATION_UE58_PILOT.cmd" in chain, "item16 chain no longer calls M700 bounded pilot")
req("TRY_PASS45_LEVERACTION_DERIVED_LEVER_UE58_PILOT.cmd" in chain, "item16 chain no longer calls Lever bounded pilot")
req("runtime_acceptance=0" in chain and "item16_checked=0" in chain and "merge_permitted=0" in chain, "item16 chain fail-closed markers drifted")

if errors:
    print("PASS45 ITEM16 UE58 COMPAT: FAIL")
    for error in errors:
        print(f"[FAIL] {error}")
    raise SystemExit(1)

print("PASS45 ITEM16 UE58 COMPAT: PASS")
print("m700_legacy_fps=20 m700_compat_fps=60 m700_frames=66 duration=1.10 bone_curve_api=1")
print("lever_legacy_fps=20 lever_compat_fps=60 lever_frames=51 duration=0.85 bone_curve_api=1")
print("deprecated_add_bone_track_direct_calls=0")
print("production_cutover=0 runtime_acceptance=0 item16_checked=0 merge_permitted=0")
