#!/usr/bin/env python3
"""Review factual local UE 5.8 M700/Lever calibration pilot evidence.

This tool never authors production animation, chooses a final calibration value,
imports/saves Unreal assets, closes item 16, or claims runtime acceptance. It only
consolidates already-produced local UE evidence so continuation does not replay the
same source investigation.

The review is deliberately fail-closed against stale pre-recovery evidence. M700
must prove the current UE 5.8 add_bone_curve + 60 fps + compilation-barrier path.
Lever must additionally prove the integral 30->60 resampling envelope and the
0.85 s motion / 52-frame technical-envelope validation bridge. Both evidence files
must also name the exact pinned Stein source path and SHA-256 used by the current
pilots. A legacy or cross-source pilot JSON is not sufficient for current-head
calibration review.
"""
from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent
M700_PATH = ROOT / "OsterConflict" / "Saved" / "PASS45" / "PASS45_M700_DERIVED_BOLT_TRANSLATION_UE58_PILOT.json"
LEVER_PATH = ROOT / "OsterConflict" / "Saved" / "PASS45" / "PASS45_LEVERACTION_DERIVED_LEVER_UE58_PILOT.json"
OUT_DIR = ROOT / "PC_TEST" / "TEST_RESULTS"
OUT_JSON = OUT_DIR / "PASS45_ITEM16_M700_LEVER_CALIBRATION_REVIEW.json"
OUT_MD = OUT_DIR / "PASS45_ITEM16_M700_LEVER_CALIBRATION_REVIEW.md"

M700_EXPECTED_SOURCE = "OsterConflict/Content/Raw/R13/Weapons/SteinClassicWeapons/WeaponsPack/M700/SKM_M700.fbx"
M700_EXPECTED_SOURCE_SHA256 = "b7e003e01be8441e452730bc06c38c5e9752e523ae1b401ed2a6cc6cdca16840"
LEVER_EXPECTED_SOURCE = "OsterConflict/Content/Raw/R13/Weapons/SteinClassicWeapons/WeaponsPack/LeverAction/SKM_LeverAction.fbx"
LEVER_EXPECTED_SOURCE_SHA256 = "b2bf25bd47e9c4f6404897f67ad2a76a02971365fb7a689761936891d4591c69"
M700_COMPAT_FPS = 60
M700_COMPAT_FRAMES = 66
M700_COMPAT_KEYS = 67
LEVER_INITIAL_FPS = 30
LEVER_COMPAT_FPS = 60
LEVER_MOTION_DURATION = 0.85
LEVER_MOTION_END_FRAME = 51
LEVER_COMPAT_FRAMES = 52
LEVER_COMPAT_KEYS = 53
LEVER_SOURCE_FRAMES = 26
LEVER_TAIL_PAD_FRAMES = 1
LEVER_SEQUENCE_DURATION = LEVER_COMPAT_FRAMES / float(LEVER_COMPAT_FPS)


def fail(message: str) -> None:
    raise SystemExit(f"PASS45 ITEM16 CALIBRATION REVIEW: FAIL\n[FAIL] {message}")


def read_json(path: Path, label: str) -> dict:
    if not path.is_file():
        fail(f"{label} evidence missing: {path}")
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except Exception as exc:
        fail(f"{label} evidence invalid JSON: {exc}")
    if not isinstance(value, dict):
        fail(f"{label} evidence root is not an object")
    return value


def require_false(data: dict, key: str, label: str) -> None:
    if data.get(key) is not False:
        fail(f"{label} must preserve {key}=false; got {data.get(key)!r}")


def require_true(data: dict, key: str, label: str) -> None:
    if data.get(key) is not True:
        fail(f"{label} must preserve {key}=true; got {data.get(key)!r}")


def require_int(data: dict, key: str, expected: int, label: str) -> None:
    try:
        actual = int(data.get(key))
    except (TypeError, ValueError):
        fail(f"{label} missing/invalid {key}: {data.get(key)!r}")
    if actual != expected:
        fail(f"{label} {key} drifted: expected={expected} actual={actual}")


