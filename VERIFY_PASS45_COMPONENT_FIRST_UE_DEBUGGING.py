#!/usr/bin/env python3
"""Static contract for PASS45 component-first local UE debugging cadence."""
from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parent
errors: list[str] = []


def read(rel: str) -> str:
    path = ROOT / rel
    if not path.is_file():
        raise SystemExit(f"PASS45 COMPONENT-FIRST UE DEBUGGING: FAIL\n[FAIL] missing file: {rel}")
    return path.read_text(encoding="utf-8", errors="replace")


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


agents = read("AGENTS.md")
protocol = read("_DOCS/PASS45_COMPONENT_FIRST_UE_DEBUGGING_PROTOCOL.md")
ledger = read("OSTER_CONFLICT_WORK_LEDGER.md")
history = read("PASS45_RUNTIME_RECOVERY_HISTORY.md")
full_chain = read("OsterConflict/RUN_PASS45_ITEM16_LOCAL_UE58_EVIDENCE.cmd")
lever_launcher = read("OsterConflict/TRY_PASS45_LEVERACTION_DERIVED_LEVER_UE58_PILOT.cmd")

req(
    "31. **Use component-first local UE debugging; the user is not the first test environment.**" in agents,
    "AGENTS.md rule 31 is missing",
)
req(
    "_DOCS/PASS45_COMPONENT_FIRST_UE_DEBUGGING_PROTOCOL.md" in agents,
    "AGENTS.md does not bind the component-first protocol",
)

for needle in (
    "When a fail-closed local UE chain stops at one component, that component becomes the only local rerun target until it passes",
    "Do **not** ask the user to rerun earlier phases that already passed",
    "Level A — static / remote preflight",
    "Level B — single-component UE proof",
    "Level C — consolidated chain / gameplay acceptance",
    "The full gameplay route is an acceptance gate, not a Python/asset-authoring debugger.",
    "OsterConflict/TRY_PASS45_LEVERACTION_DERIVED_LEVER_UE58_PILOT.cmd",
    "Do not rerun the five-phase chain until this Lever-only proof passes.",
):
    req(needle in protocol, f"component-first protocol invariant missing: {needle}")

for text, label in ((ledger, "ledger"), (history, "history")):
    req("component-first" in text.lower(), f"{label} does not record component-first cadence")
    req(
        "OsterConflict/TRY_PASS45_LEVERACTION_DERIVED_LEVER_UE58_PILOT.cmd" in text,
        f"{label} does not name the current Lever-only local operation",
    )
    req(
        "RUN_PASS45_ITEM16_LOCAL_UE58_EVIDENCE.cmd" in text,
        f"{label} lost the later consolidated five-phase chain",
    )
    req(
        "22/36 = 61.1%" in text,
        f"{label} lost frozen PASS45 progress accounting",
    )

req(
    "PASS45_ITEM16_LOCAL_UE58_EVIDENCE_CHAIN_COMPLETE" in full_chain,
    "full item-16 chain no longer retains its completion marker",
)
req(
    "No full gameplay runtime is run by this orchestrator." in full_chain,
    "full bounded item-16 chain lost its no-full-gameplay marker",
)

for needle in (
    "PASS45_LEVERACTION_DERIVED_LEVER_UE58_PILOT_COMPAT.py",
    "PASS45_LEVERACTION_DERIVED_LEVER_UE58_PILOT_PASS",
    "runtime_acceptance=0",
    "item16_checked=0",
):
    req(needle in lever_launcher, f"Lever component launcher invariant missing: {needle}")

lower_launcher = lever_launcher.lower()
for forbidden in ("git pull", "git push", "git reset", "git clean", "git checkout", "gh "):
    req(forbidden not in lower_launcher, f"Lever component launcher contains forbidden Git mutation: {forbidden}")

if errors:
    print("PASS45 COMPONENT-FIRST UE DEBUGGING: FAIL")
    for error in errors:
        print(f"[FAIL] {error}")
    raise SystemExit(1)

print("PASS45 COMPONENT-FIRST UE DEBUGGING: PASS")
print("preflight_first=1 component_only_until_pass=1 full_chain_after_component_pass=1 full_gameplay_acceptance_only=1")
print("current_local_component=LeverAction official_progress=22/36=61.1% runtime_acceptance=0 item16_checked=0 merge_permitted=0")
