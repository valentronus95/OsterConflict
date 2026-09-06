#!/usr/bin/env python3
"""Executable source contract for PASS45 item-16 strict manual-action runtime evidence.

The strict runtime route must bind each READY line to the exact production animation
path currently declared by the gameplay profile. A generic READY marker is not enough:
otherwise a stale calibration pilot or unrelated weapon line could satisfy the gate.
The synthetic acceptance log must also carry the exact expected mechanical-audio
playback object for each weapon, not merely some /Game/ sound.
"""
from __future__ import annotations

import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent
RUNTIME = ROOT / "VERIFY_PASS45_MANUAL_ACTION_RUNTIME.py"
MAIN = ROOT / "RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd"

PROFILES_GOOD = r'''
const FOCWeaponAnimationProfile Profiles[] =
{
    { FName(TEXT("OC_SNP1")), TEXT(""), TEXT(""), true, TEXT("/Game/Production/Weapons/M700/AN_M700_BoltCycle.AN_M700_BoltCycle"), true },
    { FName(TEXT("OC_SG1")), TEXT(""), TEXT(""), true, TEXT("/Game/Production/Weapons/Remington870/AN_Remington870_PumpCycle.AN_Remington870_PumpCycle"), true },
    { FName(TEXT("R13_LEVER4570")), TEXT(""), TEXT(""), true, TEXT("/Game/Production/Weapons/LeverAction/AN_LeverAction_Cycle.AN_LeverAction_Cycle"), true },
};
'''.strip()

READY = (
    (
        "OC_SNP1",
        "EOCWeaponActionType::BoltAction",
        "duration=1.100",
        "bolt_cycle=1",
        "/Game/PASS45/Audio/ManualAction/SW_PASS45_BoltAction_CC0_Donor.SW_PASS45_BoltAction_CC0_Donor",
        "/Game/Production/Weapons/M700/AN_M700_BoltCycle.AN_M700_BoltCycle",
    ),
    (
        "OC_SG1",
        "EOCWeaponActionType::PumpAction",
        "duration=0.720",
        "pump_cycle=1",
        "/Game/R13/Audio/shotguncock.shotguncock",
        "/Game/Production/Weapons/Remington870/AN_Remington870_PumpCycle.AN_Remington870_PumpCycle",
    ),
    (
        "R13_LEVER4570",
        "EOCWeaponActionType::LeverAction",
        "duration=0.850",
        "lever_cycle=1",
        "/Game/PASS45/Audio/ManualAction/SW_PASS45_LeverAction_CC0_Donor.SW_PASS45_LeverAction_CC0_Donor",
        "/Game/Production/Weapons/LeverAction/AN_LeverAction_Cycle.AN_LeverAction_Cycle",
    ),
)


def build_log() -> str:
    lines: list[str] = []
    for weapon, action, duration, audio_field, audio_object_path, animation_path in READY:
        lines.append(
            f"PASS45_MANUAL_ACTION_CYCLE_READY weapon={weapon} action={action} {duration} authoritative=1"
        )
        lines.append(f"PASS45_WEAPON_AUDIO_FALLBACK_READY weapon={weapon} {audio_field}")
        lines.append(
            "PASS45_MANUAL_ACTION_AUDIO_PLAYBACK_DISPATCHED "
            f"weapon={weapon} action={action} sound={audio_object_path} "
            "route=local2d bus_gt_zero=1 effective_volume_gt_zero=1 "
            "second_gameplay_timer=0 runtime_acceptance=0"
        )
        lines.append(
            "PASS45_MANUAL_ACTION_AUTHORED_SOURCE_BRIDGE_READY "
            f"weapon={weapon} action={action} path={animation_path} "
            "replicated_gate=1 second_gameplay_timer=0 runtime_acceptance=0"
        )
    return "\n".join(lines) + "\n"


def run_case(log_text: str, profiles_text: str, *, expect_success: bool, label: str) -> list[str]:
    errors: list[str] = []
    with tempfile.TemporaryDirectory(prefix="pass45_item16_runtime_") as temp_dir:
        temp = Path(temp_dir)
        log_path = temp / "gameplay.log"
        profiles_path = temp / "profiles.cpp"
        log_path.write_text(log_text, encoding="utf-8")
        profiles_path.write_text(profiles_text, encoding="utf-8")
        result = subprocess.run(
            [sys.executable, str(RUNTIME), str(log_path), str(profiles_path)],
            cwd=ROOT,
            text=True,
            capture_output=True,
            check=False,
        )
        succeeded = result.returncode == 0
        if succeeded != expect_success:
            errors.append(
                f"{label}: expected_success={int(expect_success)} actual_rc={result.returncode}\n"
                f"stdout={result.stdout}\nstderr={result.stderr}"
            )
    return errors