def require_float(data: dict, key: str, expected: float, tolerance: float, label: str) -> None:
    try:
        actual = float(data.get(key))
    except (TypeError, ValueError):
        fail(f"{label} missing/invalid {key}: {data.get(key)!r}")
    if abs(actual - expected) > tolerance:
        fail(f"{label} {key} drifted: expected={expected} actual={actual}")


def require_source_identity(
    data: dict,
    *,
    expected_path: str,
    expected_sha256: str,
    label: str,
) -> None:
    actual_path = str(data.get("source", ""))
    actual_sha = str(data.get("source_sha256", ""))
    if actual_path != expected_path:
        fail(f"{label} source path drifted: expected={expected_path!r} actual={actual_path!r}")
    if actual_sha != expected_sha256:
        fail(f"{label} source SHA-256 drifted: expected={expected_sha256} actual={actual_sha or 'MISSING'}")


def require_controller(data: dict, label: str) -> dict:
    controller = data.get("controller")
    if not isinstance(controller, dict):
        fail(f"{label} current UE58 controller evidence missing")
    if controller.get("track_creation_api") != "add_bone_curve":
        fail(
            f"{label} stale/unsupported bone-track creation evidence: "
            f"{controller.get('track_creation_api')!r}"
        )
    require_true(controller, "asset_compilation_barrier_before_sampling", label)
    return controller


def validate_common(data: dict, label: str) -> None:
    engine = str(data.get("engine_version", ""))
    if not engine.startswith("5.8"):
        fail(f"{label} is not UE 5.8 evidence: {engine!r}")
    for key in ("runtime_visual_acceptance", "runtime_acceptance", "item16_checked", "production_cutover"):
        require_false(data, key, label)
    require_false(data, "source_authored_endpoint", label)


def validate_m700(data: dict) -> dict:
    validate_common(data, "M700")
    require_source_identity(
        data,
        expected_path=M700_EXPECTED_SOURCE,
        expected_sha256=M700_EXPECTED_SOURCE_SHA256,
        label="M700",
    )
    if data.get("status") != "M700_BOLT_TRANSLATION_DERIVED_UE58_MOTION_PROOF_ONLY":
        fail(f"M700 status drifted: {data.get('status')!r}")
    require_false(data, "pilot_travel_accepted", "M700")
    require_false(data, "bolt_stop_used_as_endpoint", "M700")
    if data.get("rotation_calibration_pending") is not True:
        fail("M700 must preserve rotation_calibration_pending=true")
    if data.get("bolt_bone_addressable") is not True or data.get("bolt_motion_nontrivial") is not True:
        fail("M700 no longer proves addressable non-trivial BOLT motion")

    controller = require_controller(data, "M700")
    require_int(controller, "frame_rate", M700_COMPAT_FPS, "M700")
    require_int(controller, "frame_count", M700_COMPAT_FRAMES, "M700")
    require_int(controller, "key_count", M700_COMPAT_KEYS, "M700")
    require_float(data, "cycle_duration_seconds", 1.10, 1e-6, "M700")
    require_float(data, "play_length_seconds", 1.10, 0.011, "M700")

    pilot = float(data.get("pilot_max_travel", 0.0))
    sampled = float(data.get("max_sampled_translation_delta", 0.0))
    if pilot <= 0.0 or sampled <= 0.0:
        fail(f"M700 pilot motion is not measurable: pilot={pilot}, sampled={sampled}")
    return {
        "engine_version": data["engine_version"],
        "source": data["source"],
        "source_sha256": data["source_sha256"],
        "pilot_axis": data.get("pilot_axis"),
        "pilot_max_travel": pilot,
        "max_sampled_translation_delta": sampled,
        "end_return_error": float(data.get("end_return_error", 0.0)),
        "ue58_track_creation_api": controller["track_creation_api"],
        "ue58_frame_rate": int(controller["frame_rate"]),
        "ue58_frame_count": int(controller["frame_count"]),
        "ue58_asset_compilation_barrier": True,
        "travel_accepted": False,
        "rotation_pending": True,
        "manual_visual_calibration_required": True,
    }


