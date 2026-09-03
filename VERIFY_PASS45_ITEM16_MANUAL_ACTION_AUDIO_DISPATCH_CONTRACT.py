#!/usr/bin/env python3
"""Fail-closed source contract for PASS45 item-16 manual-action audio dispatch.

Runtime evidence already proves that the required bolt/pump/lever SoundBase loaded into
the repository fallback profile. This contract closes the other half of that evidence:
the authoritative manual-action transition must actually dispatch ManualActionCycle,
and a non-empty resolved set must flow to Play2D/PlayAt without a second gameplay
timer. Direct audible/feel acceptance remains a separate UE 5.8 manual gate.
"""
from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parent
PRESENTATION = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCFirstPersonWeaponPresentationSubsystem.cpp"
AUDIO = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCWeaponAudioComponent.cpp"
RUNTIME = ROOT / "VERIFY_PASS45_MANUAL_ACTION_RUNTIME.py"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS45 ITEM16 MANUAL ACTION AUDIO DISPATCH: FAIL\n[FAIL] missing file: {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


def isolate(text: str, start_marker: str, end_marker: str, label: str, errors: list[str]) -> str:
    start = text.find(start_marker)
    end = text.find(end_marker, start + len(start_marker)) if start >= 0 else -1
    if start < 0 or end < 0 or end <= start:
        errors.append(f"cannot isolate {label}")
        return ""
    return text[start:end]


def validate(presentation: str, audio: str, runtime: str) -> list[str]:
    errors: list[str] = []

    action_block = isolate(
        presentation,
        "const bool bActionCycling = Weapon->IsActionCycling();",
        "State.bWasActionCycling = bActionCycling;",
        "manual-action presentation transition",
        errors,
    )
    audio_block = isolate(
        audio,
        "void UOCWeaponAudioComponent::HandleStateEventLocal(",
        "void UOCWeaponAudioComponent::HandleImpactLocal(",
        "weapon audio state-event handler",
        errors,
    )

    dispatch = "Audio->HandleStateEventLocal(EOCWeaponAudioEvent::ManualActionCycle, Weapon->GetActorLocation(), EventSeed);"
    if action_block:
        if action_block.count(dispatch) != 1:
            errors.append("manual-action transition must dispatch exactly one ManualActionCycle audio event")
        if "if (UOCWeaponAudioComponent* Audio = Weapon->GetWeaponAudioComponent())" not in action_block:
            errors.append("manual-action transition lost weapon-audio component lookup")
        dispatch_pos = action_block.find(dispatch)
        profile_pos = action_block.find("OCResolveWeaponAnimationProfile(WeaponId)")
        if dispatch_pos < 0 or profile_pos < 0 or dispatch_pos > profile_pos:
            errors.append("manual-action audio dispatch no longer occurs before authored animation presentation")
        for forbidden in ("SetTimer(", "ManualActionCycleTimerHandle", "ActionCycleStartTime"):
            if forbidden in action_block:
                errors.append(f"manual-action presentation regained a second gameplay timing owner: {forbidden}")

    for marker in (
        "case EOCWeaponAudioEvent::ManualActionCycle:",
        "case EOCWeaponActionType::BoltAction: return &Profile->BoltCycle;",
        "case EOCWeaponActionType::PumpAction: return &Profile->PumpCycle;",
        "case EOCWeaponActionType::LeverAction: return &Profile->LeverCycle;",
        "StateProfile = EnsureRepositoryFallbackProfile();",
        "Set = ResolveSet(StateProfile);",
        "if (!Set || Set->IsEmpty())",
        "PASS45_WEAPON_AUDIO_CONTENT_GAP weapon=%s event=manual_action",
        "USoundBase* Sound = Pick(*Set, EventSeed);",
        "Play2D(Sound, StateProfile->LocalMechanicalVolume);",
        "PlayAt(Sound, SourceLocation, 1.0f);",
    ):
        if marker not in audio_block:
            errors.append(f"manual-action audio success/failure route missing: {marker}")

    # Repository fallback arrays may only receive successfully loaded SoundBase objects.
    for label, marker in (
        ("Bolt", "if (USoundBase* Bolt = LoadSound"),
        ("Pump", "if (USoundBase* Pump = LoadSound"),
        ("Lever", "if (USoundBase* Lever = LoadSound"),
    ):
        if marker not in audio:
            errors.append(f"{label} fallback route no longer guards Add() behind successful LoadSound")
    for marker in (
        "RepositoryFallbackProfile->BoltCycle.Add(Bolt);",
        "RepositoryFallbackProfile->PumpCycle.Add(Pump);",
        "RepositoryFallbackProfile->LeverCycle.Add(Lever);",
        "PASS45_WEAPON_AUDIO_FALLBACK_READY",
    ):
        if marker not in audio:
            errors.append(f"manual-action loaded-audio evidence route missing: {marker}")

    # Runtime gate must require both the loaded action-family slot and reject the event content-gap marker.
    for marker in (
        '"audio_field": "bolt_cycle=1"',
        '"audio_field": "pump_cycle=1"',
        '"audio_field": "lever_cycle=1"',
        '"PASS45_WEAPON_AUDIO_FALLBACK_READY"',
        '("PASS45_WEAPON_AUDIO_CONTENT_GAP", "event=manual_action")',
    ):
        if marker not in runtime:
            errors.append(f"runtime manual-action audio evidence gate missing: {marker}")

    return errors


def main() -> int:
    presentation = read(PRESENTATION)
    audio = read(AUDIO)
    runtime = read(RUNTIME)
    errors = validate(presentation, audio, runtime)

    # Adversarial source mutations prove the guard is not a decorative grep collection.
    negative_cases = (
        (
            "missing dispatch",
            presentation.replace(
                "Audio->HandleStateEventLocal(EOCWeaponAudioEvent::ManualActionCycle, Weapon->GetActorLocation(), EventSeed);",
                "// removed dispatch",
                1,
            ),
            audio,
            runtime,
            "dispatch exactly one",
        ),
        (
            "missing success playback",
            presentation,
            audio.replace("Play2D(Sound, StateProfile->LocalMechanicalVolume);", "// removed Play2D", 1),
            runtime,
            "Play2D",
        ),
        (
            "missing runtime content-gap rejection",
            presentation,
            audio,
            runtime.replace('(\"PASS45_WEAPON_AUDIO_CONTENT_GAP\", \"event=manual_action\")', '(\"REMOVED\",)', 1),
            "CONTENT_GAP",
        ),
    )
    for label, p_text, a_text, r_text, expected in negative_cases:
        mutated = validate(p_text, a_text, r_text)
        if not mutated or not any(expected in error for error in mutated):
            errors.append(f"negative case {label!r} was not rejected for expected reason containing {expected!r}: {mutated}")

    if errors:
        print("PASS45 ITEM16 MANUAL ACTION AUDIO DISPATCH: FAIL")
        for error in errors:
            print(f"[FAIL] {error}")
        raise SystemExit(1)

    print("PASS45 ITEM16 MANUAL ACTION AUDIO DISPATCH: PASS")
    print("manual_action_dispatch=1 loaded_sound_guard=1 play2d_or_playat=1 content_gap_fail_closed=1")
    print("audio_runtime_inference=loaded_slot_plus_mandatory_dispatch direct_audible_acceptance=pending")
    print("second_gameplay_timer=0 runtime_acceptance=0 item16_checked=0 merge_permitted=0 user_local_execution_requested=0")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
