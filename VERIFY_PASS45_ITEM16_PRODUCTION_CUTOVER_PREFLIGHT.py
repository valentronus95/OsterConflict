#!/usr/bin/env python3
"""Fail-closed production-cutover preflight for PASS45 item 16.

Current repository truth intentionally has no accepted production M700 bolt cycle
or Lever Action lever cycle. This guard makes that boundary executable instead of
leaving it as a comment humans can heroically forget six weeks later.

Two states are valid:

1. CALIBRATION GAP (current): no calibration approval artifact exists and M700 /
   Lever production manual-action profile paths MUST remain empty.
2. CALIBRATION APPROVED (future): a repository-controlled approval JSON may exist,
   but it must pin the exact Stein source identities, current-head UE 5.8 visual
   calibration acceptance and explicit final motion values. Even then this verifier
   does not author assets, wire profile paths, claim runtime acceptance, close item
   16 or permit merge.

Pilot paths or pilot constants are never accepted as production cutover evidence.
"""
from __future__ import annotations

import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent
PROFILES = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCWeaponAnimationProfiles.cpp"
APPROVAL = ROOT / "_DOCS" / "PASS45_ITEM16_MANUAL_ACTION_CALIBRATION_APPROVAL.json"

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


def require_bool(obj: dict, key: str, expected: bool) -> None:
    actual = obj.get(key)
    req(actual is expected, f"approval {key} expected={expected!r} actual={actual!r}")


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

for forbidden in (
    "/Game/PASS45/ImportPilots/M700DerivedBoltTranslation",
    "AN_PASS45_M700_BoltTranslation_Pilot",
    "/Game/PASS45/ImportPilots/LeverActionDerivedLever",
    "AN_PASS45_LeverAction_Cycle_Pilot",
):
    req(forbidden not in profiles, f"calibration pilot leaked into production profile: {forbidden}")

approval_present = APPROVAL.is_file()
if not approval_present:
    req(m700_path == "", f"M700 production manual-action path populated without approval: {m700_path!r}")
    req(lever_path == "", f"Lever production manual-action path populated without approval: {lever_path!r}")
else:
    try:
        approval = json.loads(APPROVAL.read_text(encoding="utf-8"))
    except Exception as exc:
        errors.append(f"approval JSON invalid: {exc}")
        approval = {}
    req(isinstance(approval, dict), "approval root must be an object")
    if isinstance(approval, dict):
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

        # Calibration approval merely opens the authoring gate. It never proves that
        # production assets/profile wiring or runtime acceptance have happened.
        req(m700_path == "", "M700 profile must remain empty until a separate production-authoring commit")
        req(lever_path == "", "Lever profile must remain empty until a separate production-authoring commit")

if errors:
    print("PASS45 ITEM16 PRODUCTION CUTOVER PREFLIGHT: FAIL")
    for error in errors:
        print(f"[FAIL] {error}")
    raise SystemExit(1)

print("PASS45 ITEM16 PRODUCTION CUTOVER PREFLIGHT: PASS")
print(f"approval_present={int(approval_present)}")
print(f"m700_production_manual_action_path_present={int(bool(m700_path))}")
print(f"lever_production_manual_action_path_present={int(bool(lever_path))}")
print("pilot_profile_leak=0 production_authoring_gate_fail_closed=1")
print("runtime_acceptance=0 item16_checked=0 merge_permitted=0 user_local_execution_requested=0")
