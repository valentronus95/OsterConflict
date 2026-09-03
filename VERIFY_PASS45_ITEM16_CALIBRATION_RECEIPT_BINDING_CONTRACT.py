#!/usr/bin/env python3
"""Synthetic regression contract for item-16 calibration -> production receipt binding."""
from __future__ import annotations

import copy
import hashlib
import json
import tempfile
from pathlib import Path

from VERIFY_PASS45_ITEM16_CALIBRATION_RECEIPT_BINDING import validate_pair

M700_SHA = "b7e003e01be8441e452730bc06c38c5e9752e523ae1b401ed2a6cc6cdca16840"
LEVER_SHA = "b2bf25bd47e9c4f6404897f67ad2a76a02971365fb7a689761936891d4591c69"
EVIDENCE_HEAD = "0123456789abcdef0123456789abcdef01234567"


def assert_case(label: str, errors: list[str], *, should_pass: bool, needle: str | None = None) -> list[str]:
    failures: list[str] = []
    passed = not errors
    if passed != should_pass:
        failures.append(f"{label}: expected_pass={int(should_pass)} errors={errors}")
    if needle is not None and not any(needle in error for error in errors):
        failures.append(f"{label}: expected error containing {needle!r}, got {errors}")
    return failures


def main() -> int:
    failures: list[str] = []
    with tempfile.TemporaryDirectory(prefix="pass45_item16_binding_") as temp_dir:
        approval_path = Path(temp_dir) / "approval.json"
        approval = {
            "schema": 1,
            "status": "ITEM16_MANUAL_ACTION_VISUAL_CALIBRATION_APPROVED_FOR_PRODUCTION_AUTHORING",
            "current_head_ue58_visual_calibration_accepted": True,
            "runtime_acceptance": False,
            "item16_checked": False,
            "merge_permitted": False,
            "evidence_head_sha": EVIDENCE_HEAD,
            "m700": {
                "source_sha256": M700_SHA,
                "accepted_translation": -6.25,
                "accepted_rotation_deg": 37.5,
            },
            "lever_action": {
                "source_sha256": LEVER_SHA,
                "accepted_angle_deg": -42.0,
            },
        }
        approval_path.write_text(json.dumps(approval, indent=2, sort_keys=True) + "\n", encoding="utf-8")
        approval_sha = hashlib.sha256(approval_path.read_bytes()).hexdigest()

        receipt = {
            "schema": 1,
            "status": "ITEM16_MANUAL_ACTION_PRODUCTION_ASSETS_AUTHORED",
            "runtime_acceptance": False,
            "item16_checked": False,
            "merge_permitted": False,
            "calibration_approval_sha256": approval_sha,
            "calibration_evidence_head_sha": EVIDENCE_HEAD,
            "m700": {
                "source_sha256": M700_SHA,
                "accepted_translation": -6.25,
                "accepted_rotation_deg": 37.5,
            },
            "lever_action": {
                "source_sha256": LEVER_SHA,
                "accepted_angle_deg": -42.0,
            },
        }

        failures.extend(assert_case("exact binding", validate_pair(approval_path, approval, receipt), should_pass=True))

        bad = copy.deepcopy(receipt)
        bad["calibration_approval_sha256"] = "0" * 64
        failures.extend(assert_case(
            "wrong approval hash",
            validate_pair(approval_path, approval, bad),
            should_pass=False,
            needle="exact calibration approval bytes",
        ))

        bad = copy.deepcopy(receipt)
        bad["calibration_evidence_head_sha"] = "f" * 40
        failures.extend(assert_case(
            "wrong evidence head",
            validate_pair(approval_path, approval, bad),
            should_pass=False,
            needle="calibration evidence head mismatch",
        ))

        bad = copy.deepcopy(receipt)
        bad["m700"]["accepted_translation"] = -7.0
        failures.extend(assert_case(
            "wrong M700 translation",
            validate_pair(approval_path, approval, bad),
            should_pass=False,
            needle="M700 accepted_translation mismatch",
        ))

        bad = copy.deepcopy(receipt)
        bad["m700"]["accepted_rotation_deg"] = 15.0
        failures.extend(assert_case(
            "wrong M700 rotation",
            validate_pair(approval_path, approval, bad),
            should_pass=False,
            needle="M700 accepted_rotation_deg mismatch",
        ))

        bad = copy.deepcopy(receipt)
        bad["lever_action"]["accepted_angle_deg"] = -45.0
        failures.extend(assert_case(
            "wrong Lever angle",
            validate_pair(approval_path, approval, bad),
            should_pass=False,
            needle="Lever Action accepted_angle_deg mismatch",
        ))

        bad = copy.deepcopy(receipt)
        bad["m700"]["source_sha256"] = LEVER_SHA
        failures.extend(assert_case(
            "cross-source M700 receipt",
            validate_pair(approval_path, approval, bad),
            should_pass=False,
            needle="M700 authoring receipt source SHA-256",
        ))

        bad = copy.deepcopy(receipt)
        bad["lever_action"]["accepted_angle_deg"] = True
        failures.extend(assert_case(
            "boolean angle rejected",
            validate_pair(approval_path, approval, bad),
            should_pass=False,
            needle="must be numeric, got boolean",
        ))

    if failures:
        print("PASS45 ITEM16 CALIBRATION RECEIPT BINDING CONTRACT: FAIL")
        for failure in failures:
            print(f"[FAIL] {failure}")
        raise SystemExit(1)

    print("PASS45 ITEM16 CALIBRATION RECEIPT BINDING CONTRACT: PASS")
    print("exact_hash=1 evidence_head=1 m700_translation=1 m700_rotation=1 lever_angle=1 source_sha=1 numeric_type=1")
    print("runtime_acceptance=0 item16_checked=0 merge_permitted=0 user_local_execution_requested=0")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