def validate_lever(data: dict) -> dict:
    validate_common(data, "Lever Action")
    require_source_identity(
        data,
        expected_path=LEVER_EXPECTED_SOURCE,
        expected_sha256=LEVER_EXPECTED_SOURCE_SHA256,
        label="Lever Action",
    )
    if data.get("status") != "LEVERACTION_DERIVED_UE58_MOTION_PROOF_ONLY":
        fail(f"Lever Action status drifted: {data.get('status')!r}")
    require_false(data, "pilot_angle_accepted", "Lever Action")
    if data.get("lever_bone_addressable") is not True or data.get("lever_motion_nontrivial") is not True:
        fail("Lever Action no longer proves addressable non-trivial LEVER motion")

    controller = require_controller(data, "Lever Action")
    require_int(controller, "frame_rate", LEVER_COMPAT_FPS, "Lever Action")
    require_int(controller, "frame_count", LEVER_COMPAT_FRAMES, "Lever Action")
    require_int(controller, "key_count", LEVER_COMPAT_KEYS, "Lever Action")
    require_int(controller, "initial_transient_frame_rate", LEVER_INITIAL_FPS, "Lever Action")
    require_int(controller, "resampled_source_frames", LEVER_SOURCE_FRAMES, "Lever Action")
    require_int(controller, "motion_end_frame", LEVER_MOTION_END_FRAME, "Lever Action")
    require_int(controller, "tail_pad_frames", LEVER_TAIL_PAD_FRAMES, "Lever Action")
    require_float(controller, "motion_duration_seconds", LEVER_MOTION_DURATION, 1e-9, "Lever Action")
    require_float(controller, "sequence_duration_seconds", LEVER_SEQUENCE_DURATION, 1e-9, "Lever Action")
    require_true(controller, "sequence_envelope_validation_bridge", "Lever Action")
    require_float(data, "cycle_duration_seconds", LEVER_MOTION_DURATION, 1e-6, "Lever Action")
    require_float(data, "play_length_seconds", LEVER_SEQUENCE_DURATION, 1e-5, "Lever Action")

    if (LEVER_COMPAT_FRAMES * LEVER_INITIAL_FPS) % LEVER_COMPAT_FPS != 0:
        fail("internal Lever integral-grid review contract drifted")
    if LEVER_MOTION_END_FRAME / float(LEVER_COMPAT_FPS) != LEVER_MOTION_DURATION:
        fail("internal Lever motion-end review contract drifted")

    angle = float(data.get("pilot_max_angle_deg", 0.0))
    sampled = float(data.get("max_sampled_rotation_delta_deg", 0.0))
    if abs(angle) < 1.0 or sampled <= 0.0:
        fail(f"Lever Action pilot motion is not measurable: angle={angle}, sampled={sampled}")
    return {
        "engine_version": data["engine_version"],
        "source": data["source"],
        "source_sha256": data["source_sha256"],
        "pilot_axis": data.get("pilot_axis"),
        "pilot_max_angle_deg": angle,
        "max_sampled_rotation_delta_deg": sampled,
        "end_return_error_deg": float(data.get("end_return_error_deg", 0.0)),
        "ue58_track_creation_api": controller["track_creation_api"],
        "ue58_initial_frame_rate": int(controller["initial_transient_frame_rate"]),
        "ue58_compat_frame_rate": int(controller["frame_rate"]),
        "ue58_compat_frame_count": int(controller["frame_count"]),
        "ue58_resampled_source_frames": int(controller["resampled_source_frames"]),
        "ue58_motion_duration_seconds": float(controller["motion_duration_seconds"]),
        "ue58_sequence_duration_seconds": float(controller["sequence_duration_seconds"]),
        "ue58_motion_end_frame": int(controller["motion_end_frame"]),
        "ue58_tail_pad_frames": int(controller["tail_pad_frames"]),
        "ue58_sequence_envelope_validation_bridge": True,
        "ue58_asset_compilation_barrier": True,
        "angle_accepted": False,
        "manual_visual_calibration_required": True,
    }


