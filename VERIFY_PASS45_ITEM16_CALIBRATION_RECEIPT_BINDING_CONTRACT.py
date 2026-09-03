#!/usr/bin/env python3
"""Synthetic regression contract for item-16 calibration -> production receipt binding."""
from __future__ import annotations

import ast
import copy
import hashlib
import json
import subprocess
import tempfile
from pathlib import Path

from PASS45_ITEM16_CALIBRATION_SOURCE_IDENTITY import (
    LEVER_SOURCE,
    LEVER_SOURCE_SHA256,
    M700_SOURCE,
    M700_SOURCE_SHA256,
)
from VERIFY_PASS45_ITEM16_CALIBRATION_RECEIPT_BINDING import (
    validate_approval,
    validate_authoring_receipt_header,
    validate_evidence_head_repository,
    validate_pair,
)

ROOT = Path(__file__).resolve().parent
BINDING = ROOT / "VERIFY_PASS45_ITEM16_CALIBRATION_RECEIPT_BINDING.py"
PACKAGE_BINDING = ROOT / "VERIFY_PASS45_ITEM16_PRODUCTION_PACKAGE_BINDING.py"
PREFLIGHT = ROOT / "VERIFY_PASS45_ITEM16_PRODUCTION_CUTOVER_PREFLIGHT.py"
CALIBRATION_BINDING_MODULE = "VERIFY_PASS45_ITEM16_CALIBRATION_RECEIPT_BINDING"
RECEIPT_STATUS = "ITEM16_MANUAL_ACTION_PRODUCTION_ASSETS_AUTHORED"
RECEIPT_HEADER_KEYS = {"schema", "status", "runtime_acceptance", "item16_checked", "merge_permitted"}
EVIDENCE_HEAD = "0123456789abcdef0123456789abcdef01234567"


def assert_case(label: str, errors: list[str], *, should_pass: bool, needle: str | None = None) -> list[str]:
    failures: list[str] = []
    passed = not errors
    if passed != should_pass:
        failures.append(f"{label}: expected_pass={int(should_pass)} errors={errors}")
    if needle is not None and not any(needle in error for error in errors):
        failures.append(f"{label}: expected error containing {needle!r}, got {errors}")
    return failures


def imported_names(path: Path, module: str) -> set[str]:
    tree = ast.parse(path.read_text(encoding="utf-8"), filename=str(path))
    names: set[str] = set()
    for node in ast.walk(tree):
        if isinstance(node, ast.ImportFrom) and node.module == module:
            names.update(alias.name for alias in node.names)
    return names


def receipt_get_keys(path: Path, *, function_name: str | None = None) -> set[str]:
    tree = ast.parse(path.read_text(encoding="utf-8"), filename=str(path))
    root: ast.AST = tree
    if function_name is not None:
        matches = [
            node for node in tree.body
            if isinstance(node, (ast.FunctionDef, ast.AsyncFunctionDef)) and node.name == function_name
        ]
        if not matches:
            return {"<missing-function>"}
        root = matches[0]
    keys: set[str] = set()
    for node in ast.walk(root):
        if not isinstance(node, ast.Call) or not isinstance(node.func, ast.Attribute):
            continue
        if node.func.attr != "get" or not isinstance(node.func.value, ast.Name):
            continue
        if node.func.value.id != "receipt" or not node.args:
            continue
        first = node.args[0]
        if isinstance(first, ast.Constant) and isinstance(first.value, str):
            keys.add(first.value)
    return keys


def called_names(path: Path, function_name: str) -> set[str]:
    tree = ast.parse(path.read_text(encoding="utf-8"), filename=str(path))
    matches = [
        node for node in tree.body
        if isinstance(node, (ast.FunctionDef, ast.AsyncFunctionDef)) and node.name == function_name
    ]
    if not matches:
        return set()
    names: set[str] = set()
    for node in ast.walk(matches[0]):
        if isinstance(node, ast.Call) and isinstance(node.func, ast.Name):
            names.add(node.func.id)
    return names


