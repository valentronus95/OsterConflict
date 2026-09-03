#!/usr/bin/env python3
"""Bind PASS45 item-16 production authoring to the exact accepted calibration receipt.

This is a repository/source gate only. It prevents a future production-authoring
receipt from drifting away from the exact manual UE 5.8 calibration approval that
opened the authoring gate. It also binds that approval to a real ancestor commit and
rejects later changes to the calibration-critical M700 / Lever source chain. It does
not create calibration, runtime acceptance, or merge permission.
"""
from __future__ import annotations

import hashlib
import json
import math
import re
import subprocess
from pathlib import Path
from typing import Sequence

ROOT = Path(__file__).resolve().parent
APPROVAL = ROOT / "_DOCS" / "PASS45_ITEM16_MANUAL_ACTION_CALIBRATION_APPROVAL.json"
RECEIPT = ROOT / "_DOCS" / "PASS45_ITEM16_PRODUCTION_AUTHORING_RECEIPT.json"

APPROVAL_STATUS = "ITEM16_MANUAL_ACTION_VISUAL_CALIBRATION_APPROVED_FOR_PRODUCTION_AUTHORING"
RECEIPT_STATUS = "ITEM16_MANUAL_ACTION_PRODUCTION_ASSETS_AUTHORED"
M700_SOURCE = "OsterConflict/Content/Raw/R13/Weapons/SteinClassicWeapons/WeaponsPack/M700/SKM_M700.fbx"
M700_SOURCE_SHA256 = "b7e003e01be8441e452730bc06c38c5e9752e523ae1b401ed2a6cc6cdca16840"
LEVER_SOURCE = "OsterConflict/Content/Raw/R13/Weapons/SteinClassicWeapons/WeaponsPack/LeverAction/SKM_LeverAction.fbx"
LEVER_SOURCE_SHA256 = "b2bf25bd47e9c4f6404897f67ad2a76a02971365fb7a689761936891d4591c69"
HEX40 = re.compile(r"[0-9a-f]{40}")
HEX64 = re.compile(r"[0-9a-f]{64}")

CALIBRATION_CRITICAL_PATHS = (
    "PASS45_M700_DERIVED_BOLT_TRANSLATION_SOURCE.py",
    "PASS45_M700_DERIVED_BOLT_TRANSLATION_UE58_PILOT_COMPAT.py",
    "PASS45_M700_SOURCE_MOTION_AUDIT.py",
    "PASS45_M700_BOLT_GEOMETRY_AUDIT.py",
    "_DOCS/PASS45_M700_SOURCE_MOTION_AUDIT_2026-09-02.json",
    "_DOCS/PASS45_M700_BOLT_GEOMETRY_AUDIT_2026-09-02.json",
    "OsterConflict/Content/Raw/R13/Weapons/SteinClassicWeapons/WeaponsPack/M700/SKM_M700.fbx",
    "OsterConflict/TRY_PASS45_M700_DERIVED_BOLT_TRANSLATION_UE58_PILOT.cmd",
    "PASS45_LEVERACTION_DERIVED_LEVER_SOURCE.py",
    "PASS45_LEVERACTION_DERIVED_LEVER_UE58_PILOT_COMPAT.py",
    "PASS45_LEVERACTION_SOURCE_MOTION_AUDIT.py",
    "_DOCS/PASS45_LEVERACTION_SOURCE_MOTION_AUDIT_2026-09-02.json",
    "OsterConflict/Content/Raw/R13/Weapons/SteinClassicWeapons/WeaponsPack/LeverAction/SKM_LeverAction.fbx",
    "OsterConflict/TRY_PASS45_LEVERACTION_DERIVED_LEVER_UE58_PILOT.cmd",
    "PASS45_ITEM16_M700_LEVER_CALIBRATION_REVIEW.py",
    "OsterConflict/RUN_PASS45_ITEM16_LOCAL_UE58_EVIDENCE.cmd",
    "OsterConflict/REVIEW_PASS45_ITEM16_M700_LEVER_CALIBRATION.cmd",
)


def load_json(path: Path, label: str, errors: list[str]) -> dict:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except Exception as exc:
        errors.append(f"{label} JSON invalid: {exc}")
        return {}
    if not isinstance(value, dict):
        errors.append(f"{label} root must be an object")
        return {}
    return value


def require_false_flags(obj: dict, label: str, errors: list[str]) -> None:
    for key in ("runtime_acceptance", "item16_checked", "merge_permitted"):
        if obj.get(key) is not False:
            errors.append(f"{label} {key} must remain false, got {obj.get(key)!r}")


def as_finite_number(obj: dict, key: str, label: str, errors: list[str]) -> float | None:
    value = obj.get(key)
    if isinstance(value, bool):
        errors.append(f"{label} {key} must be numeric, got boolean {value!r}")
        return None
    try:
        parsed = float(value)
    except (TypeError, ValueError):
        errors.append(f"{label} {key} must be numeric, got {value!r}")
        return None
    if not math.isfinite(parsed):
        errors.append(f"{label} {key} must be finite, got {value!r}")
        return None
    return parsed


