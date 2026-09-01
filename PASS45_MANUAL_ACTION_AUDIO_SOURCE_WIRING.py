#!/usr/bin/env python3
from __future__ import annotations

import argparse
from pathlib import Path

ROOT = Path(__file__).resolve().parent
CPP = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCWeaponAudioComponent.cpp"

OLD_ACTION_BLOCK = '''    if (ActionType == EOCWeaponActionType::PumpAction)
    {
        if (USoundBase* Pump = LoadSound(TEXT("/Game/R13/Audio/shotguncock.shotguncock")))
        {
            RepositoryFallbackProfile->PumpCycle.Add(Pump);
        }
    }
'''

NEW_ACTION_BLOCK = '''    if (ActionType == EOCWeaponActionType::BoltAction)
    {
        // Repository-owned CC0 action-family donor. The source is Mosin-Nagant, not exact M700 identity.
        if (USoundBase* Bolt = LoadSound(TEXT("/Game/PASS45/Audio/ManualAction/SW_PASS45_BoltAction_CC0_Donor.SW_PASS45_BoltAction_CC0_Donor")))
        {
            RepositoryFallbackProfile->BoltCycle.Add(Bolt);
        }
    }
    if (ActionType == EOCWeaponActionType::PumpAction)
    {
        if (USoundBase* Pump = LoadSound(TEXT("/Game/R13/Audio/shotguncock.shotguncock")))
        {
            RepositoryFallbackProfile->PumpCycle.Add(Pump);
        }
    }
    if (ActionType == EOCWeaponActionType::LeverAction)
    {
        // Repository-owned CC0 action-family donor. This is not an exact Stein/Marlin/Model-1894 identity claim.
        if (USoundBase* Lever = LoadSound(TEXT("/Game/PASS45/Audio/ManualAction/SW_PASS45_LeverAction_CC0_Donor.SW_PASS45_LeverAction_CC0_Donor")))
        {
            RepositoryFallbackProfile->LeverCycle.Add(Lever);
        }
    }
'''

OLD_LOG_BLOCK = '''        UE_LOG(LogTemp, Display,
            TEXT("PASS45_WEAPON_AUDIO_FALLBACK_READY weapon=%s shot=1 reload=%d pump_cycle=%d exact_profile_override=0 authoritative_mutation=0 runtime_acceptance=0"),
            *WeaponId.ToString(),
            RepositoryFallbackProfile->ReloadStart.IsEmpty() ? 0 : 1,
            RepositoryFallbackProfile->PumpCycle.IsEmpty() ? 0 : 1);
'''

NEW_LOG_BLOCK = '''        UE_LOG(LogTemp, Display,
            TEXT("PASS45_WEAPON_AUDIO_FALLBACK_READY weapon=%s shot=1 reload=%d bolt_cycle=%d pump_cycle=%d lever_cycle=%d exact_profile_override=0 authoritative_mutation=0 runtime_acceptance=0"),
            *WeaponId.ToString(),
            RepositoryFallbackProfile->ReloadStart.IsEmpty() ? 0 : 1,
            RepositoryFallbackProfile->BoltCycle.IsEmpty() ? 0 : 1,
            RepositoryFallbackProfile->PumpCycle.IsEmpty() ? 0 : 1,
            RepositoryFallbackProfile->LeverCycle.IsEmpty() ? 0 : 1);
'''


def classify(text: str, old: str, new: str, label: str) -> str:
    old_count = text.count(old)
    new_count = text.count(new)
    if new_count == 1:
        # NEW_ACTION_BLOCK intentionally contains OLD_ACTION_BLOCK as its pump-action middle section.
        # In that nested case, old_count=1 is the expected fully-wired state, not an ambiguity.
        expected_old_count = 1 if old in new else 0
        if old_count == expected_old_count:
            return "wired"
    if old_count == 1 and new_count == 0:
        return "pending"
    raise SystemExit(
        f"PASS45 MANUAL ACTION AUDIO SOURCE WIRING: ambiguous {label} anchor "
        f"old_count={old_count} new_count={new_count}"
    )


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--mode", choices=("check", "write"), default="check")
    args = parser.parse_args()

    if not CPP.is_file():
        raise SystemExit(f"missing source file: {CPP.relative_to(ROOT)}")
    text = CPP.read_text(encoding="utf-8")

    action_state = classify(text, OLD_ACTION_BLOCK, NEW_ACTION_BLOCK, "action block")
    log_state = classify(text, OLD_LOG_BLOCK, NEW_LOG_BLOCK, "diagnostic block")
    if action_state != log_state:
        raise SystemExit(
            f"PASS45 MANUAL ACTION AUDIO SOURCE WIRING: partial state action={action_state} log={log_state}"
        )

    if action_state == "wired":
        print("PASS45 MANUAL ACTION AUDIO SOURCE WIRING: ALREADY WIRED")
        print("runtime_acceptance=0 item16_checked=0")
        return

    if args.mode == "check":
        raise SystemExit("PASS45 MANUAL ACTION AUDIO SOURCE WIRING: PENDING")

    updated = text.replace(OLD_ACTION_BLOCK, NEW_ACTION_BLOCK, 1).replace(OLD_LOG_BLOCK, NEW_LOG_BLOCK, 1)
    if updated == text:
        raise SystemExit("PASS45 MANUAL ACTION AUDIO SOURCE WIRING: replacement produced no diff")
    CPP.write_text(updated, encoding="utf-8", newline="\n")

    verified = CPP.read_text(encoding="utf-8")
    if classify(verified, OLD_ACTION_BLOCK, NEW_ACTION_BLOCK, "action block") != "wired":
        raise SystemExit("PASS45 MANUAL ACTION AUDIO SOURCE WIRING: action postcondition failed")
    if classify(verified, OLD_LOG_BLOCK, NEW_LOG_BLOCK, "diagnostic block") != "wired":
        raise SystemExit("PASS45 MANUAL ACTION AUDIO SOURCE WIRING: diagnostic postcondition failed")

    print("PASS45 MANUAL ACTION AUDIO SOURCE WIRING: WIRED")
    print("- BoltAction -> repository-owned CC0 bolt-family SoundWave path")
    print("- PumpAction -> existing tracked shotgun cock asset")
    print("- LeverAction -> repository-owned CC0 lever-family SoundWave path")
    print("- missing UE assets still fail visible because LoadSound returns null")
    print("runtime_acceptance=0 item16_checked=0")


if __name__ == "__main__":
    main()