def git(repo: Path, *args: str) -> str:
    result = subprocess.run(
        ["git", *args],
        cwd=repo,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )
    if result.returncode != 0:
        raise RuntimeError(f"git {' '.join(args)} failed: {result.stderr}")
    return result.stdout.strip()


def provenance_contract(failures: list[str]) -> None:
    critical = "PASS45_M700_DERIVED_BOLT_TRANSLATION_SOURCE.py"
    with tempfile.TemporaryDirectory(prefix="pass45_item16_provenance_") as temp_dir:
        repo = Path(temp_dir)
        git(repo, "init", "-q")
        git(repo, "config", "user.email", "pass45@example.invalid")
        git(repo, "config", "user.name", "PASS45 Contract")

        (repo / critical).write_text("pilot = 1\n", encoding="utf-8")
        git(repo, "add", critical)
        git(repo, "commit", "-q", "-m", "base calibration source")
        evidence_head = git(repo, "rev-parse", "HEAD")

        (repo / "README.txt").write_text("non critical\n", encoding="utf-8")
        git(repo, "add", "README.txt")
        git(repo, "commit", "-q", "-m", "non-critical change")
        failures.extend(assert_case(
            "non-critical descendant allowed",
            validate_evidence_head_repository(
                evidence_head,
                repo_root=repo,
                critical_paths=(critical,),
            ),
            should_pass=True,
        ))

        (repo / critical).write_text("pilot = 2\n", encoding="utf-8")
        git(repo, "add", critical)
        git(repo, "commit", "-q", "-m", "critical drift")
        failures.extend(assert_case(
            "critical descendant rejected",
            validate_evidence_head_repository(
                evidence_head,
                repo_root=repo,
                critical_paths=(critical,),
            ),
            should_pass=False,
            needle="calibration-critical source changed",
        ))

        failures.extend(assert_case(
            "unknown evidence commit rejected",
            validate_evidence_head_repository(
                "f" * 40,
                repo_root=repo,
                critical_paths=(critical,),
            ),
            should_pass=False,
            needle="not a repository commit",
        ))

        orphan = git(repo, "commit-tree", git(repo, "write-tree"), "-m", "detached unrelated")
        failures.extend(assert_case(
            "non-ancestor evidence rejected",
            validate_evidence_head_repository(
                orphan,
                repo_root=repo,
                critical_paths=(critical,),
            ),
            should_pass=False,
            needle="not an ancestor",
        ))


