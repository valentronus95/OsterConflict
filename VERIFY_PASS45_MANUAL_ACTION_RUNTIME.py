#!/usr/bin/env python3
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent
DEFAULT_GAMEPLAY_LOG = ROOT / "Logs" / "R14_CURRENT_GAMEPLAY.log"
DEFAULT_PROFILES = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCWeaponAnimationProfiles.cpp"

EXPECTED = (
    {
        "weapon": "OC_SNP1",
        "action": "EOCWeaponActionType::BoltAction",
        "duration": "duration=1.100",
        "audio_field": "bolt_cycle=1",
        "production_prefix": "/Game/Production/Weapons/M700/",
    },
    {
        "weapon": "OC_SG1",
        "action": "EOCWeaponActionType::PumpAction",
        "duration": "duration=0.720",
        "audio_field": "pump_cycle=1",
        "production_prefix": "/Game/Production/Weapons/Remington870/",
    },
    {
        "weapon": "R13_LEVER4570",
        "action": "EOCWeaponActionType::LeverAction",
        "duration": "duration=0.850",
        "audio_field": "lever_cycle=1",
        "production_prefix": "/Game/Production/Weapons/LeverAction/",
    },
)


def lines_with(text: str, *needles: str) -> list[str]:
    return [line for line in text.splitlines() if all(needle in line for needle in needles)]


def profile_manual_path(text: str, weapon_id: str) -> str | None:
    pattern = re.compile(
        r"\{\s*FName\(TEXT\(\"" + re.escape(weapon_id) + r"\"\)\)\s*,"
        r"\s*TEXT\(\"[^\"]*\"\)\s*,\s*TEXT\(\"[^\"]*\"\)\s*,\s*true\s*,"
        r"\s*TEXT\(\"([^\"]*)\"\)\s*,\s*true\s*\}",
        re.MULTILINE,
    )
    match = pattern.search(text)
    return match.group(1) if match else None


def main() -> int:
    log_path = Path(sys.argv[1]) if len(sys.argv) > 1 else DEFAULT_GAMEPLAY_LOG
    profiles_path = Path(sys.argv[2]) if len(sys.argv) > 2 else DEFAULT_PROFILES
    if not log_path.is_file():
        print(f"PASS45 MANUAL ACTION RUNTIME: FAIL: missing gameplay log: {log_path}")
        return 1
    if not profiles_path.is_file():
        print(f"PASS45 MANUAL ACTION RUNTIME: FAIL: missing animation profiles: {profiles_path}")
        return 1

    gameplay = log_path.read_text(encoding="utf-8", errors="replace")
    profiles = profiles_path.read_text(encoding="utf-8", errors="replace")
    errors: list[str] = []

    for expected in EXPECTED:
        weapon = expected["weapon"]
        action = expected["action"]
        production_prefix = expected["production_prefix"]
        profile_path = profile_manual_path(profiles, weapon)

        if profile_path is None:
            errors.append(f"cannot resolve required manual-action profile for {weapon}")
            profile_path_ok = False
        elif not profile_path:
            errors.append(f"production manual-action profile path missing for {weapon}")
            profile_path_ok = False
        elif not profile_path.startswith(production_prefix):
            errors.append(
                f"manual-action profile escaped production namespace for {weapon}: "
                f"expected_prefix={production_prefix} actual={profile_path}"
            )
            profile_path_ok = False
        elif "/ImportPilots/" in profile_path or "Pilot" in profile_path:
            errors.append(f"calibration pilot leaked into runtime profile for {weapon}: {profile_path}")
            profile_path_ok = False
        else:
            profile_path_ok = True

        cycle_lines = lines_with(
            gameplay,
            "PASS45_MANUAL_ACTION_CYCLE_READY",
            f"weapon={weapon}",
            f"action={action}",
            expected["duration"],
            "authoritative=1",
        )
        if not cycle_lines:
            errors.append(f"missing authoritative cycle evidence for {weapon} {action}")

        audio_lines = lines_with(
            gameplay,
            "PASS45_WEAPON_AUDIO_FALLBACK_READY",
            f"weapon={weapon}",
            expected["audio_field"],
        )
        if not audio_lines:
            errors.append(f"missing loaded mechanical audio evidence for {weapon}: {expected['audio_field']}")

        if profile_path_ok:
            authored_lines = lines_with(
                gameplay,
                "PASS45_MANUAL_ACTION_AUTHORED_SOURCE_BRIDGE_READY",
                f"weapon={weapon}",
                f"action={action}",
                f"path={profile_path}",
                "replicated_gate=1",
                "second_gameplay_timer=0",
                "runtime_acceptance=0",
            )
            if not authored_lines:
                errors.append(
                    f"missing exact production authored moving-part animation evidence for {weapon} {action}: "
                    f"path={profile_path}"
                )

        forbidden = (
            ("PASS45_WEAPON_AUDIO_CONTENT_GAP", "event=manual_action"),
            ("PASS45_MANUAL_ACTION_AUTHORED_CONTENT_GAP",),
            ("PASS45_MANUAL_ACTION_AUTHORED_SOURCE_BRIDGE_FAIL",),
        )
        for markers in forbidden:
            if lines_with(gameplay, *markers, f"weapon={weapon}"):
                errors.append(f"runtime content gap/failure present for {weapon}: {' + '.join(markers)}")

    if errors:
        print("PASS45 MANUAL ACTION RUNTIME: FAIL")
        for error in errors:
            print("[FAIL]", error)
        print("STATUS: item16 remains OPEN; runtime acceptance=0")
        return 1

    print("PASS45 MANUAL ACTION RUNTIME: PASS")
    print("- M700 bolt, Remington 870 pump and Lever Action cycles were each factually exercised")
    print("- authoritative cycle timing, loaded mechanical audio and exact production-profile authored animation evidence are present")
    print("- calibration-pilot paths and manual-action audio/content bridge failures are rejected for all three required weapons")
    print("STATUS: AUTOMATED ITEM16 RUNTIME EVIDENCE PASS; direct visual/audio feel acceptance still required")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
