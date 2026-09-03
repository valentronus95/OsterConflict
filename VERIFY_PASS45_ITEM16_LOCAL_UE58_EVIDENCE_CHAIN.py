#!/usr/bin/env python3
"""Fail-closed static contract for the one-shot PASS45 item-16 local UE 5.8 evidence chain."""
from __future__ import annotations

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent


def read(rel: str) -> str:
    path = ROOT / rel
    if not path.is_file():
        raise SystemExit(f"PASS45 ITEM16 LOCAL UE58 EVIDENCE CHAIN: FAIL\n[FAIL] missing file: {rel}")
    return path.read_text(encoding="utf-8", errors="replace")


errors: list[str] = []


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


chain = read("OsterConflict/RUN_PASS45_ITEM16_LOCAL_UE58_EVIDENCE.cmd")
m700_launcher = read("OsterConflict/TRY_PASS45_M700_DERIVED_BOLT_TRANSLATION_UE58_PILOT.cmd")
remington_launcher = read("OsterConflict/TRY_PASS45_REMINGTON870_DERIVED_PUMP_UE58_ASSEMBLY_AUDIT.cmd")
lever_launcher = read("OsterConflict/TRY_PASS45_LEVERACTION_DERIVED_LEVER_UE58_PILOT.cmd")
audio_launcher = read("OsterConflict/PASS45_IMPORT_MANUAL_ACTION_AUDIO_UE58.cmd")
audio_import = read("PASS45_MANUAL_ACTION_AUDIO_UE_IMPORT.py")
audio_fresh = read("PASS45_MANUAL_ACTION_AUDIO_UE_FRESH_LOAD.py")
profiles = read("OsterConflict/Source/OsterConflict/Private/OCWeaponAnimationProfiles.cpp")
audio_component = read("OsterConflict/Source/OsterConflict/Private/OCWeaponAudioComponent.cpp")

launchers = {
    "M700": "TRY_PASS45_M700_DERIVED_BOLT_TRANSLATION_UE58_PILOT.cmd",
    "Remington870": "TRY_PASS45_REMINGTON870_DERIVED_PUMP_UE58_ASSEMBLY_AUDIT.cmd",
    "LeverAction": "TRY_PASS45_LEVERACTION_DERIVED_LEVER_UE58_PILOT.cmd",
    "Audio": "PASS45_IMPORT_MANUAL_ACTION_AUDIO_UE58.cmd",
}
for label, name in launchers.items():
    req(chain.count(name) == 1, f"{label} launcher must appear exactly once in orchestrator")

for var, label in (
    ("%M700%", "M700"),
    ("%REMINGTON%", "Remington870"),
    ("%LEVER%", "LeverAction"),
    ("%AUDIO%", "Audio"),
):
    req(f'call "{var}"' in chain, f"{label} phase is not invoked with CALL")
    req(
        chain.find(f'call "{var}"') < chain.find('PASS45_ITEM16_LOCAL_UE58_EVIDENCE_CHAIN_COMPLETE'),
        f"{label} phase is not before completion marker",
    )

req(chain.count('set "RC=!ERRORLEVEL!"') == 4, "orchestrator must capture four phase return codes")
for code in (91, 92, 93, 94):
    req(f"exit /b {code}" in chain, f"orchestrator missing fail-closed exit code {code}")
req("exit /b 90" in chain, "orchestrator must fail if any launcher is missing")

for needle in (
    "PASS45_ITEM16_LOCAL_UE58_EVIDENCE_CHAIN_COMPLETE",
    "STATUS: EVIDENCE CHAIN COMPLETE, ITEM 16 STILL OPEN.",
    "Motion phases are isolated proof-only and do not save production packages.",
    "Audio phase saves only the two repository-owned Bolt/Lever donor SoundWave assets under /Game/PASS45/Audio/ManualAction.",
    "This is the ONLY save-bearing phase in this orchestrator.",
    "runtime_visual_acceptance=0",
    "runtime_acceptance=0",
    "item16_checked=0",
    "merge_permitted=0",
):
    req(needle in chain, f"orchestrator safety/status marker missing: {needle}")

