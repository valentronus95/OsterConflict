#!/usr/bin/env python3
"""Fail-closed source contract for PASS45 item-16 manual-action timing semantics.

The authoritative gameplay cycle and the authored animation play length are related
but not the same source of truth. Gameplay owns the Bolt/Pump/Lever lockout through
AOCWeaponBase::bActionCycling / ManualActionCycleSeconds. Presentation observes the
replicated transition and may keep a non-looping authored sequence attached until
the gameplay gate resets, but it must not create a second action-cycle timer.

This distinction matters immediately for Remington 870: the deterministic audited
PumpCycle source is 0.55 s while gameplay's pump lockout is 0.72 s. That is not, by
itself, a source failure and must never be 'fixed' by silently stretching donor
motion or by moving gameplay authority into presentation. Runtime feel/visual/audio
acceptance remains a separate factual UE 5.8 gate.
"""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent
errors: list[str] = []


def read(rel: str) -> str:
    path = ROOT / rel
    if not path.is_file():
        raise SystemExit(f"PASS45 ITEM16 MANUAL ACTION TIMING: FAIL\n[FAIL] missing file: {rel}")
    return path.read_text(encoding="utf-8", errors="replace")


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


def constructor_block(text: str, class_name: str, next_class_name: str | None) -> str:
    start = text.find(f"{class_name}::{class_name}()")
    if start < 0:
        errors.append(f"cannot isolate constructor block {class_name}")
        return ""
    if next_class_name is None:
        end = len(text)
    else:
        end = text.find(f"{next_class_name}::{next_class_name}()", start + 1)
        if end < 0:
            errors.append(f"cannot find next constructor after {class_name}: {next_class_name}")
            return ""
    return text[start:end]


def parse_scalar(text: str, pattern: str, label: str) -> float:
    match = re.search(pattern, text, re.MULTILINE)
    if not match:
        errors.append(f"missing {label}")
        return float("nan")
    return float(match.group(1))


variants = read("OsterConflict/Source/OsterConflict/Private/OCWeaponVariants.cpp")
profiles = read("OsterConflict/Source/OsterConflict/Private/OCWeaponAnimationProfiles.cpp")
presentation = read("OsterConflict/Source/OsterConflict/Private/OCFirstPersonWeaponPresentationSubsystem.cpp")
m700_source = read("PASS45_M700_DERIVED_BOLT_TRANSLATION_SOURCE.py")
remington_source = read("PASS45_REMINGTON870_DERIVED_PUMP_SOURCE.py")
lever_source = read("PASS45_LEVERACTION_DERIVED_LEVER_SOURCE.py")
runtime_evidence = read("VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py")

m700_block = constructor_block(variants, "AOCWeapon_Sniper", "AOCWeapon_Shotgun")
remington_block = constructor_block(variants, "AOCWeapon_Shotgun", "AOCWeapon_LMG")
lever_block = constructor_block(variants, "AOCWeapon_LeverAction", None)

for block, weapon_id, action, gate, label in (
    (m700_block, 'TEXT("OC_SNP1")', "EOCWeaponActionType::BoltAction", 1.10, "M700"),
    (remington_block, 'TEXT("OC_SG1")', "EOCWeaponActionType::PumpAction", 0.72, "Remington870"),
    (lever_block, 'TEXT("R13_LEVER4570")', "EOCWeaponActionType::LeverAction", 0.85, "LeverAction"),
):
    req(weapon_id in block, f"{label} constructor lost expected weapon id")
    req(action in block, f"{label} constructor lost expected mechanical action")
    req(
        f"T.ManualActionCycleSeconds = {gate:.2f}f;" in block,
        f"{label} gameplay manual-action gate drifted from {gate:.2f}s",
    )

m700_motion = parse_scalar(m700_source, r"^CYCLE_DURATION\s*=\s*([0-9.]+)$", "M700 source CYCLE_DURATION")
remington_motion = parse_scalar(remington_source, r"^PUMP_DURATION\s*=\s*([0-9.]+)$", "Remington source PUMP_DURATION")
lever_motion = parse_scalar(lever_source, r"^CYCLE_DURATION\s*=\s*([0-9.]+)$", "Lever source CYCLE_DURATION")