def require_nonzero_number(obj: dict, key: str, label: str, errors: list[str]) -> float | None:
    value = as_finite_number(obj, key, label, errors)
    if value is not None and abs(value) <= 1e-9:
        errors.append(f"{label} {key} must be non-zero")
    return value


def validate_approval(approval: dict) -> list[str]:
    """Own the complete repository-controlled calibration approval schema.

    Preflight and authoring-receipt binding both delegate here so exact source identity,
    accepted values and fail-closed flags cannot drift into competing definitions.
    """
    errors: list[str] = []
    if approval.get("schema") != 1:
        errors.append(f"approval schema must be 1, got {approval.get('schema')!r}")
    if approval.get("status") != APPROVAL_STATUS:
        errors.append(f"approval status invalid: {approval.get('status')!r}")
    if approval.get("current_head_ue58_visual_calibration_accepted") is not True:
        errors.append("approval lacks current-head UE 5.8 visual calibration acceptance")
    require_false_flags(approval, "approval", errors)

    evidence_head = str(approval.get("evidence_head_sha", ""))
    if not HEX40.fullmatch(evidence_head):
        errors.append(f"approval evidence_head_sha must be exact lowercase 40-hex SHA, got {evidence_head!r}")

    m700 = approval.get("m700")
    lever = approval.get("lever_action")
    if not isinstance(m700, dict):
        errors.append("approval m700 object missing")
    else:
        if m700.get("source") != M700_SOURCE:
            errors.append(f"approval.m700 source mismatch: {m700.get('source')!r}")
        if m700.get("source_sha256") != M700_SOURCE_SHA256:
            errors.append("approval.m700 source_sha256 does not match pinned M700 source")
        require_nonzero_number(m700, "accepted_translation", "approval.m700", errors)
        require_nonzero_number(m700, "accepted_rotation_deg", "approval.m700", errors)
        if m700.get("pilot_value_promoted_without_visual_review") is not False:
            errors.append("approval.m700 pilot_value_promoted_without_visual_review must be false")
    if not isinstance(lever, dict):
        errors.append("approval lever_action object missing")
    else:
        if lever.get("source") != LEVER_SOURCE:
            errors.append(f"approval.lever_action source mismatch: {lever.get('source')!r}")
        if lever.get("source_sha256") != LEVER_SOURCE_SHA256:
            errors.append("approval.lever_action source_sha256 does not match pinned Lever source")
        require_nonzero_number(lever, "accepted_angle_deg", "approval.lever_action", errors)
        if lever.get("pilot_value_promoted_without_visual_review") is not False:
            errors.append("approval.lever_action pilot_value_promoted_without_visual_review must be false")
    return errors


def _git(repo_root: Path, *args: str) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        ["git", *args],
        cwd=repo_root,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )


def validate_evidence_head_repository(
    evidence_head: str,
    *,
    repo_root: Path = ROOT,
    current_head: str = "HEAD",
    critical_paths: Sequence[str] = CALIBRATION_CRITICAL_PATHS,
) -> list[str]:
    """Prove that calibration evidence belongs to this lineage and is not stale.

    The accepted calibration commit may be an ancestor because approval and authoring
    necessarily create later commits. What may not happen is a calibration-critical
    source/input change after that evidence commit while the old approval remains live.
    """
    errors: list[str] = []
    if not HEX40.fullmatch(evidence_head):
        return errors

    shallow = _git(repo_root, "rev-parse", "--is-shallow-repository")
    if shallow.returncode != 0:
        errors.append(f"cannot determine repository history depth: {shallow.stderr.strip() or shallow.stdout.strip()}")
        return errors
    if shallow.stdout.strip().lower() == "true":
        errors.append("repository history is shallow; exact calibration evidence ancestry cannot be verified")
        return errors

    commit_check = _git(repo_root, "cat-file", "-e", f"{evidence_head}^{{commit}}")
    if commit_check.returncode != 0:
        errors.append(f"approval evidence_head_sha is not a repository commit: {evidence_head}")
        return errors

    head_check = _git(repo_root, "rev-parse", "--verify", f"{current_head}^{{commit}}")
    if head_check.returncode != 0:
        errors.append(f"current repository head cannot be resolved: {current_head}")
        return errors

    ancestor = _git(repo_root, "merge-base", "--is-ancestor", evidence_head, current_head)
    if ancestor.returncode != 0:
        errors.append(
            f"approval evidence_head_sha is not an ancestor of current repository head: "
            f"evidence={evidence_head} current={current_head}"
        )
        return errors

    diff = _git(repo_root, "diff", "--name-only", f"{evidence_head}..{current_head}", "--", *critical_paths)
    if diff.returncode != 0:
        errors.append(f"cannot compare calibration-critical source against evidence head: {diff.stderr.strip()}")
        return errors
    changed = sorted({line.strip() for line in diff.stdout.splitlines() if line.strip()})
    if changed:
        errors.append(
            "calibration-critical source changed after approved evidence head; fresh current-head UE 5.8 visual "
            "calibration is required: " + ", ".join(changed)
        )
    return errors


