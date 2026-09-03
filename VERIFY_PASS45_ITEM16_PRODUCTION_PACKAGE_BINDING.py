#!/usr/bin/env python3
"""Bind PASS45 item-16 production animation receipts to exact package bytes.

Calibration approval and source identity are already guarded elsewhere. This gate
closes the next provenance gap: once a production-authoring receipt exists, the
receipt must pin the exact SHA-256 of every authored M700 / Lever Action .uasset.
A later same-path package replacement must therefore fail closed instead of silently
remaining "authored" merely because the file still exists.

This is repository/source evidence only. It does not create calibration, runtime
acceptance, item-16 closure or merge permission.
"""
from __future__ import annotations

import hashlib
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent
RECEIPT = ROOT / "_DOCS" / "PASS45_ITEM16_PRODUCTION_AUTHORING_RECEIPT.json"

RECEIPT_STATUS = "ITEM16_MANUAL_ACTION_PRODUCTION_ASSETS_AUTHORED"
M700_PREFIX = "/Game/Production/Weapons/M700/"
LEVER_PREFIX = "/Game/Production/Weapons/LeverAction/"
HEX64 = re.compile(r"[0-9a-f]{64}")


def expected_package_file(object_path: str) -> str | None:
    if not object_path.startswith("/Game/") or "." not in object_path:
        return None
    package_object, object_name = object_path.rsplit(".", 1)
    asset_name = package_object.rsplit("/", 1)[-1]
    if not object_name or object_name != asset_name:
        return None
    return "OsterConflict/Content/" + package_object[len("/Game/"):] + ".uasset"


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def validate_authored_package(
    entry: object,
    *,
    label: str,
    prefix: str,
    root: Path = ROOT,
) -> list[str]:
    errors: list[str] = []
    if not isinstance(entry, dict):
        return [f"production authoring receipt missing {label} object"]

    object_path = str(entry.get("sequence_object_path", ""))
    package_file = str(entry.get("package_file", ""))
    package_sha256 = str(entry.get("package_sha256", ""))

    if not object_path.startswith(prefix):
        errors.append(f"{label} sequence is outside production namespace: {object_path!r}")
    if "/ImportPilots/" in object_path or "Pilot" in object_path:
        errors.append(f"{label} sequence points at calibration pilot: {object_path!r}")

    derived_file = expected_package_file(object_path)
    if derived_file is None:
        errors.append(f"{label} sequence object path is not canonical: {object_path!r}")
    elif package_file != derived_file:
        errors.append(
            f"{label} package mapping mismatch expected={derived_file!r} actual={package_file!r}"
        )

    if not HEX64.fullmatch(package_sha256):
        errors.append(f"{label} package_sha256 must be exact lowercase 64-hex")

    if package_file:
        package_path = root / package_file
        try:
            package_path.resolve().relative_to(root.resolve())
        except ValueError:
            errors.append(f"{label} package path escapes repository root: {package_file!r}")
        else:
            if not package_path.is_file():
                errors.append(f"{label} authored production package missing: {package_file}")
            elif HEX64.fullmatch(package_sha256):
                actual_sha256 = sha256_file(package_path)
                if actual_sha256 != package_sha256:
                    errors.append(
                        f"{label} production package SHA-256 mismatch "
                        f"expected={package_sha256} actual={actual_sha256}"
                    )

    return errors


def load_receipt(path: Path) -> tuple[dict, list[str]]:
    errors: list[str] = []
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except Exception as exc:
        return {}, [f"production authoring receipt JSON invalid: {exc}"]
    if not isinstance(value, dict):
        return {}, ["production authoring receipt root must be an object"]
    return value, errors


def validate_receipt(receipt: dict, *, root: Path = ROOT) -> list[str]:
    errors: list[str] = []
    if receipt.get("schema") != 1:
        errors.append(f"production authoring receipt schema must be 1, got {receipt.get('schema')!r}")
    if receipt.get("status") != RECEIPT_STATUS:
        errors.append(f"production authoring receipt status invalid: {receipt.get('status')!r}")
    for key in ("runtime_acceptance", "item16_checked", "merge_permitted"):
        if receipt.get(key) is not False:
            errors.append(f"production authoring receipt {key} must remain false")

    errors.extend(
        validate_authored_package(
            receipt.get("m700"), label="M700", prefix=M700_PREFIX, root=root
        )
    )
    errors.extend(
        validate_authored_package(
            receipt.get("lever_action"), label="LeverAction", prefix=LEVER_PREFIX, root=root
        )
    )
    return errors


def main() -> int:
    if not RECEIPT.is_file():
        print("PASS45 ITEM16 PRODUCTION PACKAGE BINDING: PASS")
        print("state=AUTHORING_RECEIPT_ABSENT package_byte_binding_armed=1")
        print("runtime_acceptance=0 item16_checked=0 merge_permitted=0 user_local_execution_requested=0")
        return 0

    receipt, errors = load_receipt(RECEIPT)
    if receipt:
        errors.extend(validate_receipt(receipt))

    if errors:
        print("PASS45 ITEM16 PRODUCTION PACKAGE BINDING: FAIL")
        for error in errors:
            print(f"[FAIL] {error}")
        print("runtime_acceptance=0 item16_checked=0 merge_permitted=0 user_local_execution_requested=0")
        return 1

    print("PASS45 ITEM16 PRODUCTION PACKAGE BINDING: PASS")
    print("state=AUTHORING_RECEIPT_PACKAGE_BYTES_BOUND package_sha256=1 same_path_replacement_fail_closed=1")
    print("runtime_acceptance=0 item16_checked=0 merge_permitted=0 user_local_execution_requested=0")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
