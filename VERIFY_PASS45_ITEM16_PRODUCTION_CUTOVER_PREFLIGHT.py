#!/usr/bin/env python3
"""Fail-closed production-cutover preflight for PASS45 item 16.

The preflight owns calibration-approval truth, not production profile wiring. The
staged cutover verifier owns the later authoring-receipt/profile/runtime-evidence
transition. This separation prevents two CI guards from demanding contradictory
states when production authoring eventually becomes factual.

Valid states:

1. CALIBRATION GAP (current): no approval, no authoring receipt, M700 / Lever
   production manual-action profile paths empty.
2. CALIBRATION APPROVED: approval pins exact current-head UE 5.8 visual calibration;
   until a separate production authoring receipt exists, both profile paths stay empty.
3. AUTHORING RECEIPT PRESENT: this preflight validates the calibration approval and
   receipt identity only, then delegates package/profile/runtime wiring to
   VERIFY_PASS45_ITEM16_PRODUCTION_PROFILE_CUTOVER.py.

No state here may claim runtime acceptance, close item 16 or permit PR #94 merge.
"""
from __future__ import annotations

import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent
PROFILES = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCWeaponAnimationProfiles.cpp"
APPROVAL = ROOT / "_DOCS" / "PASS45_ITEM16_MANUAL_ACTION_CALIBRATION_APPROVAL.json"
AUTHORING_RECEIPT = ROOT / "_DOCS" / "PASS45_ITEM16_PRODUCTION_AUTHORING_RECEIPT.json"

M700_SOURCE = "OsterConflict/Content/Raw/R13/Weapons/SteinClassicWeapons/WeaponsPack/M700/SKM_M700.fbx"
M700_SHA256 = "b7e003e01be8441e452730bc06c38c5e9752e523ae1b401ed2a6cc6cdca16840"
LEVER_SOURCE = "OsterConflict/Content/Raw/R13/Weapons/SteinClassicWeapons/WeaponsPack/LeverAction/SKM_LeverAction.fbx"
LEVER_SHA256 = "b2bf25bd47e9c4f6404897f67ad2a76a02971365fb7a689761936891d4591c69"

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


def require_bool(obj: dict, key: str, expected: bool, label: str = "approval") -> None:
    actual = obj.get(key)
    req(actual is expected, f"{label} {key} expected={expected!r} actual={actual!r}")


def require_number(obj: dict, key: str, *, nonzero: bool = True) -> float | None:
    value = obj.get(key)
    try:
        parsed = float(value)
    except (TypeError, ValueError):
        errors.append(f"approval missing/invalid numeric {key}: {value!r}")
        return None
    if nonzero and abs(parsed) <= 1e-9:
        errors.append(f"approval {key} must be non-zero")
    return parsed


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
    req(approval.get("schema") == 1, f"approval schema drifted: {approval.get('schema')!r}")
    req(approval.get("status") == "ITEM16_MANUAL_ACTION_VISUAL_CALIBRATION_APPROVED_FOR_PRODUCTION_AUTHORING",
        f"approval status invalid: {approval.get('status')!r}")
    require_bool(approval, "current_head_ue58_visual_calibration_accepted", True)
    require_bool(approval, "runtime_acceptance", False)
    require_bool(approval, "item16_checked", False)
    require_bool(approval, "merge_permitted", False)

    m700 = approval.get("m700")
    lever = approval.get("lever_action")
    req(isinstance(m700, dict), "approval m700 object missing")
    req(isinstance(lever, dict), "approval lever_action object missing")
    if isinstance(m700, dict):
        req(m700.get("source") == M700_SOURCE, f"M700 approval source drifted: {m700.get('source')!r}")
        req(m700.get("source_sha256") == M700_SHA256, "M700 approval source SHA-256 drifted")
        require_number(m700, "accepted_translation")
        require_number(m700, "accepted_rotation_deg")
        require_bool(m700, "pilot_value_promoted_without_visual_review", False)
    if isinstance(lever, dict):
        req(lever.get("source") == LEVER_SOURCE, f"Lever approval source drifted: {lever.get('source')!r}")
        req(lever.get("source_sha256") == LEVER_SHA256, "Lever approval source SHA-256 drifted")
        require_number(lever, "accepted_angle_deg")
        require_bool(lever, "pilot_value_promoted_without_visual_review", False)

    evidence_head = str(approval.get("evidence_head_sha", ""))
    req(bool(re.fullmatch(r"[0-9a-f]{40}", evidence_head)),
        f"approval evidence_head_sha must be exact 40-hex SHA, got {evidence_head!r}")

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
print("pilot_profile_leak=0 calibration_gate_fail_closed=1 staged_cutover_guard=1")
print("runtime_acceptance=0 item16_checked=0 merge_permitted=0 user_local_execution_requested=0")
