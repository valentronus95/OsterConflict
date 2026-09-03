#!/usr/bin/env python3
"""Contract checks for PASS45 checkpoint continuation and item16 calibration review."""
from __future__ import annotations

import importlib.util
from pathlib import Path

ROOT = Path(__file__).resolve().parent
PROTOCOL = ROOT / "_DOCS" / "PASS45_CHECKPOINT_CONTINUATION_PROTOCOL.md"
HISTORY = ROOT / "PASS45_RUNTIME_RECOVERY_HISTORY.md"
REVIEW = ROOT / "PASS45_ITEM16_M700_LEVER_CALIBRATION_REVIEW.py"
LAUNCHER = ROOT / "OsterConflict" / "REVIEW_PASS45_ITEM16_M700_LEVER_CALIBRATION.cmd"


def fail(message: str) -> None:
    raise SystemExit(f"PASS45 CHECKPOINT/ITEM16 CONTRACT: FAIL\n[FAIL] {message}")


def require(path: Path, needles: tuple[str, ...]) -> str:
    if not path.is_file():
        fail(f"missing file: {path.relative_to(ROOT)}")
    text = path.read_text(encoding="utf-8")
    for needle in needles:
        if needle not in text:
            fail(f"{path.relative_to(ROOT)} missing required marker: {needle}")
    return text


def load_review_module():
    spec = importlib.util.spec_from_file_location("pass45_item16_review", REVIEW)
    if spec is None or spec.loader is None:
        fail("cannot load calibration review module")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def main() -> int:
    require(PROTOCOL, (
        "Do **not** restart a full-project audit by default.",
        "Inspect only the first factual open checklist item",
        "Parallel chats",
        "Local user Changes",
        "one consolidated current-head UE 5.8 weapon runtime acceptance",
    ))
    require(HISTORY, (
        "PASS45_CHECKPOINT_CONTINUATION_PROTOCOL.md",
        "item 16",
        "22/36 = 61.1%",
        "remote quarantine audit",
        "MANUAL CURRENT-HEAD UE 5.8 VISUAL CALIBRATION",
    ))
    require(REVIEW, (
        "PASS45_ITEM16_M700_LEVER_CALIBRATION_REVIEW_PASS",
        "M700_BOLT_TRANSLATION_DERIVED_UE58_MOTION_PROOF_ONLY",
        "LEVERACTION_DERIVED_UE58_MOTION_PROOF_ONLY",
        "source_authored_endpoint",
        "rotation_calibration_pending",
        "full_gameplay_runtime_now",
        "merge_permitted",
    ))
    require(LAUNCHER, (
        "PASS45_ITEM16_M700_LEVER_CALIBRATION_REVIEW_COMPLETE",
        "does not run full gameplay runtime",
        "runtime_acceptance=0",
        "item16_checked=0",
        "merge_permitted=0",
    ))

    mod = load_review_module()
    common = {
        "engine_version": "5.8.0-test",
        "runtime_visual_acceptance": False,
        "runtime_acceptance": False,
        "item16_checked": False,
        "production_cutover": False,
        "source_authored_endpoint": False,
        "source_sha256": "test",
    }
    m700 = mod.validate_m700({
        **common,
        "status": "M700_BOLT_TRANSLATION_DERIVED_UE58_MOTION_PROOF_ONLY",
        "pilot_travel_accepted": False,
        "bolt_stop_used_as_endpoint": False,
        "rotation_calibration_pending": True,
        "bolt_bone_addressable": True,
        "bolt_motion_nontrivial": True,
        "pilot_axis": "BOLT local Y",
        "pilot_max_travel": 0.039,
        "max_sampled_translation_delta": 0.039,
        "end_return_error": 0.0,
    })
    lever = mod.validate_lever({
        **common,
        "status": "LEVERACTION_DERIVED_UE58_MOTION_PROOF_ONLY",
        "pilot_angle_accepted": False,
        "lever_bone_addressable": True,
        "lever_motion_nontrivial": True,
        "pilot_axis": "LEVER local X",
        "pilot_max_angle_deg": -45.0,
        "max_sampled_rotation_delta_deg": 45.0,
        "end_return_error_deg": 0.0,
    })
    review = mod.build_review(m700, lever)
    if review.get("runtime_acceptance") is not False:
        fail("synthetic review falsely claims runtime acceptance")
    if review.get("item16_checked") is not False:
        fail("synthetic review falsely closes item16")
    if review.get("merge_permitted") is not False:
        fail("synthetic review falsely permits merge")
    if review.get("full_gameplay_runtime_now") is not False:
        fail("synthetic review incorrectly requires full gameplay runtime now")

    print("PASS45_CHECKPOINT_CONTINUATION_AND_ITEM16_CALIBRATION_CONTRACT_PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