lower_chain = chain.lower()
for forbidden in (
    "git.exe",
    "github",
    "unrealeditor-cmd",
    "powershell",
    "pwsh",
    "robocopy",
    "xcopy",
    "rmdir",
    "rd /",
    "erase ",
    "del /",
):
    req(forbidden not in lower_chain, f"orchestrator contains forbidden direct mutation/execution token: {forbidden}")
for line in chain.splitlines():
    command = line.strip().lower()
    req(
        re.match(r"^(?:call\s+)?(?:git|gh)(?:\.exe)?(?:\s|$)", command) is None,
        f"orchestrator directly invokes Git/GitHub CLI: {line.strip()}",
    )
for command_word in ("checkout", "reset", "clean", "pull", "push", "merge"):
    req(
        re.search(rf"(^|[&|])\s*{command_word}\s+", lower_chain, re.MULTILINE) is None,
        f"orchestrator contains forbidden direct mutation command: {command_word}",
    )
req("No Git commands are run by this orchestrator." in chain, "orchestrator must explicitly state Git is not invoked")

motion_launchers = {
    "M700": m700_launcher,
    "Remington870": remington_launcher,
    "LeverAction": lever_launcher,
}
for label, text in motion_launchers.items():
    lower = text.lower()
    req("unrealeditor-cmd.exe" in lower, f"{label} launcher no longer invokes UE commandlet")
    req("-run=pythonscript" in lower, f"{label} launcher no longer invokes the approved UE Python pilot")
    req("-unattended -nop4 -nosplash -nullrhi" in lower, f"{label} launcher lost isolated commandlet flags")
    req("runtime_acceptance=0" in lower, f"{label} launcher lost non-acceptance marker")
    req("item16_checked=0" in lower, f"{label} launcher lost item16-open marker")
    for forbidden in ("git checkout", "git reset", "git clean", "git pull", "git push", "git merge"):
        req(forbidden not in lower, f"{label} launcher contains forbidden Git mutation: {forbidden}")

for needle in (
    "PASS45_M700_DERIVED_BOLT_TRANSLATION_UE58_PILOT_PASS",
    "bolt_stop_used_as_endpoint=0",
    "pilot_travel_accepted=0",
    "rotation_calibration_pending=1",
    "saved_packages=0",
    "production_profile_changed=0",
    "runtime_visual_acceptance=0",
):
    req(needle in m700_launcher, f"M700 proof invariant missing: {needle}")

for needle in (
    "PASS45_REMINGTON870_DERIVED_PUMP_UE58_PILOT_PASS",
    "PASS45_REMINGTON870_DERIVED_PUMP_UE58_ASSEMBLY_AUDIT_PASS",
    "production_visual_completeness=UNPROVEN",
    "production_cutover=0 runtime_acceptance=0 item16_checked=0",
):
    req(needle in remington_launcher, f"Remington proof invariant missing: {needle}")

for needle in (
    "PASS45_LEVERACTION_DERIVED_LEVER_UE58_PILOT_PASS",
    "source_authored_endpoint=0",
    "pilot_angle_accepted=0",
    "saved_packages=0",
    "production_profile_changed=0",
    "runtime_visual_acceptance=0",
):
    req(needle in lever_launcher, f"Lever proof invariant missing: {needle}")

for needle in (
    "PASS45_MANUAL_ACTION_AUDIO_UE_IMPORT.py",
    "PASS45_MANUAL_ACTION_AUDIO_UE_FRESH_LOAD.py",
    "SW_PASS45_BoltAction_CC0_Donor.uasset",
    "SW_PASS45_LeverAction_CC0_Donor.uasset",
    "PASS: manual-action BoltCycle/LeverCycle donor SoundWaves were imported and independently fresh-loaded.",
    "runtime_acceptance=0 item16_checked=0",
):
    req(needle in audio_launcher, f"audio launcher invariant missing: {needle}")