def main() -> int:
    errors: list[str] = []
    if not RUNTIME.is_file():
        errors.append("missing VERIFY_PASS45_MANUAL_ACTION_RUNTIME.py")
    if not MAIN.is_file():
        errors.append("missing RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd")
    if errors:
        for error in errors:
            print("[FAIL]", error)
        return 1

    main_text = MAIN.read_text(encoding="utf-8", errors="replace")
    for marker in (
        'set "MANUAL_ACTION_VERIFY=%~dp0VERIFY_PASS45_MANUAL_ACTION_RUNTIME.py"',
        '%PY_CMD% "%MANUAL_ACTION_VERIFY%" "%GAMEPLAY_LOG%"',
        "Pass45 item 16 manual-action evidence is incomplete",
    ):
        if marker not in main_text:
            errors.append(f"strict main wrapper lost manual-action runtime gate: {marker}")

    good_log = build_log()
    errors.extend(run_case(good_log, PROFILES_GOOD, expect_success=True, label="exact production paths plus exact audio playback"))

    missing_playback_log = good_log.replace(
        "PASS45_MANUAL_ACTION_AUDIO_PLAYBACK_DISPATCHED weapon=OC_SNP1",
        "PASS45_AUDIO_PLAYBACK_MARKER_REMOVED weapon=OC_SNP1",
        1,
    )
    errors.extend(run_case(
        missing_playback_log,
        PROFILES_GOOD,
        expect_success=False,
        label="missing M700 local playback dispatch",
    ))

    wrong_audio_log = good_log.replace(
        "/Game/PASS45/Audio/ManualAction/SW_PASS45_BoltAction_CC0_Donor.SW_PASS45_BoltAction_CC0_Donor",
        "/Game/R13/Audio/shotguncock.shotguncock",
        1,
    )
    errors.extend(run_case(
        wrong_audio_log,
        PROFILES_GOOD,
        expect_success=False,
        label="wrong M700 manual-action sound identity",
    ))

    stale_pilot_log = good_log.replace(
        "/Game/Production/Weapons/M700/AN_M700_BoltCycle.AN_M700_BoltCycle",
        "/Game/PASS45/ImportPilots/M700DerivedBoltTranslation/AN_PASS45_M700_BoltTranslation_Pilot.AN_PASS45_M700_BoltTranslation_Pilot",
        1,
    )
    errors.extend(run_case(stale_pilot_log, PROFILES_GOOD, expect_success=False, label="stale pilot READY path"))

    empty_m700_profile = PROFILES_GOOD.replace(
        'TEXT("/Game/Production/Weapons/M700/AN_M700_BoltCycle.AN_M700_BoltCycle")',
        'TEXT("")',
        1,
    )
    errors.extend(run_case(good_log, empty_m700_profile, expect_success=False, label="empty M700 production profile"))

    wrong_namespace_profile = PROFILES_GOOD.replace(
        "/Game/Production/Weapons/LeverAction/AN_LeverAction_Cycle.AN_LeverAction_Cycle",
        "/Game/PASS45/ImportPilots/LeverActionDerivedLever/AN_PASS45_LeverAction_Cycle_Pilot.AN_PASS45_LeverAction_Cycle_Pilot",
        1,
    )
    errors.extend(run_case(good_log, wrong_namespace_profile, expect_success=False, label="Lever pilot profile leak"))

    if errors:
        print("PASS45 ITEM16 MANUAL ACTION RUNTIME CONTRACT: FAIL")
        for error in errors:
            print("[FAIL]", error)
        raise SystemExit(1)

    print("PASS45 ITEM16 MANUAL ACTION RUNTIME CONTRACT: PASS")
    print("exact_profile_path_binding=1 exact_audio_object_binding=1 local_audio_playback_dispatch_required=1 wrong_audio_rejected=1 stale_pilot_ready_rejected=1 empty_profile_rejected=1 pilot_profile_rejected=1")
    print("runtime_acceptance=0 item16_checked=0 merge_permitted=0 user_local_execution_requested=0")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
