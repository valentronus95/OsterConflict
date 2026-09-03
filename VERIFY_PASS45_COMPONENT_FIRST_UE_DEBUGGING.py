#!/usr/bin/env python3
"""Static contract for PASS45 component-first UE debugging cadence.

The protocol still defines the narrow Lever-only UE proof when local execution is
resumed, but the current user-authoritative checkpoint explicitly pauses all
user-local execution. CI therefore validates both things at once: the deferred
launcher remains safe/self-validating, and living checkpoint docs must not present
that launcher as an active instruction while user_local_execution_requested=0.
"""
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
    "OsterConflict/RUN_PASS45_ITEM16_LOCAL_UE58_EVIDENCE.cmd",
    "Do not rerun the five-phase chain until this Lever-only proof passes.",
):
    req(needle in protocol, f"component-first protocol invariant missing: {needle}")

for text, label in ((ledger, "ledger"), (history, "history")):
    req("component-first" in text.lower(), f"{label} does not record component-first cadence")
    req("22/36 = 61.1%" in text, f"{label} lost frozen PASS45 progress accounting")
    req(
        "user_local_execution_requested=0" in text,
        f"{label} lost the explicit user-local execution pause",
    )

history_lower = history.lower()
ledger_lower = ledger.lower()
req(
    "do not require or request pc-side checks" in history_lower,
    "history no longer preserves the user's explicit no-PC-check instruction",
)
req(
    "no pc-side checking" in ledger_lower
    or "do not require/request pc-side checks" in ledger_lower
    or "do not require or request pc-side checks" in ledger_lower,
    "ledger no longer preserves the user's explicit no-PC-check instruction",
)
req(
    "user-local execution is currently paused" in history_lower,
    "history no longer marks local execution as paused",
)
req(
    "user-local execution itself is now paused" in ledger_lower
    or "user local checks paused" in ledger_lower,
    "ledger no longer marks local execution as paused",
)
req(
    "do **not** run or request `start_here.cmd -> 2. повний runtime-тест` now" in ledger_lower,
    "ledger no longer blocks premature full gameplay requests while local execution is paused",
)

# The bounded chain remains canonical infrastructure even while it is not an
# active user instruction. Preserve its fail-closed shape for a future factual
# runtime pass without forcing living checkpoint docs to tell the user to run it.
req(
    "PASS45_ITEM16_LOCAL_UE58_EVIDENCE_CHAIN_COMPLETE" in full_chain,
    "full item-16 chain no longer retains its completion marker",
)
req(
    "No full gameplay runtime is run by this orchestrator." in full_chain,
    "full bounded item-16 chain lost its no-full-gameplay marker",
)

# The deferred Lever-only launcher must remain self-validating. A zero commandlet
# exit is not sufficient because a technical repair can pass before a later base
# pilot assertion rejects the same component.
for needle in (
    "PASS45_LEVERACTION_DERIVED_LEVER_UE58_PILOT_COMPAT.py",
    "PASS45_LEVERACTION_UE58_RESAMPLE_GRID_READY initial_fps=30 compat_fps=60 compat_frames=52 source_frames=26 motion_end_frame=51 tail_pad_frames=1",
    "PASS45_LEVERACTION_UE58_ASSET_COMPILATION_BARRIER_END stage=after_set_bone_track_keys_before_sampling",
    "PASS45_LEVERACTION_UE58_SEQUENCE_ENVELOPE_CONTRACT_ARMED motion_duration=0.850000 sequence_envelope=0.866667 tail_pad_frames=1",
    "PASS45_LEVERACTION_UE58_MOTION_DURATION_RESTORED motion_duration=0.850000 sequence_envelope=0.866667 tail_pad_frames=1",
    "PASS45_LEVERACTION_DERIVED_LEVER_UE58_PILOT_PASS",
    "PASS45_LEVERACTION_UE58_ASSET_COMPILATION_BARRIER_END stage=post_pilot_before_commandlet_exit",
    "source_authored_endpoint=0",
    "pilot_angle_accepted=0",
    "saved_packages=0",
    "production_profile_changed=0",
    "production_cutover=0",
    "runtime_visual_acceptance=0",
    "runtime_acceptance=0",
    "item16_checked=0",
):
    req(needle in lever_launcher, f"Lever component launcher exact proof invariant missing: {needle}")

req(
    lever_launcher.count('findstr /L /C:') >= 14,
    "Lever component launcher no longer performs the expected exact-marker postflight",
)
req(
    "pilot_sequence_duration_mismatch" not in lever_launcher,
    "Lever launcher contains a stale hard-coded duration-mismatch acceptance path",
)

lower_launcher = lever_launcher.lower()
for forbidden in ("git pull", "git push", "git reset", "git clean", "git checkout", "gh "):
    req(forbidden not in lower_launcher, f"Lever component launcher contains forbidden Git mutation: {forbidden}")

if errors:
    print("PASS45 COMPONENT-FIRST UE DEBUGGING: FAIL")
    for error in errors:
        print(f"[FAIL] {error}")
    raise SystemExit(1)

print("PASS45 COMPONENT-FIRST UE DEBUGGING: PASS")
print("preflight_first=1 component_only_until_pass=1 deferred_lever_postflight_safe=1 full_gameplay_acceptance_only=1")
print("user_local_execution_requested=0 deferred_local_component=LeverAction exact_postflight_checks=14 official_progress=22/36=61.1% runtime_acceptance=0 item16_checked=0 merge_permitted=0")
