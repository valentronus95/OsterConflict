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


def assert_rejected(callable_obj, payload: dict, expected_fragment: str, label: str) -> None:
    try:
        callable_obj(payload)
    except SystemExit as exc:
        if expected_fragment not in str(exc):
            fail(f"{label} rejected for the wrong reason: {exc}")
    else:
        fail(f"{label} stale evidence was incorrectly accepted")


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
        "user_local_execution_requested=0",
    ))
    require(REVIEW, (
        "PASS45_ITEM16_M700_LEVER_CALIBRATION_REVIEW_PASS",
        "M700_BOLT_TRANSLATION_DERIVED_UE58_MOTION_PROOF_ONLY",
        "LEVERACTION_DERIVED_UE58_MOTION_PROOF_ONLY",
        "source_authored_endpoint",
        "rotation_calibration_pending",
        "CURRENT_UE58_COMPAT_FAIL_CLOSED",
        "M700_EXPECTED_SOURCE_SHA256",
        "LEVER_EXPECTED_SOURCE_SHA256",
        "source_identity_pinned",
        "track_creation_api",
        "asset_compilation_barrier_before_sampling",
        "sequence_envelope_validation_bridge",
        "resampled_source_frames",
        "legacy_pilot_evidence_accepted=0",
        "cross_source_pilot_evidence_accepted=0",
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
    }
    m700_payload = {
        **common,
        "source": mod.M700_EXPECTED_SOURCE,
        "source_sha256": mod.M700_EXPECTED_SOURCE_SHA256,
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
        "cycle_duration_seconds": 1.10,
        "play_length_seconds": 1.10,
        "controller": {
            "track_creation_api": "add_bone_curve",
            "frame_rate": 60,
            "frame_count": 66,
            "key_count": 67,
            "asset_compilation_barrier_before_sampling": True,
        },
    }
    lever_payload = {
        **common,
        "source": mod.LEVER_EXPECTED_SOURCE,
        "source_sha256": mod.LEVER_EXPECTED_SOURCE_SHA256,
        "status": "LEVERACTION_DERIVED_UE58_MOTION_PROOF_ONLY",
        "pilot_angle_accepted": False,
        "lever_bone_addressable": True,
        "lever_motion_nontrivial": True,
        "pilot_axis": "LEVER local X",
        "pilot_max_angle_deg": -45.0,
        "max_sampled_rotation_delta_deg": 45.0,
        "end_return_error_deg": 0.0,
        "cycle_duration_seconds": 0.85,
        "play_length_seconds": 52.0 / 60.0,
        "controller": {
            "track_creation_api": "add_bone_curve",
            "frame_rate": 60,
            "frame_count": 52,
            "key_count": 53,
            "initial_transient_frame_rate": 30,
            "resampled_source_frames": 26,
            "motion_duration_seconds": 0.85,
            "motion_end_frame": 51,
            "tail_pad_frames": 1,
            "sequence_duration_seconds": 52.0 / 60.0,
            "asset_compilation_barrier_before_sampling": True,
            "sequence_envelope_validation_bridge": True,
        },
    }

    m700 = mod.validate_m700(m700_payload)
    lever = mod.validate_lever(lever_payload)
    review = mod.build_review(m700, lever)
    if review.get("evidence_contract") != "CURRENT_UE58_COMPAT_FAIL_CLOSED":
        fail("synthetic review lost the current UE58 compatibility evidence contract")
    if review.get("source_identity_pinned") is not True:
        fail("synthetic review lost exact source-identity pinning")
    if review.get("runtime_acceptance") is not False:
        fail("synthetic review falsely claims runtime acceptance")
    if review.get("item16_checked") is not False:
        fail("synthetic review falsely closes item16")
    if review.get("merge_permitted") is not False:
        fail("synthetic review falsely permits merge")
    if review.get("full_gameplay_runtime_now") is not False:
        fail("synthetic review incorrectly requires full gameplay runtime now")

    stale_m700 = dict(m700_payload)
    stale_m700["controller"] = {
        "track_creation_api": "add_bone_track",
        "frame_rate": 20,
        "frame_count": 22,
        "key_count": 23,
        "asset_compilation_barrier_before_sampling": False,
    }
    assert_rejected(
        mod.validate_m700,
        stale_m700,
        "stale/unsupported bone-track creation evidence",
        "M700 legacy add_bone_track evidence",
    )

    wrong_source_m700 = dict(m700_payload)
    wrong_source_m700["source_sha256"] = "0" * 64
    assert_rejected(
        mod.validate_m700,
        wrong_source_m700,
        "source SHA-256 drifted",
        "M700 cross-source evidence",
    )

    stale_lever = dict(lever_payload)
    stale_lever["controller"] = dict(lever_payload["controller"])
    stale_lever["controller"].pop("sequence_envelope_validation_bridge")
    assert_rejected(
        mod.validate_lever,
        stale_lever,
        "sequence_envelope_validation_bridge=true",
        "Lever evidence without padded-envelope validation bridge",
    )

    wrong_source_lever = dict(lever_payload)
    wrong_source_lever["source_sha256"] = "f" * 64
    assert_rejected(
        mod.validate_lever,
        wrong_source_lever,
        "source SHA-256 drifted",
        "Lever cross-source evidence",
    )

    fractional_lever = dict(lever_payload)
    fractional_lever["controller"] = dict(lever_payload["controller"])
    fractional_lever["controller"]["frame_count"] = 51
    fractional_lever["controller"]["key_count"] = 52
    fractional_lever["controller"]["resampled_source_frames"] = 25
    assert_rejected(
        mod.validate_lever,
        fractional_lever,
        "frame_count drifted",
        "Lever rejected 51-frame fractional-grid evidence",
    )

    print("PASS45_CHECKPOINT_CONTINUATION_AND_ITEM16_CALIBRATION_CONTRACT_PASS")
    print("current_ue58_compat_evidence_required=1 source_identity_pinned=1 legacy_pilot_evidence_accepted=0 cross_source_pilot_evidence_accepted=0")
    print("runtime_acceptance=0 item16_checked=0 merge_permitted=0 user_local_execution_requested=0")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
