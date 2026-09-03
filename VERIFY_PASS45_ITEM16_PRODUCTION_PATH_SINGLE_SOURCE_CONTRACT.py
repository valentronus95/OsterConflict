#!/usr/bin/env python3
"""Regression contract: item-16 production path mapping has one source of truth."""
from __future__ import annotations

import ast
from pathlib import Path

ROOT = Path(__file__).resolve().parent
PROFILE = ROOT / "VERIFY_PASS45_ITEM16_PRODUCTION_PROFILE_CUTOVER.py"
BINDING_MODULE = "VERIFY_PASS45_ITEM16_PRODUCTION_PACKAGE_BINDING"
SYMBOL = "expected_package_file"


def main() -> int:
    failures: list[str] = []
    tree = ast.parse(PROFILE.read_text(encoding="utf-8"), filename=str(PROFILE))

    local_defs = [
        node for node in ast.walk(tree)
        if isinstance(node, (ast.FunctionDef, ast.AsyncFunctionDef)) and node.name == SYMBOL
    ]
    if local_defs:
        failures.append(f"profile cutover must not define local {SYMBOL}()")

    imported = False
    for node in ast.walk(tree):
        if isinstance(node, ast.ImportFrom) and node.module == BINDING_MODULE:
            for alias in node.names:
                if alias.name == SYMBOL and alias.asname in (None, SYMBOL):
                    imported = True
    if not imported:
        failures.append(
            f"profile cutover must import {SYMBOL} from {BINDING_MODULE}"
        )

    if failures:
        print("PASS45 ITEM16 PRODUCTION PATH SINGLE SOURCE CONTRACT: FAIL")
        for failure in failures:
            print(f"[FAIL] {failure}")
        print("runtime_acceptance=0 item16_checked=0 merge_permitted=0 user_local_execution_requested=0")
        return 1

    print("PASS45 ITEM16 PRODUCTION PATH SINGLE SOURCE CONTRACT: PASS")
    print("canonical_mapping_imported=1 duplicate_mapping_definition=0")
    print("runtime_acceptance=0 item16_checked=0 merge_permitted=0 user_local_execution_requested=0")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
