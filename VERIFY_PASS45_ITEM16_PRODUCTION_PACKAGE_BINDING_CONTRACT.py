#!/usr/bin/env python3
"""Synthetic regression contract for PASS45 item-16 production package byte binding."""
from __future__ import annotations

import copy
import hashlib
import tempfile
from pathlib import Path

from VERIFY_PASS45_ITEM16_PRODUCTION_PACKAGE_BINDING import (
    M700_PREFIX,
    validate_authored_package,
)


def assert_case(
    label: str,
    errors: list[str],
    *,
    should_pass: bool,
    needle: str | None = None,
) -> list[str]:
    failures: list[str] = []
    passed = not errors
    if passed != should_pass:
        failures.append(f"{label}: expected_pass={int(should_pass)} errors={errors}")
    if needle is not None and not any(needle in error for error in errors):
        failures.append(f"{label}: expected error containing {needle!r}, got {errors}")
    return failures


def main() -> int:
    failures: list[str] = []
    with tempfile.TemporaryDirectory(prefix="pass45_item16_package_binding_") as temp_dir:
        root = Path(temp_dir)
        package_rel = "OsterConflict/Content/Production/Weapons/M700/AN_M700_BoltCycle.uasset"
        package_path = root / package_rel
        package_path.parent.mkdir(parents=True, exist_ok=True)
        original_bytes = b"PASS45 synthetic authored M700 sequence v1\x00\x01"
        package_path.write_bytes(original_bytes)
        package_sha256 = hashlib.sha256(original_bytes).hexdigest()

        entry = {
            "sequence_object_path": "/Game/Production/Weapons/M700/AN_M700_BoltCycle.AN_M700_BoltCycle",
            "package_file": package_rel,
            "package_sha256": package_sha256,
        }

        failures.extend(
            assert_case(
                "exact package bytes",
                validate_authored_package(entry, label="M700", prefix=M700_PREFIX, root=root),
                should_pass=True,
            )
        )

        missing_hash = copy.deepcopy(entry)
        missing_hash.pop("package_sha256")
        failures.extend(
            assert_case(
                "missing package hash",
                validate_authored_package(missing_hash, label="M700", prefix=M700_PREFIX, root=root),
                should_pass=False,
                needle="package_sha256 must be exact lowercase 64-hex",
            )
        )

        wrong_hash = copy.deepcopy(entry)
        wrong_hash["package_sha256"] = "0" * 64
        failures.extend(
            assert_case(
                "wrong package hash",
                validate_authored_package(wrong_hash, label="M700", prefix=M700_PREFIX, root=root),
                should_pass=False,
                needle="production package SHA-256 mismatch",
            )
        )

        package_path.write_bytes(original_bytes + b" mutated-after-receipt")
        failures.extend(
            assert_case(
                "same-path package replacement",
                validate_authored_package(entry, label="M700", prefix=M700_PREFIX, root=root),
                should_pass=False,
                needle="production package SHA-256 mismatch",
            )
        )

        package_path.write_bytes(original_bytes)
        wrong_mapping = copy.deepcopy(entry)
        wrong_mapping["package_file"] = (
            "OsterConflict/Content/Production/Weapons/M700/AN_DIFFERENT.uasset"
        )
        failures.extend(
            assert_case(
                "wrong object-to-package mapping",
                validate_authored_package(wrong_mapping, label="M700", prefix=M700_PREFIX, root=root),
                should_pass=False,
                needle="package mapping mismatch",
            )
        )

        pilot_path = copy.deepcopy(entry)
        pilot_path["sequence_object_path"] = (
            "/Game/PASS45/ImportPilots/M700DerivedBoltTranslation/"
            "AN_PASS45_M700_BoltTranslation_Pilot.AN_PASS45_M700_BoltTranslation_Pilot"
        )
        failures.extend(
            assert_case(
                "pilot namespace rejected",
                validate_authored_package(pilot_path, label="M700", prefix=M700_PREFIX, root=root),
                should_pass=False,
                needle="outside production namespace",
            )
        )

    if failures:
        print("PASS45 ITEM16 PRODUCTION PACKAGE BINDING CONTRACT: FAIL")
        for failure in failures:
            print(f"[FAIL] {failure}")
        raise SystemExit(1)

    print("PASS45 ITEM16 PRODUCTION PACKAGE BINDING CONTRACT: PASS")
    print("exact_bytes=1 missing_hash_rejected=1 wrong_hash_rejected=1 same_path_replacement_rejected=1 canonical_mapping=1 pilot_namespace_rejected=1")
    print("runtime_acceptance=0 item16_checked=0 merge_permitted=0 user_local_execution_requested=0")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