def build_review(m700: dict, lever: dict) -> dict:
    if str(m700["engine_version"]).split("-")[0] != str(lever["engine_version"]).split("-")[0]:
        fail(f"UE evidence versions differ: M700={m700['engine_version']} Lever={lever['engine_version']}")
    return {
        "schema": 2,
        "status": "ITEM16_M700_LEVER_CURRENT_UE58_COMPAT_PILOTS_REVIEWED_CALIBRATION_PENDING",
        "evidence_contract": "CURRENT_UE58_COMPAT_FAIL_CLOSED",
        "source_identity_pinned": True,
        "m700": m700,
        "lever_action": lever,
        "next_factual_gate": "MANUAL_CURRENT_HEAD_UE58_VISUAL_CALIBRATION_BEFORE_PRODUCTION_AUTHORING",
        "full_gameplay_runtime_now": False,
        "runtime_acceptance": False,
        "item16_checked": False,
        "merge_permitted": False,
    }


def write_report(review: dict) -> None:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    OUT_JSON.write_text(json.dumps(review, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    m = review["m700"]
    l = review["lever_action"]
    text = f"""# PASS45 Item 16 M700 / Lever Calibration Review

- Status: `{review['status']}`
- Evidence contract: `{review['evidence_contract']}`
- Source identity pinned: `true`
- M700 source SHA-256: `{m['source_sha256']}`
- M700 pilot max travel: `{m['pilot_max_travel']}`
- M700 sampled translation delta: `{m['max_sampled_translation_delta']}`
- M700 UE58 authoring: `{m['ue58_track_creation_api']} @ {m['ue58_frame_rate']} fps / {m['ue58_frame_count']} frames`
- M700 rotation calibration pending: `true`
- Lever source SHA-256: `{l['source_sha256']}`
- Lever pilot max angle: `{l['pilot_max_angle_deg']} deg`
- Lever sampled rotation delta: `{l['max_sampled_rotation_delta_deg']} deg`
- Lever UE58 envelope: `{l['ue58_compat_frame_count']} @ {l['ue58_compat_frame_rate']} fps -> {l['ue58_resampled_source_frames']} frames @ {l['ue58_initial_frame_rate']} fps`
- Lever factual motion duration: `{l['ue58_motion_duration_seconds']} s`
- Lever technical sequence duration: `{l['ue58_sequence_duration_seconds']} s`
- Lever tail pad: `{l['ue58_tail_pad_frames']} frame`
- Final M700 travel/rotation accepted: `false`
- Final Lever angle accepted: `false`
- Full gameplay runtime now: `false`
- Next factual gate: `{review['next_factual_gate']}`
- `runtime_acceptance=0`
- `item16_checked=0`
- `merge_permitted=0`

This report consolidates current UE 5.8 compatibility-pilot evidence only. Legacy,
cross-source, or pre-recovery pilot JSON is rejected. Final motion values still
require direct current-head UE 5.8 visual calibration before production
authoring/cutover.
"""
    OUT_MD.write_text(text, encoding="utf-8")


def main() -> int:
    m700 = validate_m700(read_json(M700_PATH, "M700"))
    lever = validate_lever(read_json(LEVER_PATH, "Lever Action"))
    review = build_review(m700, lever)
    write_report(review)
    print("PASS45_ITEM16_M700_LEVER_CALIBRATION_REVIEW_PASS")
    print("evidence_contract=CURRENT_UE58_COMPAT_FAIL_CLOSED")
    print("source_identity_pinned=1")
    print(f"m700_pilot_max_travel={m700['pilot_max_travel']}")
    print(f"lever_pilot_max_angle_deg={lever['pilot_max_angle_deg']}")
    print("legacy_pilot_evidence_accepted=0")
    print("cross_source_pilot_evidence_accepted=0")
    print("manual_visual_calibration_required=1")
    print("full_gameplay_runtime_now=0")
    print("runtime_acceptance=0")
    print("item16_checked=0")
    print("merge_permitted=0")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