def main() -> int:
    failures: list[str] = []
    provenance_contract(failures)

    for path, label in (
        (PACKAGE_BINDING, "production package binding"),
        (PREFLIGHT, "production cutover preflight"),
    ):
        imports = imported_names(path, CALIBRATION_BINDING_MODULE)
        if "validate_authoring_receipt_header" not in imports:
            failures.append(f"{label} must import canonical validate_authoring_receipt_header")
        duplicate_keys = RECEIPT_HEADER_KEYS & receipt_get_keys(path)
        if duplicate_keys:
            failures.append(
                f"{label} revalidates authoring receipt header keys instead of delegating: "
                + ", ".join(sorted(duplicate_keys))
            )
        if RECEIPT_STATUS in path.read_text(encoding="utf-8"):
            failures.append(f"{label} re-hardcodes production authoring receipt status")

    pair_calls = called_names(BINDING, "validate_pair")
    if "validate_authoring_receipt_header" not in pair_calls:
        failures.append("validate_pair must delegate authoring receipt base schema to validate_authoring_receipt_header")
    pair_duplicate_keys = RECEIPT_HEADER_KEYS & receipt_get_keys(BINDING, function_name="validate_pair")
    if pair_duplicate_keys:
        failures.append(
            "validate_pair revalidates authoring receipt header keys directly: "
            + ", ".join(sorted(pair_duplicate_keys))
        )

    with tempfile.TemporaryDirectory(prefix="pass45_item16_binding_") as temp_dir:
        approval_path = Path(temp_dir) / "approval.json"
        approval = {
            "schema": 1,
            "status": "ITEM16_MANUAL_ACTION_VISUAL_CALIBRATION_APPROVED_FOR_PRODUCTION_AUTHORING",
            "current_head_ue58_visual_calibration_accepted": True,
            "runtime_acceptance": False,
            "item16_checked": False,
            "merge_permitted": False,
            "evidence_head_sha": EVIDENCE_HEAD,
            "m700": {
                "source": M700_SOURCE,
                "source_sha256": M700_SOURCE_SHA256,
                "accepted_translation": -6.25,
                "accepted_rotation_deg": 37.5,
                "pilot_value_promoted_without_visual_review": False,
            },
            "lever_action": {
                "source": LEVER_SOURCE,
                "source_sha256": LEVER_SOURCE_SHA256,
                "accepted_angle_deg": -42.0,
                "pilot_value_promoted_without_visual_review": False,
            },
        }
        approval_path.write_text(json.dumps(approval, indent=2, sort_keys=True) + "\n", encoding="utf-8")
        approval_sha = hashlib.sha256(approval_path.read_bytes()).hexdigest()

        receipt = {
            "schema": 1,
            "status": RECEIPT_STATUS,
            "runtime_acceptance": False,
            "item16_checked": False,
            "merge_permitted": False,
            "calibration_approval_sha256": approval_sha,
            "calibration_evidence_head_sha": EVIDENCE_HEAD,
            "m700": {
                "source_sha256": M700_SOURCE_SHA256,
                "accepted_translation": -6.25,
                "accepted_rotation_deg": 37.5,
            },
            "lever_action": {
                "source_sha256": LEVER_SOURCE_SHA256,
                "accepted_angle_deg": -42.0,
            },
        }

        failures.extend(assert_case("exact approval", validate_approval(approval), should_pass=True))
        failures.extend(assert_case(
            "exact authoring receipt header",
            validate_authoring_receipt_header(receipt),
            should_pass=True,
        ))
        failures.extend(assert_case("exact binding", validate_pair(approval_path, approval, receipt), should_pass=True))

        bad_header = copy.deepcopy(receipt)
        bad_header["schema"] = 2
        failures.extend(assert_case(
            "wrong authoring receipt schema",
            validate_authoring_receipt_header(bad_header),
            should_pass=False,
            needle="schema must be 1",
        ))
        failures.extend(assert_case(
            "pair rejects wrong authoring receipt schema",
            validate_pair(approval_path, approval, bad_header),
            should_pass=False,
            needle="schema must be 1",
        ))

        bad_header = copy.deepcopy(receipt)
        bad_header["status"] = "WRONG_STATUS"
        failures.extend(assert_case(
            "wrong authoring receipt status",
            validate_authoring_receipt_header(bad_header),
            should_pass=False,
            needle="status invalid",
        ))

        bad_header = copy.deepcopy(receipt)
        bad_header["merge_permitted"] = True
        failures.extend(assert_case(
            "authoring receipt cannot self-promote merge",
            validate_authoring_receipt_header(bad_header),
            should_pass=False,
            needle="merge_permitted must remain false",
        ))

        bad_approval = copy.deepcopy(approval)
        bad_approval["m700"]["source"] = LEVER_SOURCE
        failures.extend(assert_case(
            "wrong M700 approval source",
            validate_approval(bad_approval),
            should_pass=False,
            needle="approval.m700 source mismatch",
        ))

        bad_approval = copy.deepcopy(approval)
        bad_approval["m700"]["source_sha256"] = LEVER_SOURCE_SHA256
        failures.extend(assert_case(
            "wrong M700 approval SHA",
            validate_approval(bad_approval),
            should_pass=False,
            needle="pinned M700 source",
        ))

        bad_approval = copy.deepcopy(approval)
        bad_approval["m700"]["accepted_rotation_deg"] = 0.0
        failures.extend(assert_case(
            "zero M700 approved rotation",
            validate_approval(bad_approval),
            should_pass=False,
            needle="accepted_rotation_deg must be non-zero",
        ))

        bad_approval = copy.deepcopy(approval)
        bad_approval["lever_action"]["accepted_angle_deg"] = 0.0
        failures.extend(assert_case(
            "zero Lever approved angle",
            validate_approval(bad_approval),
            should_pass=False,
            needle="accepted_angle_deg must be non-zero",
        ))

        bad_approval = copy.deepcopy(approval)
        bad_approval["lever_action"]["pilot_value_promoted_without_visual_review"] = True
        failures.extend(assert_case(
            "blind pilot promotion rejected",
            validate_approval(bad_approval),
            should_pass=False,
            needle="pilot_value_promoted_without_visual_review must be false",
        ))

        bad = copy.deepcopy(receipt)
        bad["calibration_approval_sha256"] = "0" * 64
        failures.extend(assert_case(
            "wrong approval hash",
            validate_pair(approval_path, approval, bad),
            should_pass=False,
            needle="exact calibration approval bytes",
        ))

        bad = copy.deepcopy(receipt)
        bad["calibration_evidence_head_sha"] = "f" * 40
        failures.extend(assert_case(
            "wrong evidence head",
            validate_pair(approval_path, approval, bad),
            should_pass=False,
            needle="calibration evidence head mismatch",
        ))

        bad = copy.deepcopy(receipt)
        bad["m700"]["accepted_translation"] = -7.0
        failures.extend(assert_case(
            "wrong M700 translation",
            validate_pair(approval_path, approval, bad),
            should_pass=False,
            needle="M700 accepted_translation mismatch",
        ))

        bad = copy.deepcopy(receipt)
        bad["m700"]["accepted_rotation_deg"] = 15.0
        failures.extend(assert_case(
            "wrong M700 rotation",
            validate_pair(approval_path, approval, bad),
            should_pass=False,
            needle="M700 accepted_rotation_deg mismatch",
        ))

        bad = copy.deepcopy(receipt)
        bad["lever_action"]["accepted_angle_deg"] = -45.0
        failures.extend(assert_case(
            "wrong Lever angle",
            validate_pair(approval_path, approval, bad),
            should_pass=False,
            needle="Lever Action accepted_angle_deg mismatch",
        ))

        bad = copy.deepcopy(receipt)
        bad["m700"]["source_sha256"] = LEVER_SOURCE_SHA256
        failures.extend(assert_case(
            "cross-source M700 receipt",
            validate_pair(approval_path, approval, bad),
            should_pass=False,
            needle="M700 authoring receipt source SHA-256",
        ))

        bad = copy.deepcopy(receipt)
        bad["lever_action"]["accepted_angle_deg"] = True
        failures.extend(assert_case(
            "boolean angle rejected",
            validate_pair(approval_path, approval, bad),
            should_pass=False,
            needle="must be numeric, got boolean",
        ))

    if failures:
        print("PASS45 ITEM16 CALIBRATION RECEIPT BINDING CONTRACT: FAIL")
        for failure in failures:
            print(f"[FAIL] {failure}")
        raise SystemExit(1)

    print("PASS45 ITEM16 CALIBRATION RECEIPT BINDING CONTRACT: PASS")
    print("exact_hash=1 evidence_head=1 ancestor=1 critical_drift=1 exact_source=1 nonzero_values=1 pilot_promotion_rejected=1 m700_translation=1 m700_rotation=1 lever_angle=1 source_sha=1 numeric_type=1 authoring_receipt_header_single_source=1 header_consumers_delegate=1")
    print("runtime_acceptance=0 item16_checked=0 merge_permitted=0 user_local_execution_requested=0")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
