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

ROOT = Path(__file__).resolve().parent
PROFILES = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCWeaponAnimationProfiles.cpp"
RUNTIME_EVIDENCE = ROOT / "VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py"
APPROVAL = ROOT / "_DOCS" / "PASS45_ITEM16_MANUAL_ACTION_CALIBRATION_APPROVAL.json"
AUTHORING_RECEIPT = ROOT / "_DOCS" / "PASS45_ITEM16_PRODUCTION_AUTHORING_RECEIPT.json"

M700_SOURCE_SHA256 = "b7e003e01be8441e452730bc06c38c5e9752e523ae1b401ed2a6cc6cdca16840"
LEVER_SOURCE_SHA256 = "b2bf25bd47e9c4f6404897f67ad2a76a02971365fb7a689761936891d4591c69"
M700_PREFIX = "/Game/Production/Weapons/M700/"
LEVER_PREFIX = "/Game/Production/Weapons/LeverAction/"

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


def expected_package_file(object_path: str) -> str | None:
    if not object_path.startswith("/Game/") or "." not in object_path:
        return None
    package_object, object_name = object_path.rsplit(".", 1)
    asset_name = package_object.rsplit("/", 1)[-1]
    if not object_name or object_name != asset_name:
        return None
    return "OsterConflict/Content/" + package_object[len("/Game/"):] + ".uasset"


def validate_authored_asset(entry: object, *, label: str, prefix: str, source_sha256: str) -> str | None:
    if not isinstance(entry, dict):
        errors.append(f"authoring receipt missing {label} object")
        return None
    object_path = str(entry.get("sequence_object_path", ""))
    package_file = str(entry.get("package_file", ""))
    req(object_path.startswith(prefix), f"{label} sequence is outside production namespace: {object_path!r}")
    req("/ImportPilots/" not in object_path and "Pilot" not in object_path,
        f"{label} sequence points at calibration pilot: {object_path!r}")
    derived_file = expected_package_file(object_path)
    req(derived_file is not None, f"{label} sequence object path is not canonical: {object_path!r}")
    if derived_file is not None:
        req(package_file == derived_file,
            f"{label} package mapping mismatch expected={derived_file!r} actual={package_file!r}")
    req(entry.get("source_sha256") == source_sha256, f"{label} source SHA-256 drifted in authoring receipt")
    if package_file:
        req((ROOT / package_file).is_file(), f"{label} authored production package missing: {package_file}")
    return object_path or None


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
    req(approval.get("status") == "ITEM16_MANUAL_ACTION_VISUAL_CALIBRATION_APPROVED_FOR_PRODUCTION_AUTHORING",
        f"calibration approval status invalid: {approval.get('status')!r}")
    req(approval.get("current_head_ue58_visual_calibration_accepted") is True,
        "calibration approval lacks current-head UE 5.8 visual acceptance")
    req(m700_profile_path == "", f"M700 profile cut over before production authoring receipt: {m700_profile_path!r}")
    req(lever_profile_path == "", f"Lever profile cut over before production authoring receipt: {lever_profile_path!r}")
else:
    approval = load_json(APPROVAL, "calibration approval")
    receipt = load_json(AUTHORING_RECEIPT, "production authoring receipt")
    req(approval.get("status") == "ITEM16_MANUAL_ACTION_VISUAL_CALIBRATION_APPROVED_FOR_PRODUCTION_AUTHORING",
        f"calibration approval status invalid: {approval.get('status')!r}")
    req(approval.get("current_head_ue58_visual_calibration_accepted") is True,
        "production authoring cannot follow an unaccepted calibration")
    req(receipt.get("schema") == 1, f"production authoring receipt schema drifted: {receipt.get('schema')!r}")
    req(receipt.get("status") == "ITEM16_MANUAL_ACTION_PRODUCTION_ASSETS_AUTHORED",
        f"production authoring receipt status invalid: {receipt.get('status')!r}")
    for key in ("runtime_acceptance", "item16_checked", "merge_permitted"):
        req(receipt.get(key) is False, f"production authoring receipt illegally promotes {key}")

    m700_receipt_path = validate_authored_asset(
        receipt.get("m700"), label="M700", prefix=M700_PREFIX, source_sha256=M700_SOURCE_SHA256
    )
    lever_receipt_path = validate_authored_asset(
        receipt.get("lever_action"), label="LeverAction", prefix=LEVER_PREFIX, source_sha256=LEVER_SOURCE_SHA256
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
print("pilot_profile_leak=0 staged_cutover=1 strict_runtime_evidence_required_after_authoring=1")
print("runtime_acceptance=0 item16_checked=0 merge_permitted=0 user_local_execution_requested=0")
