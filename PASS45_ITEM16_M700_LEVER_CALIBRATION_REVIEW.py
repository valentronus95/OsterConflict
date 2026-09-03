#!/usr/bin/env python3
"""Review factual local UE 5.8 M700/Lever calibration pilot evidence.

This tool never authors production animation, chooses a final calibration value,
imports/saves Unreal assets, closes item 16, or claims runtime acceptance. It only
consolidates already-produced local UE evidence so continuation does not replay the
same source investigation.
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


def validate_common(data: dict, label: str) -> None:
    engine = str(data.get("engine_version", ""))
    if not engine.startswith("5.8"):
        fail(f"{label} is not UE 5.8 evidence: {engine!r}")
    for key in ("runtime_visual_acceptance", "runtime_acceptance", "item16_checked", "production_cutover"):
        require_false(data, key, label)
    require_false(data, "source_authored_endpoint", label)


def validate_m700(data: dict) -> dict:
    validate_common(data, "M700")
    if data.get("status") != "M700_BOLT_TRANSLATION_DERIVED_UE58_MOTION_PROOF_ONLY":
        fail(f"M700 status drifted: {data.get('status')!r}")
    require_false(data, "pilot_travel_accepted", "M700")
    require_false(data, "bolt_stop_used_as_endpoint", "M700")
    if data.get("rotation_calibration_pending") is not True:
        fail("M700 must preserve rotation_calibration_pending=true")
    if data.get("bolt_bone_addressable") is not True or data.get("bolt_motion_nontrivial") is not True:
        fail("M700 no longer proves addressable non-trivial BOLT motion")
    pilot = float(data.get("pilot_max_travel", 0.0))
    sampled = float(data.get("max_sampled_translation_delta", 0.0))
    if pilot <= 0.0 or sampled <= 0.0:
        fail(f"M700 pilot motion is not measurable: pilot={pilot}, sampled={sampled}")
    return {
        "engine_version": data["engine_version"],
        "source_sha256": data.get("source_sha256"),
        "pilot_axis": data.get("pilot_axis"),
        "pilot_max_travel": pilot,
        "max_sampled_translation_delta": sampled,
        "end_return_error": float(data.get("end_return_error", 0.0)),
        "travel_accepted": False,
        "rotation_pending": True,
        "manual_visual_calibration_required": True,
    }


def validate_lever(data: dict) -> dict:
    validate_common(data, "Lever Action")
    if data.get("status") != "LEVERACTION_DERIVED_UE58_MOTION_PROOF_ONLY":
        fail(f"Lever Action status drifted: {data.get('status')!r}")
    require_false(data, "pilot_angle_accepted", "Lever Action")
    if data.get("lever_bone_addressable") is not True or data.get("lever_motion_nontrivial") is not True:
        fail("Lever Action no longer proves addressable non-trivial LEVER motion")
    angle = float(data.get("pilot_max_angle_deg", 0.0))
    sampled = float(data.get("max_sampled_rotation_delta_deg", 0.0))
    if abs(angle) < 1.0 or sampled <= 0.0:
        fail(f"Lever Action pilot motion is not measurable: angle={angle}, sampled={sampled}")
    return {
        "engine_version": data["engine_version"],
        "source_sha256": data.get("source_sha256"),
        "pilot_axis": data.get("pilot_axis"),
        "pilot_max_angle_deg": angle,
        "max_sampled_rotation_delta_deg": sampled,
        "end_return_error_deg": float(data.get("end_return_error_deg", 0.0)),
        "angle_accepted": False,
        "manual_visual_calibration_required": True,
    }


def build_review(m700: dict, lever: dict) -> dict:
    if str(m700["engine_version"]).split("-")[0] != str(lever["engine_version"]).split("-")[0]:
        fail(f"UE evidence versions differ: M700={m700['engine_version']} Lever={lever['engine_version']}")
    return {
        "schema": 1,
        "status": "ITEM16_M700_LEVER_LOCAL_UE58_PILOTS_REVIEWED_CALIBRATION_PENDING",
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
- M700 pilot max travel: `{m['pilot_max_travel']}`
- M700 sampled translation delta: `{m['max_sampled_translation_delta']}`
- M700 rotation calibration pending: `true`
- Lever pilot max angle: `{l['pilot_max_angle_deg']} deg`
- Lever sampled rotation delta: `{l['max_sampled_rotation_delta_deg']} deg`
- Final M700 travel/rotation accepted: `false`
- Final Lever angle accepted: `false`
- Full gameplay runtime now: `false`
- Next factual gate: `{review['next_factual_gate']}`
- `runtime_acceptance=0`
- `item16_checked=0`
- `merge_permitted=0`

This report consolidates pilot evidence only. Final motion values require direct current-head UE 5.8 visual calibration before production authoring/cutover.
"""
    OUT_MD.write_text(text, encoding="utf-8")


def main() -> int:
    m700 = validate_m700(read_json(M700_PATH, "M700"))
    lever = validate_lever(read_json(LEVER_PATH, "Lever Action"))
    review = build_review(m700, lever)
    write_report(review)
    print("PASS45_ITEM16_M700_LEVER_CALIBRATION_REVIEW_PASS")
    print(f"m700_pilot_max_travel={m700['pilot_max_travel']}")
    print(f"lever_pilot_max_angle_deg={lever['pilot_max_angle_deg']}")
    print("manual_visual_calibration_required=1")
    print("full_gameplay_runtime_now=0")
    print("runtime_acceptance=0")
    print("item16_checked=0")
    print("merge_permitted=0")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
