#!/usr/bin/env python3
"""Regression contract: item-16 profile cutover reuses canonical production/calibration bindings."""
from __future__ import annotations

import ast
from pathlib import Path

ROOT = Path(__file__).resolve().parent
PROFILE = ROOT / "VERIFY_PASS45_ITEM16_PRODUCTION_PROFILE_CUTOVER.py"
PACKAGE_BINDING_MODULE = "VERIFY_PASS45_ITEM16_PRODUCTION_PACKAGE_BINDING"
CALIBRATION_BINDING_MODULE = "VERIFY_PASS45_ITEM16_CALIBRATION_RECEIPT_BINDING"
PACKAGE_SYMBOL = "expected_package_file"
CALIBRATION_SYMBOLS = {
    "validate_approval",
    "validate_evidence_head_repository",
    "validate_pair",
}


def imported_names(tree: ast.AST, module: str) -> set[str]:
    names: set[str] = set()
    for node in ast.walk(tree):
        if isinstance(node, ast.ImportFrom) and node.module == module:
            names.update(alias.name for alias in node.names if alias.asname in (None, alias.name))
    return names


def called_names(tree: ast.AST) -> set[str]:
    names: set[str] = set()
    for node in ast.walk(tree):
        if isinstance(node, ast.Call) and isinstance(node.func, ast.Name):
            names.add(node.func.id)
    return names


def main() -> int:
    failures: list[str] = []
    tree = ast.parse(PROFILE.read_text(encoding="utf-8"), filename=str(PROFILE))

    forbidden_local_defs = {PACKAGE_SYMBOL, *CALIBRATION_SYMBOLS}
    local_defs = {
        node.name
        for node in ast.walk(tree)
        if isinstance(node, (ast.FunctionDef, ast.AsyncFunctionDef)) and node.name in forbidden_local_defs
    }
    if local_defs:
        failures.append(
            "profile cutover must not redefine canonical binding helpers: " + ", ".join(sorted(local_defs))
        )

    package_imports = imported_names(tree, PACKAGE_BINDING_MODULE)
    if PACKAGE_SYMBOL not in package_imports:
        failures.append(
            f"profile cutover must import {PACKAGE_SYMBOL} from {PACKAGE_BINDING_MODULE}"
        )

    calibration_imports = imported_names(tree, CALIBRATION_BINDING_MODULE)
    missing_imports = CALIBRATION_SYMBOLS - calibration_imports
    if missing_imports:
        failures.append(
            "profile cutover must import canonical calibration binding helpers: "
            + ", ".join(sorted(missing_imports))
        )

    calls = called_names(tree)
    if PACKAGE_SYMBOL not in calls:
        failures.append(f"profile cutover imports but does not call {PACKAGE_SYMBOL}")
    missing_calls = CALIBRATION_SYMBOLS - calls
    if missing_calls:
        failures.append(
            "profile cutover imports but does not call calibration binding helpers: "
            + ", ".join(sorted(missing_calls))
        )

    if failures:
        print("PASS45 ITEM16 PRODUCTION PATH SINGLE SOURCE CONTRACT: FAIL")
        for failure in failures:
            print(f"[FAIL] {failure}")
        print("runtime_acceptance=0 item16_checked=0 merge_permitted=0 user_local_execution_requested=0")
        return 1

    print("PASS45 ITEM16 PRODUCTION PATH SINGLE SOURCE CONTRACT: PASS")
    print("canonical_mapping_imported=1 duplicate_mapping_definition=0 calibration_binding_imports=1 calibration_binding_calls=1")
    print("runtime_acceptance=0 item16_checked=0 merge_permitted=0 user_local_execution_requested=0")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