for needle in (
    'DESTINATION_PATH = "/Game/PASS45/Audio/ManualAction"',
    '"bolt": "SW_PASS45_BoltAction_CC0_Donor"',
    '"lever": "SW_PASS45_LeverAction_CC0_Donor"',
    'task.set_editor_property("save", True)',
    "save_loaded_asset",
    'print("runtime_acceptance=0 item16_checked=0")',
):
    req(needle in audio_import, f"audio importer scope/non-acceptance contract missing: {needle}")

for forbidden in (
    "/Game/Production/Weapons",
    "OCWeaponAnimationProfiles",
    "runtime_acceptance=1",
    "item16_checked=1",
):
    req(forbidden not in audio_import, f"audio importer regained forbidden production/acceptance behavior: {forbidden}")

for needle in (
    "SW_PASS45_BoltAction_CC0_Donor",
    "SW_PASS45_LeverAction_CC0_Donor",
    "runtime_acceptance=0",
    "item16_checked=0",
):
    req(needle in audio_fresh, f"audio fresh-load verifier invariant missing: {needle}")

for needle in (
    'EOCWeaponActionType::BoltAction',
    '/Game/PASS45/Audio/ManualAction/SW_PASS45_BoltAction_CC0_Donor.SW_PASS45_BoltAction_CC0_Donor',
    'RepositoryFallbackProfile->BoltCycle.Add(Bolt);',
    'EOCWeaponActionType::PumpAction',
    '/Game/R13/Audio/shotguncock.shotguncock',
    'RepositoryFallbackProfile->PumpCycle.Add(Pump);',
    'EOCWeaponActionType::LeverAction',
    '/Game/PASS45/Audio/ManualAction/SW_PASS45_LeverAction_CC0_Donor.SW_PASS45_LeverAction_CC0_Donor',
    'RepositoryFallbackProfile->LeverCycle.Add(Lever);',
    'case EOCWeaponAudioEvent::ManualActionCycle:',
):
    req(needle in audio_component, f"manual-action runtime audio routing invariant missing: {needle}")

req(
    '{ FName(TEXT("OC_SNP1")), TEXT(""), TEXT(""), true, TEXT(""), true }' in profiles,
    "M700 production manual-action path is no longer fail-closed/empty",
)
req(
    '{ FName(TEXT("R13_LEVER4570")), TEXT(""), TEXT(""), true, TEXT(""), true }' in profiles,
    "LeverAction production manual-action path is no longer fail-closed/empty",
)
req(
    'TEXT("/Game/Production/Weapons/Remington870/AN_Remington870_PumpCycle.AN_Remington870_PumpCycle")' in profiles,
    "Remington production pump sequence disappeared or drifted",
)
for forbidden in (
    "/Game/PASS45/ImportPilots/M700DerivedBoltTranslation",
    "AN_PASS45_M700_BoltTranslation_Pilot",
    "/Game/PASS45/ImportPilots/LeverActionDerivedLever",
):
    req(forbidden not in profiles, f"pilot path leaked into production animation profile: {forbidden}")

if errors:
    print("PASS45 ITEM16 LOCAL UE58 EVIDENCE CHAIN: FAIL")
    for error in errors:
        print(f"[FAIL] {error}")
    raise SystemExit(1)

print(
    "PASS45 ITEM16 LOCAL UE58 EVIDENCE CHAIN: PASS "
    "phase_count=4 fail_closed=1 motion_phases_unsaved=1 "
    "remington_assembly_includes_pump_motion=1 audio_only_save_bearing_phase=1 "
    "m700_profile_fail_closed=1 lever_profile_fail_closed=1 remington_production_pump_preserved=1 "
    "git_mutation=0 production_cutover=0 runtime_visual_acceptance=0 runtime_acceptance=0 "
    "item16_checked=0 merge_permitted=0"
)
