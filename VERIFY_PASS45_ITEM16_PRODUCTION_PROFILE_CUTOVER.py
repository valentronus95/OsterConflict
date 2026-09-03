#!/usr/bin/env python3
"""Fail-closed state machine for PASS45 item-16 production profile cutover.

This guard keeps calibration approval, production asset authoring, gameplay-profile
cutover and runtime evidence as distinct states. It intentionally passes the current
repository state where M700 and Lever Action still have empty required manual-action
paths. Future work may advance only when the preceding repository-controlled receipt
exists; a calibration pilot can never become production merely by changing a path.
"""
from __future__ import annotations

import json
import re
from pathlib import Path

from VERIFY_PASS45_ITEM16_CALIBRATION_RECEIPT_BINDING import (
    validate_approval,
    validate_evidence_head_repository,
    validate_pair,
)
from VERIFY_PASS45_ITEM16_PRODUCTION_PACKAGE_BINDING import (
    LEVER_PREFIX,
    M700_PREFIX,
    validate_authored_package,
)

ROOT = Path(__file__).resolve().parent
PROFILES = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCWeaponAnimationProfiles.cpp"
RUNTIME_EVIDENCE = ROOT / "VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py"
APPROVAL = ROOT / "_DOCS" / "PASS45_ITEM16_MANUAL_ACTION_CALIBRATION_APPROVAL.json"
AUTHORING_RECEIPT = ROOT / "_DOCS" / "PASS45_ITEM16_PRODUCTION_AUTHORING_RECEIPT.json"

errors: list[str] = []


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


def read_text(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS45 ITEM16 PRODUCTION PROFILE CUTOVER: FAIL\n[FAIL] missing file: {path}")
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


def validate_authored_asset(entry: object, *, label: str, prefix: str) -> str | None:
    # Receipt-entry shape, byte identity, canonical object-path mapping, production
    # namespace ownership, pilot rejection and package existence all have one owner in
    # VERIFY_PASS45_ITEM16_PRODUCTION_PACKAGE_BINDING.py. Donor-source identity is
    # already bound by validate_pair() -> validate_approval(). Profile cutover only
    # consumes those canonical decisions and returns the receipt object path.
    errors.extend(validate_authored_package(entry, label=label, prefix=prefix, root=ROOT))
    if not isinstance(entry, dict):
        return None
    return str(entry.get("sequence_object_path", "")) or None


profiles = read_text(PROFILES)
runtime_evidence = read_text(RUNTIME_EVIDENCE)
m700_profile_path = profile_manual_path(profiles, "OC_SNP1")
lever_profile_path = profile_manual_path(profiles, "R13_LEVER4570")
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
    req(m700_profile_path == "", f"M700 production path populated before calibration approval: {m700_profile_path!r}")
    req(lever_profile_path == "", f"Lever production path populated before calibration approval: {lever_profile_path!r}")
elif not receipt_present:
    approval = load_json(APPROVAL, "calibration approval")
    errors.extend(validate_approval(approval))
    errors.extend(validate_evidence_head_repository(str(approval.get("evidence_head_sha", ""))))
    req(m700_profile_path == "", f"M700 profile cut over before production authoring receipt: {m700_profile_path!r}")
    req(lever_profile_path == "", f"Lever profile cut over before production authoring receipt: {lever_profile_path!r}")
else:
    approval = load_json(APPROVAL, "calibration approval")
    receipt = load_json(AUTHORING_RECEIPT, "production authoring receipt")
    errors.extend(validate_pair(APPROVAL, approval, receipt))
    errors.extend(validate_evidence_head_repository(str(approval.get("evidence_head_sha", ""))))

    m700_receipt_path = validate_authored_asset(
        receipt.get("m700"), label="M700", prefix=M700_PREFIX
    )
    lever_receipt_path = validate_authored_asset(
        receipt.get("lever_action"), label="LeverAction", prefix=LEVER_PREFIX
    )
    if m700_receipt_path:
        req(m700_profile_path == m700_receipt_path,
            f"M700 profile/receipt mismatch profile={m700_profile_path!r} receipt={m700_receipt_path!r}")
        for marker in (
            "PASS45_MANUAL_ACTION_AUTHORED_SOURCE_BRIDGE_READY",
            "weapon=OC_SNP1",
            "EOCWeaponActionType::BoltAction",
            m700_receipt_path,
        ):
            req(marker in runtime_evidence, f"strict runtime evidence is not armed for M700 cutover: {marker}")
    if lever_receipt_path:
        req(lever_profile_path == lever_receipt_path,
            f"Lever profile/receipt mismatch profile={lever_profile_path!r} receipt={lever_receipt_path!r}")
        for marker in (
            "PASS45_MANUAL_ACTION_AUTHORED_SOURCE_BRIDGE_READY",
            "weapon=R13_LEVER4570",
            "EOCWeaponActionType::LeverAction",
            lever_receipt_path,
        ):
            req(marker in runtime_evidence, f"strict runtime evidence is not armed for Lever cutover: {marker}")

# No repository-controlled pre-runtime stage may self-promote acceptance.
for path in (APPROVAL, AUTHORING_RECEIPT):
    if path.is_file():
        text = path.read_text(encoding="utf-8", errors="replace")
        for forbidden in ('"runtime_acceptance": true', '"item16_checked": true', '"merge_permitted": true'):
            req(forbidden not in text, f"{path.name} contains forbidden acceptance promotion: {forbidden}")

if errors:
    print("PASS45 ITEM16 PRODUCTION PROFILE CUTOVER: FAIL")
    for error in errors:
        print(f"[FAIL] {error}")
    raise SystemExit(1)

if receipt_present:
    state = "PRODUCTION_AUTHORED_AND_PROFILE_CUTOVER_GUARDED"
elif approval_present:
    state = "CALIBRATION_APPROVED_AUTHORING_PENDING"
else:
    state = "CALIBRATION_GAP_FAIL_CLOSED"

print("PASS45 ITEM16 PRODUCTION PROFILE CUTOVER: PASS")
print(f"state={state}")
print(f"approval_present={int(approval_present)} authoring_receipt_present={int(receipt_present)}")
print(f"m700_profile_path_present={int(bool(m700_profile_path))} lever_profile_path_present={int(bool(lever_profile_path))}")
print("pilot_profile_leak=0 staged_cutover=1 production_package_sha256_required=1 canonical_package_mapping_single_source=1 production_namespace_single_source=1 production_package_validation_single_source=1 calibration_approval_schema_single_source=1 calibration_donor_sha_single_source=1 profile_direct_donor_sha_check=0 exact_approval_receipt_binding=1 calibration_evidence_ancestry_required=1 strict_runtime_evidence_required_after_authoring=1")
print("runtime_acceptance=0 item16_checked=0 merge_permitted=0 user_local_execution_requested=0")