def validate_pair(approval_path: Path, approval: dict, receipt: dict) -> list[str]:
    errors = validate_approval(approval)
    if receipt.get("schema") != 1:
        errors.append(f"production authoring receipt schema must be 1, got {receipt.get('schema')!r}")
    if receipt.get("status") != RECEIPT_STATUS:
        errors.append(f"production authoring receipt status invalid: {receipt.get('status')!r}")
    require_false_flags(receipt, "production authoring receipt", errors)

    expected_approval_sha256 = hashlib.sha256(approval_path.read_bytes()).hexdigest()
    receipt_approval_sha256 = str(receipt.get("calibration_approval_sha256", ""))
    if not HEX64.fullmatch(receipt_approval_sha256):
        errors.append(
            "production authoring receipt calibration_approval_sha256 must be exact lowercase 64-hex"
        )
    elif receipt_approval_sha256 != expected_approval_sha256:
        errors.append(
            "production authoring receipt is not bound to the exact calibration approval bytes: "
            f"expected={expected_approval_sha256} actual={receipt_approval_sha256}"
        )

    evidence_head = str(approval.get("evidence_head_sha", ""))
    receipt_evidence_head = str(receipt.get("calibration_evidence_head_sha", ""))
    if receipt_evidence_head != evidence_head:
        errors.append(
            "production authoring receipt calibration evidence head mismatch: "
            f"approval={evidence_head!r} receipt={receipt_evidence_head!r}"
        )

    approval_m700 = approval.get("m700")
    approval_lever = approval.get("lever_action")
    receipt_m700 = receipt.get("m700")
    receipt_lever = receipt.get("lever_action")

    if not isinstance(receipt_m700, dict):
        errors.append("production authoring receipt m700 object missing")
    if not isinstance(receipt_lever, dict):
        errors.append("production authoring receipt lever_action object missing")

    if isinstance(approval_m700, dict) and isinstance(receipt_m700, dict):
        if receipt_m700.get("source_sha256") != approval_m700.get("source_sha256"):
            errors.append("M700 authoring receipt source SHA-256 does not match calibration approval")
        for key in ("accepted_translation", "accepted_rotation_deg"):
            approved = as_finite_number(approval_m700, key, "approval.m700", errors)
            authored = as_finite_number(receipt_m700, key, "receipt.m700", errors)
            if approved is not None and authored is not None and not math.isclose(
                approved, authored, rel_tol=0.0, abs_tol=1e-9
            ):
                errors.append(f"M700 {key} mismatch approval={approved} receipt={authored}")

    if isinstance(approval_lever, dict) and isinstance(receipt_lever, dict):
        if receipt_lever.get("source_sha256") != approval_lever.get("source_sha256"):
            errors.append("Lever Action authoring receipt source SHA-256 does not match calibration approval")
        approved = as_finite_number(approval_lever, "accepted_angle_deg", "approval.lever_action", errors)
        authored = as_finite_number(receipt_lever, "accepted_angle_deg", "receipt.lever_action", errors)
        if approved is not None and authored is not None and not math.isclose(
            approved, authored, rel_tol=0.0, abs_tol=1e-9
        ):
            errors.append(f"Lever Action accepted_angle_deg mismatch approval={approved} receipt={authored}")

    return errors


def main() -> int:
    approval_present = APPROVAL.is_file()
    receipt_present = RECEIPT.is_file()
    errors: list[str] = []

    if receipt_present and not approval_present:
        errors.append("production authoring receipt exists without calibration approval")
    elif approval_present:
        approval = load_json(APPROVAL, "calibration approval", errors)
        if approval:
            errors.extend(validate_evidence_head_repository(str(approval.get("evidence_head_sha", ""))))
        if receipt_present:
            receipt = load_json(RECEIPT, "production authoring receipt", errors)
            if approval and receipt:
                errors.extend(validate_pair(APPROVAL, approval, receipt))
        elif approval:
            errors.extend(validate_approval(approval))

    if errors:
        print("PASS45 ITEM16 CALIBRATION RECEIPT BINDING: FAIL")
        for error in errors:
            print(f"[FAIL] {error}")
        print("runtime_acceptance=0 item16_checked=0 merge_permitted=0 user_local_execution_requested=0")
        return 1

    if receipt_present:
        state = "AUTHORING_RECEIPT_BOUND_TO_EXACT_CALIBRATION"
    elif approval_present:
        state = "CALIBRATION_APPROVED_RECEIPT_PENDING"
    else:
        state = "CALIBRATION_GAP_NO_RECEIPT"

    print("PASS45 ITEM16 CALIBRATION RECEIPT BINDING: PASS")
    print(f"state={state}")
    print(f"approval_present={int(approval_present)} receipt_present={int(receipt_present)}")
    print("exact_approval_sha256_binding=1 exact_source_identity=1 evidence_head_binding=1 evidence_head_ancestor=1 calibration_critical_drift=0")
    print("authored_value_binding=1 source_sha_binding=1 approval_schema_single_source=1")
    print("runtime_acceptance=0 item16_checked=0 merge_permitted=0 user_local_execution_requested=0")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
