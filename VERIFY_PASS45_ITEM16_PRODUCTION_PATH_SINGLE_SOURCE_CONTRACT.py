#!/usr/bin/env python3
"""Regression contract: item-16 profile cutover reuses canonical production/calibration bindings."""
from __future__ import annotations

import ast
from pathlib import Path

ROOT = Path(__file__).resolve().parent
PROFILE = ROOT / "VERIFY_PASS45_ITEM16_PRODUCTION_PROFILE_CUTOVER.py"
PACKAGE_BINDING_MODULE = "VERIFY_PASS45_ITEM16_PRODUCTION_PACKAGE_BINDING"
CALIBRATION_BINDING_MODULE = "VERIFY_PASS45_ITEM16_CALIBRATION_RECEIPT_BINDING"
SOURCE_IDENTITY_MODULE = "PASS45_ITEM16_CALIBRATION_SOURCE_IDENTITY"
PACKAGE_SYMBOLS = {"validate_authored_package", "M700_PREFIX", "LEVER_PREFIX"}
CALIBRATION_SYMBOLS = {
    "validate_approval",
    "validate_evidence_head_repository",
    "validate_pair",
}
FORBIDDEN_LOCAL_VALIDATION_DIAGNOSTICS = {
    "authoring receipt missing",
    "sequence is outside production namespace",
    "sequence points at calibration pilot",
    "sequence object path is not canonical",
    "package mapping mismatch",
    "authored production package missing",
    "source SHA-256 drifted in authoring receipt",
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


def assigned_names(tree: ast.AST) -> set[str]:
    names: set[str] = set()
    for node in ast.walk(tree):
        if isinstance(node, (ast.Assign, ast.AnnAssign)):
            targets = node.targets if isinstance(node, ast.Assign) else [node.target]
            for target in targets:
                if isinstance(target, ast.Name):
                    names.add(target.id)
    return names


def loaded_names(tree: ast.AST) -> set[str]:
    return {
        node.id
        for node in ast.walk(tree)
        if isinstance(node, ast.Name) and isinstance(node.ctx, ast.Load)
    }


def main() -> int:
    failures: list[str] = []
    profile_text = PROFILE.read_text(encoding="utf-8")
    tree = ast.parse(profile_text, filename=str(PROFILE))

    forbidden_local_defs = {"validate_authored_package", *CALIBRATION_SYMBOLS}
    local_defs = {
        node.name
        for node in ast.walk(tree)
        if isinstance(node, (ast.FunctionDef, ast.AsyncFunctionDef)) and node.name in forbidden_local_defs
    }
    if local_defs:
        failures.append(
            "profile cutover must not redefine canonical binding helpers: " + ", ".join(sorted(local_defs))
        )

    assignments = assigned_names(tree)
    duplicate_namespace_owners = {"M700_PREFIX", "LEVER_PREFIX"} & assignments
    if duplicate_namespace_owners:
        failures.append(
            "profile cutover must not own production namespace prefixes: "
            + ", ".join(sorted(duplicate_namespace_owners))
        )

    package_imports = imported_names(tree, PACKAGE_BINDING_MODULE)
    missing_package_imports = PACKAGE_SYMBOLS - package_imports
    if missing_package_imports:
        failures.append(
            "profile cutover must import canonical package binding symbols: "
            + ", ".join(sorted(missing_package_imports))
        )

    calibration_imports = imported_names(tree, CALIBRATION_BINDING_MODULE)
    missing_calibration_imports = CALIBRATION_SYMBOLS - calibration_imports
    if missing_calibration_imports:
        failures.append(
            "profile cutover must import canonical calibration binding helpers: "
            + ", ".join(sorted(missing_calibration_imports))
        )

    direct_source_identity_imports = imported_names(tree, SOURCE_IDENTITY_MODULE)
    if direct_source_identity_imports:
        failures.append(
            "profile cutover must consume donor identity through canonical calibration binding, not import it directly: "
            + ", ".join(sorted(direct_source_identity_imports))
        )

    calls = called_names(tree)
    if "validate_authored_package" not in calls:
        failures.append("profile cutover imports but does not call validate_authored_package")
    if "expected_package_file" in calls:
        failures.append("profile cutover must not revalidate canonical package mapping directly")
    missing_calls = CALIBRATION_SYMBOLS - calls
    if missing_calls:
        failures.append(
            "profile cutover imports but does not call calibration binding helpers: "
            + ", ".join(sorted(missing_calls))
        )

    loads = loaded_names(tree)
    missing_namespace_uses = {"M700_PREFIX", "LEVER_PREFIX"} - loads
    if missing_namespace_uses:
        failures.append(
            "profile cutover imports but does not use canonical production namespace prefixes: "
            + ", ".join(sorted(missing_namespace_uses))
        )

    repeated_local_rules = sorted(
        diagnostic for diagnostic in FORBIDDEN_LOCAL_VALIDATION_DIAGNOSTICS if diagnostic in profile_text
    )
    if repeated_local_rules:
        failures.append(
            "profile cutover repeats canonical production/calibration validation rules: "
            + ", ".join(repeated_local_rules)
        )

    if failures:
        print("PASS45 ITEM16 PRODUCTION PATH SINGLE SOURCE CONTRACT: FAIL")
        for failure in failures:
            print(f"[FAIL] {failure}")
        print("runtime_acceptance=0 item16_checked=0 merge_permitted=0 user_local_execution_requested=0")
        return 1

    print("PASS45 ITEM16 PRODUCTION PATH SINGLE SOURCE CONTRACT: PASS")
    print("canonical_package_validator_imported=1 canonical_package_validator_called=1 duplicate_package_validation=0 production_namespace_imported=1 duplicate_namespace_owner=0 calibration_binding_imports=1 calibration_binding_calls=1 direct_source_identity_import=0 duplicate_donor_sha_validation=0")
    print("runtime_acceptance=0 item16_checked=0 merge_permitted=0 user_local_execution_requested=0")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