req(abs(m700_motion - 1.10) < 1e-9, f"M700 pilot motion duration drifted: {m700_motion}")
req(abs(remington_motion - 0.55) < 1e-9, f"Remington audited PumpCycle duration drifted: {remington_motion}")
req(abs(lever_motion - 0.85) < 1e-9, f"Lever pilot motion duration drifted: {lever_motion}")
req(
    abs(remington_motion - 0.72) > 1e-9,
    "Remington authored PumpCycle was silently stretched to the gameplay gate; timing sources must remain distinct unless factual evidence authorizes a source change",
)

# Presentation must observe the authoritative replicated gate and reuse its duration
# only as presentation cleanup/reset timing. It may not own a second gameplay cycle.
for needle in (
    "const bool bActionCycling = Weapon->IsActionCycling();",
    "if (bActionCycling && !State.bWasActionCycling)",
    "Weapon->GetManualActionCycleDuration()",
    "PlayWeaponAnimation(*Weapon, ManualActionSequence, State, ResetDelay)",
    "PASS45_MANUAL_ACTION_AUTHORED_SOURCE_BRIDGE_READY",
    "replicated_gate=1",
    "second_gameplay_timer=0",
    "procedural_fallback=0",
):
    req(needle in presentation, f"manual-action presentation ownership invariant missing: {needle}")

for forbidden in (
    "ActionCycleStartTime",
    "ManualActionCycleTimerHandle",
    "SetTimer(ManualAction",
    "PASS45_MANUAL_ACTION_PROCEDURAL_FALLBACK_ACTIVE",
):
    req(forbidden not in presentation, f"second/procedural manual-action owner returned: {forbidden}")

# Content boundary: Remington alone has an accepted source-wired action sequence.
# M700/Lever must stay fail-visible until factual calibration authors a production sequence.
req(
    '{ FName(TEXT("OC_SNP1")), TEXT(""), TEXT(""), true, TEXT(""), true }' in profiles,
    "M700 production manual-action slot is no longer fail-closed",
)
req(
    '/Game/Production/Weapons/Remington870/AN_Remington870_PumpCycle.AN_Remington870_PumpCycle' in profiles,
    "Remington production PumpCycle path disappeared",
)
req(
    '{ FName(TEXT("R13_LEVER4570")), TEXT(""), TEXT(""), true, TEXT(""), true }' in profiles,
    "Lever production manual-action slot is no longer fail-closed",
)
for forbidden in (
    "/Game/PASS45/ImportPilots/M700DerivedBoltTranslation",
    "AN_PASS45_M700_BoltTranslation_Pilot",
    "/Game/PASS45/ImportPilots/LeverActionDerivedLever",
    "AN_PASS45_LeverAction_Cycle_Pilot",
):
    req(forbidden not in profiles, f"calibration pilot leaked into production profile: {forbidden}")

# Strict runtime evidence must still require factual Remington gameplay activation,
# but source timing semantics must not be converted into source-only runtime acceptance.
for needle in (
    "PASS45_MANUAL_ACTION_AUTHORED_SOURCE_BRIDGE_READY",
    "weapon=OC_SG1",
    "action=EOCWeaponActionType::PumpAction",
    "/Game/Production/Weapons/Remington870/AN_Remington870_PumpCycle.AN_Remington870_PumpCycle",
    "replicated_gate=1",
    "second_gameplay_timer=0",
    "runtime_acceptance=0",
    "VISUAL_ACCEPTANCE=PENDING_MANUAL_OBSERVATION",
):
    req(needle in runtime_evidence, f"strict runtime evidence lost Remington fail-closed marker: {needle}")

for forbidden in (
    "PUMP_DURATION = 0.72",
    "sequence_duration == gate_duration",
    "sequence_seconds=0.720000",
    "runtime_acceptance=1",
    "item16_checked=1",
):
    req(forbidden not in runtime_evidence, f"runtime evidence gained invalid source-only timing/acceptance assumption: {forbidden}")

if errors:
    print("PASS45 ITEM16 MANUAL ACTION TIMING: FAIL")
    for error in errors:
        print(f"[FAIL] {error}")
    raise SystemExit(1)

print("PASS45 ITEM16 MANUAL ACTION TIMING: PASS")
print("gameplay_owner=bActionCycling presentation_owner=observer second_gameplay_timer=0")
print("m700_motion_s=1.10 m700_gate_s=1.10 production_sequence=0 calibration_pending=1")
print("remington_sequence_s=0.55 remington_gate_s=0.72 timing_sources_distinct=1 production_sequence=1")
print("lever_motion_s=0.85 lever_gate_s=0.85 production_sequence=0 calibration_pending=1")
print("runtime_acceptance=0 item16_checked=0 merge_permitted=0 user_local_execution_requested=0")
