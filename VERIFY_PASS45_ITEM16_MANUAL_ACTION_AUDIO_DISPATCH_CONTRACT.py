#!/usr/bin/env python3
"""Fail-closed source contract for PASS45 item-16 manual-action audio dispatch.

Runtime evidence must prove more than a loaded bolt/pump/lever SoundBase. The
manual-action transition must dispatch ManualActionCycle, the exact repository-owned
manual-action sound expected for that weapon must flow through local playback with
positive effective volume, and the runtime log must preserve that factual dispatch.
Direct audible/feel acceptance remains a separate UE 5.8 manual gate.
"""
from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parent
PRESENTATION = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCFirstPersonWeaponPresentationSubsystem.cpp"
AUDIO = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCWeaponAudioComponent.cpp"
RUNTIME = ROOT / "VERIFY_PASS45_MANUAL_ACTION_RUNTIME.py"

EXPECTED_AUDIO = (
    (
        "M700 bolt",
        "/Game/PASS45/Audio/ManualAction/SW_PASS45_BoltAction_CC0_Donor.SW_PASS45_BoltAction_CC0_Donor",
    ),
    (
        "Remington 870 pump",
        "/Game/R13/Audio/shotguncock.shotguncock",
    ),
    (
        "Lever Action",
        "/Game/PASS45/Audio/ManualAction/SW_PASS45_LeverAction_CC0_Donor.SW_PASS45_LeverAction_CC0_Donor",
    ),
)


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
        "const bool bLocalPlayback = IsLocalWeaponOwner() && Event != EOCWeaponAudioEvent::Drop;",
        "const float PlaybackVolume = bLocalPlayback ? StateProfile->LocalMechanicalVolume : 1.0f;",
        "const float WeaponBusVolume = UOCAudioUserSettings::Get()->GetBusVolume(EOCAudioBus::Weapons);",
        "const bool bPlaybackDispatchable = Sound && PlaybackVolume > 0.0f && WeaponBusVolume > 0.0f;",
        "Play2D(Sound, PlaybackVolume);",
        "PlayAt(Sound, SourceLocation, PlaybackVolume);",
        "if (Event == EOCWeaponAudioEvent::ManualActionCycle)",
        "PASS45_MANUAL_ACTION_AUDIO_PLAYBACK_DISPATCHED",
        "sound=%s route=%s bus_gt_zero=1 effective_volume_gt_zero=1 second_gameplay_timer=0 runtime_acceptance=0",
        "PASS45_MANUAL_ACTION_AUDIO_PLAYBACK_FAIL",
    ):
        if marker not in audio_block:
            errors.append(f"manual-action audio success/failure route missing: {marker}")

    playback_marker_pos = audio_block.find("PASS45_MANUAL_ACTION_AUDIO_PLAYBACK_DISPATCHED")
    play2d_pos = audio_block.find("Play2D(Sound, PlaybackVolume);")
    playat_pos = audio_block.find("PlayAt(Sound, SourceLocation, PlaybackVolume);")
    if playback_marker_pos < 0 or play2d_pos < 0 or playat_pos < 0 or playback_marker_pos < max(play2d_pos, playat_pos):
        errors.append("manual-action playback evidence must be emitted only after the Play2D/PlayAt dispatch branches")

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

    # Bind runtime expectation to the exact sound objects actually loaded by source.
    for label, object_path in EXPECTED_AUDIO:
        if f'LoadSound(TEXT("{object_path}"))' not in audio:
            errors.append(f"{label} exact fallback sound object drifted from source: {object_path}")
        if f'"audio_object_path": "{object_path}"' not in runtime:
            errors.append(f"{label} exact runtime sound identity gate missing: {object_path}")

    # Runtime gate must require loaded action-family audio, exact playback identity,
    # positive bus/effective volume and failure-marker rejection for every required weapon.
    for marker in (
        '"audio_field": "bolt_cycle=1"',
        '"audio_field": "pump_cycle=1"',
        '"audio_field": "lever_cycle=1"',
        '"PASS45_WEAPON_AUDIO_FALLBACK_READY"',
        '"PASS45_MANUAL_ACTION_AUDIO_PLAYBACK_DISPATCHED"',
        'f"sound={audio_object_path}"',
        '"route=local2d"',
        '"bus_gt_zero=1"',
        '"effective_volume_gt_zero=1"',
        '("PASS45_WEAPON_AUDIO_CONTENT_GAP", "event=manual_action")',
        '("PASS45_MANUAL_ACTION_AUDIO_PLAYBACK_FAIL",)',
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
            "missing local success playback",
            presentation,
            audio.replace("Play2D(Sound, PlaybackVolume);", "// removed Play2D", 1),
            runtime,
            "Play2D",
        ),
        (
            "M700 source sound identity drift",
            presentation,
            audio.replace(
                "/Game/PASS45/Audio/ManualAction/SW_PASS45_BoltAction_CC0_Donor.SW_PASS45_BoltAction_CC0_Donor",
                "/Game/R13/Audio/shotguncock.shotguncock",
                1,
            ),
            runtime,
            "M700 bolt exact fallback sound object drifted",
        ),
        (
            "missing runtime playback evidence",
            presentation,
            audio,
            runtime.replace('"PASS45_MANUAL_ACTION_AUDIO_PLAYBACK_DISPATCHED"', '"REMOVED_PLAYBACK_MARKER"', 1),
            "PLAYBACK_DISPATCHED",
        ),
        (
            "missing exact runtime sound identity",
            presentation,
            audio,
            runtime.replace(
                '"audio_object_path": "/Game/R13/Audio/shotguncock.shotguncock"',
                '"audio_object_path": "/Game/R13/Audio/gunreload1.gunreload1"',
                1,
            ),
            "Remington 870 pump exact runtime sound identity gate missing",
        ),
        (
            "missing positive runtime volume requirement",
            presentation,
            audio,
            runtime.replace('"effective_volume_gt_zero=1"', '"REMOVED_EFFECTIVE_VOLUME"', 1),
            "effective_volume_gt_zero=1",
        ),
        (
            "missing runtime playback failure rejection",
            presentation,
            audio,
            runtime.replace('(\"PASS45_MANUAL_ACTION_AUDIO_PLAYBACK_FAIL\",)', '(\"REMOVED\",)', 1),
            "PLAYBACK_FAIL",
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
    print("manual_action_dispatch=1 loaded_sound_guard=1 exact_sound_identity=1 local_playback_dispatch=1 positive_bus_and_volume=1 content_gap_fail_closed=1")
    print("runtime_playback_marker_required=1 wrong_sound_identity_rejected=1 direct_audible_acceptance=pending")
    print("second_gameplay_timer=0 runtime_acceptance=0 item16_checked=0 merge_permitted=0 user_local_execution_requested=0")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
