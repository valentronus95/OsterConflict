#!/usr/bin/env python3
"""Static fail-closed contract for PASS45 2026-09-03 asset intake."""
from __future__ import annotations

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent


def read(rel: str) -> str:
    path = ROOT / rel
    if not path.is_file():
        raise SystemExit(f"PASS45 ASSET INTAKE CONTRACT: FAIL\n[FAIL] missing file: {rel}")
    return path.read_text(encoding="utf-8", errors="replace")


errors: list[str] = []


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


spec = read("_DOCS/PASS45_ASSET_INTAKE_2026-09-03.md")
auditor = read("PASS45_ASSET_INTAKE_20260903.py")
launcher = read("OsterConflict/RUN_PASS45_ASSET_INTAKE_20260903.cmd")
history = read("PASS45_RUNTIME_RECOVERY_HISTORY.md")

for marker in (
    "QUARANTINED / NOT PRODUCTION / NOT RUNTIME-ACCEPTED",
    "Never merge `asset-intake-20260903` wholesale",
    "18 ZIP archives",
    "asset_intake_quarantine=1",
    "runtime_acceptance=0",
    "item16_checked=0",
    "merge_permitted=0",
    "one consolidated current-head weapon runtime acceptance",
):
    req(marker in spec, f"spec missing required marker: {marker}")

for marker in (
    "PASS45_ASSET_INTAKE_AUDIT_COMPLETE",
    "runtime_acceptance=0",
    "item16_checked=0",
    "merge_permitted=0",
    "REJECT_ARCHIVE",
    "NEEDS_PROVENANCE",
    "AUDITABLE_CANDIDATE",
    "unsafe_member",
    "sha256_file",
):
    req(marker in auditor, f"auditor missing required marker/function: {marker}")

for forbidden in (
    "subprocess.",
    "os.system",
    "shutil.rmtree",
    "unlink(",
    "git checkout",
    "git reset",
    "git clean",
    "git pull",
    "git push",
    "git merge",
    "unrealeditor",
):
    req(forbidden.lower() not in auditor.lower(), f"auditor contains forbidden mutation/execution token: {forbidden}")

req("extractall(" not in auditor.lower(), "auditor must not bulk-extract archives")
req("zipfile.ZipFile" in auditor, "auditor must inspect ZIP archives")
req("PurePosixPath" in auditor, "auditor must perform path-safety checks")
req("PC_TEST" in auditor and "TEST_RESULTS" in auditor, "auditor report must stay in ignored local test-results path")

for marker in (
    "No Git commands, no UE import, no production extraction, no deletion.",
    "PASS45_ASSET_INTAKE_AUDIT_PASS",
    "runtime_acceptance=0",
    "item16_checked=0",
    "merge_permitted=0",
):
    req(marker in launcher, f"launcher missing safety/status marker: {marker}")

for line in launcher.splitlines():
    command = line.strip().lower()
    req(
        re.match(r"^(?:call\s+)?(?:git|gh)(?:\.exe)?(?:\s|$)", command) is None,
        f"launcher directly invokes Git/GitHub CLI: {line.strip()}",
    )
    req(
        re.match(r"^(?:del|erase|rmdir|robocopy|xcopy|unrealeditor)(?:\.exe)?(?:\s|$)", command) is None,
        f"launcher contains forbidden direct mutation/execution command: {line.strip()}",
    )

for marker in (
    "asset-intake-20260903",
    "22/36 = 61.1%",
    "item 16",
    "Do **not** run the expensive full UE 5.8 runtime acceptance after every tiny weapon tweak.",
    "one consolidated current-head weapon runtime acceptance",
    "PR #94 OPEN / UNMERGED",
):
    req(marker in history, f"history missing checkpoint marker: {marker}")

req("items 37+" in spec.lower(), "spec must explicitly preserve frozen 36-item architecture")
req("No checklist items 37+ are created." in history, "history must preserve frozen 36-item architecture")

if errors:
    print("PASS45 ASSET INTAKE CONTRACT: FAIL")
    for error in errors:
        print(f"[FAIL] {error}")
    raise SystemExit(1)

print("PASS45 ASSET INTAKE CONTRACT: PASS")
print("quarantine_only=1")
print("runtime_acceptance=0")
print("item16_checked=0")
print("merge_permitted=0")
