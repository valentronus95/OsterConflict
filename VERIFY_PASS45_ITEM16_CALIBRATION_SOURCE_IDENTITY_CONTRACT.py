#!/usr/bin/env python3
"""Regression contract for single-source item-16 calibration donor identity."""
from __future__ import annotations

import ast
import re
from pathlib import Path

from PASS45_ITEM16_CALIBRATION_SOURCE_IDENTITY import (
    LEVER_SOURCE,
    LEVER_SOURCE_SHA256,
    M700_SOURCE,
    M700_SOURCE_SHA256,
)
from VERIFY_PASS45_ITEM16_CALIBRATION_RECEIPT_BINDING import CALIBRATION_CRITICAL_PATHS

ROOT = Path(__file__).resolve().parent
IDENTITY = ROOT / "PASS45_ITEM16_CALIBRATION_SOURCE_IDENTITY.py"
REVIEW = ROOT / "PASS45_ITEM16_M700_LEVER_CALIBRATION_REVIEW.py"
BINDING = ROOT / "VERIFY_PASS45_ITEM16_CALIBRATION_RECEIPT_BINDING.py"
BINDING_CONTRACT = ROOT / "VERIFY_PASS45_ITEM16_CALIBRATION_RECEIPT_BINDING_CONTRACT.py"
PROFILE_CUTOVER = ROOT / "VERIFY_PASS45_ITEM16_PRODUCTION_PROFILE_CUTOVER.py"
MODULE = "PASS45_ITEM16_CALIBRATION_SOURCE_IDENTITY"
HEX64 = re.compile(r"[0-9a-f]{64}")


def imports_identity(path: Path, required_names: set[str]) -> bool:
    tree = ast.parse(path.read_text(encoding="utf-8"), filename=str(path))
    imported: set[str] = set()
    for node in ast.walk(tree):
        if isinstance(node, ast.ImportFrom) and node.module == MODULE:
            imported.update(alias.name for alias in node.names)
    return required_names.issubset(imported)


def main() -> int:
    failures: list[str] = []
    expected = {
        "M700_SOURCE": M700_SOURCE,
        "M700_SOURCE_SHA256": M700_SOURCE_SHA256,
        "LEVER_SOURCE": LEVER_SOURCE,
        "LEVER_SOURCE_SHA256": LEVER_SOURCE_SHA256,
    }

    if not M700_SOURCE.endswith("/M700/SKM_M700.fbx"):
        failures.append(f"unexpected M700 source path: {M700_SOURCE}")
    if not LEVER_SOURCE.endswith("/LeverAction/SKM_LeverAction.fbx"):
        failures.append(f"unexpected Lever source path: {LEVER_SOURCE}")
    for label, value in (("M700", M700_SOURCE_SHA256), ("Lever", LEVER_SOURCE_SHA256)):
        if not HEX64.fullmatch(value):
            failures.append(f"{label} source SHA-256 is not lowercase 64-hex: {value!r}")

    full_identity_names = {"M700_SOURCE", "M700_SOURCE_SHA256", "LEVER_SOURCE", "LEVER_SOURCE_SHA256"}
    sha_identity_names = {"M700_SOURCE_SHA256", "LEVER_SOURCE_SHA256"}
    consumers = (
        (BINDING, "calibration receipt binding", full_identity_names),
        (REVIEW, "calibration review", full_identity_names),
        (BINDING_CONTRACT, "calibration receipt binding contract", full_identity_names),
        (PROFILE_CUTOVER, "production profile cutover", sha_identity_names),
    )
    for path, label, required_names in consumers:
        if not imports_identity(path, required_names):
            failures.append(f"{label} does not import required donor identity from the canonical module")

    identity_text = IDENTITY.read_text(encoding="utf-8")
    for path, _label, _required_names in consumers:
        text = path.read_text(encoding="utf-8")
        for name, literal in expected.items():
            if literal in text:
                failures.append(f"{path.name} re-hardcodes {name} instead of importing canonical identity")
        if MODULE not in text:
            failures.append(f"{path.name} lacks canonical donor-identity module import")

    if "PASS45_ITEM16_CALIBRATION_SOURCE_IDENTITY.py" not in CALIBRATION_CRITICAL_PATHS:
        failures.append("canonical donor-identity module is not calibration-critical for ancestry invalidation")

    for name, literal in expected.items():
        if literal not in identity_text:
            failures.append(f"canonical identity module lost {name}")

    if failures:
        print("PASS45 ITEM16 CALIBRATION SOURCE IDENTITY CONTRACT: FAIL")
        for failure in failures:
            print(f"[FAIL] {failure}")
        print("runtime_acceptance=0 item16_checked=0 merge_permitted=0 user_local_execution_requested=0")
        return 1

    print("PASS45 ITEM16 CALIBRATION SOURCE IDENTITY CONTRACT: PASS")
    print("single_source=1 review_imports=1 binding_imports=1 binding_contract_imports=1 profile_cutover_sha_imports=1 identity_change_invalidates_approval=1")
    print("runtime_acceptance=0 item16_checked=0 merge_permitted=0 user_local_execution_requested=0")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
