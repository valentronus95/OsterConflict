#!/usr/bin/env python3
"""Fail-closed production-cutover preflight for PASS45 item 16.

The preflight owns calibration-state sequencing, not a second calibration-approval
schema. Exact approval identity/value validation is delegated to
VERIFY_PASS45_ITEM16_CALIBRATION_RECEIPT_BINDING.py so preflight and authoring receipt
binding cannot drift into competing definitions.

Valid states:

1. CALIBRATION GAP (current): no approval, no authoring receipt, M700 / Lever
   production manual-action profile paths empty.
2. CALIBRATION APPROVED: approval pins exact current-head UE 5.8 visual calibration;
   until a separate production authoring receipt exists, both profile paths stay empty.
3. AUTHORING RECEIPT PRESENT: this preflight validates the calibration approval and
   receipt state only, then delegates package/profile/runtime wiring to
   VERIFY_PASS45_ITEM16_PRODUCTION_PROFILE_CUTOVER.py.

No state here may claim runtime acceptance, close item 16 or permit PR #94 merge.
"""
from __future__ import annotations

import json
import re
from pathlib import Path

from VERIFY_PASS45_ITEM16_CALIBRATION_RECEIPT_BINDING import validate_approval

ROOT = Path(__file__).resolve().parent
PROFILES = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCWeaponAnimationProfiles.cpp"
APPROVAL = ROOT / "_DOCS" / "PASS45_ITEM16_MANUAL_ACTION_CALIBRATION_APPROVAL.json"
AUTHORING_RECEIPT = ROOT / "_DOCS" / "PASS45_ITEM16_PRODUCTION_AUTHORING_RECEIPT.json"

errors: list[str] = []


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


def read_text(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS45 ITEM16 PRODUCTION CUTOVER PREFLIGHT: FAIL\n[FAIL] missing file: {path}")
    return path.read_text(encoding="utf-8", errors="replace")


def load_json(path: Path, label: str) -> dict:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except Exception as exc:
        errors.append(f"{label} JSON invalid: {exc}")
        return {}
    if not isinstance(value, dict):
        errors.append(f"{label} root must be an object")
        return {}
    return value


def profile_manual_path(text: str, weapon_id: str) -> str | None:
    pattern = re.compile(
        r"\{\s*FName\(TEXT\(\"" + re.escape(weapon_id) + r"\"\)\)\s*,"
        r"\s*TEXT\(\"[^\"]*\"\)\s*,\s*TEXT\(\"[^\"]*\"\)\s*,\s*true\s*,"
        r"\s*TEXT\(\"([^\"]*)\"\)\s*,\s*true\s*\}",
        re.MULTILINE,
    )
    match = pattern.search(text)
    if not match:
        errors.append(f"cannot resolve required manual-action profile for {weapon_id}")
        return None
    return match.group(1)


def require_bool(obj: dict, key: str, expected: bool, label: str) -> None:
    actual = obj.get(key)
    req(actual is expected, f"{label} {key} expected={expected!r} actual={actual!r}")


profiles = read_text(PROFILES)
m700_path = profile_manual_path(profiles, "OC_SNP1")
lever_path = profile_manual_path(profiles, "R13_LEVER4570")
approval_present = APPROVAL.is_file()
receipt_present = AUTHORING_RECEIPT.is_file()

for forbidden in (
    "/Game/PASS45/ImportPilots/M700DerivedBoltTranslation",
    "AN_PASS45_M700_BoltTranslation_Pilot",
    "/Game/PASS45/ImportPilots/LeverActionDerivedLever",
    "AN_PASS45_LeverAction_Cycle_Pilot",
):
    req(forbidden not in profiles, f"calibration pilot leaked into production profile: {forbidden}")

if not approval_present:
    req(not receipt_present, "production authoring receipt exists before calibration approval")
    req(m700_path == "", f"M700 production manual-action path populated without approval: {m700_path!r}")
    req(lever_path == "", f"Lever production manual-action path populated without approval: {lever_path!r}")
else:
    approval = load_json(APPROVAL, "approval")
    if approval:
        errors.extend(validate_approval(approval))

    if not receipt_present:
        # Calibration approval only opens the authoring gate. Until a separate
        # authoring receipt exists, production profile wiring is still forbidden.
        req(m700_path == "", "M700 profile must remain empty until a production-authoring receipt exists")
        req(lever_path == "", "Lever profile must remain empty until a production-authoring receipt exists")
    else:
        receipt = load_json(AUTHORING_RECEIPT, "production authoring receipt")
        req(receipt.get("schema") == 1, f"production authoring receipt schema drifted: {receipt.get('schema')!r}")
        req(receipt.get("status") == "ITEM16_MANUAL_ACTION_PRODUCTION_ASSETS_AUTHORED",
            f"production authoring receipt status invalid: {receipt.get('status')!r}")
        require_bool(receipt, "runtime_acceptance", False, "production authoring receipt")
        require_bool(receipt, "item16_checked", False, "production authoring receipt")
        require_bool(receipt, "merge_permitted", False, "production authoring receipt")
        # Exact package/profile/runtime wiring is deliberately delegated to the
        # separate staged-cutover verifier. Do not reintroduce an empty-path rule here.

if errors:
    print("PASS45 ITEM16 PRODUCTION CUTOVER PREFLIGHT: FAIL")
    for error in errors:
        print(f"[FAIL] {error}")
    raise SystemExit(1)

if receipt_present:
    state = "AUTHORING_RECEIPT_DELEGATED_TO_PROFILE_CUTOVER_GUARD"
elif approval_present:
    state = "CALIBRATION_APPROVED_AUTHORING_PENDING"
else:
    state = "CALIBRATION_GAP_FAIL_CLOSED"

print("PASS45 ITEM16 PRODUCTION CUTOVER PREFLIGHT: PASS")
print(f"state={state}")
print(f"approval_present={int(approval_present)} authoring_receipt_present={int(receipt_present)}")
print(f"m700_production_manual_action_path_present={int(bool(m700_path))}")
print(f"lever_production_manual_action_path_present={int(bool(lever_path))}")
print("pilot_profile_leak=0 calibration_gate_fail_closed=1 approval_schema_single_source=1 staged_cutover_guard=1")
print("runtime_acceptance=0 item16_checked=0 merge_permitted=0 user_local_execution_requested=0")
