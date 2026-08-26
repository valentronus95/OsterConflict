#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
TYPES = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public" / "OCWeaponTypes.h"
BASE_H = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public" / "OCWeaponBase.h"
BASE_CPP = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCWeaponBase.cpp"
CHARACTER_H = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public" / "OCCharacter.h"
CHARACTER_CPP = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCCharacter.cpp"
VARIANTS = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCWeaponVariants.cpp"
LAUNCHER = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCAntiArmorLauncher.cpp"
TZ = ROOT / "PASS45_RUNTIME_RECOVERY_TZ.md"

errors: list[str] = []


def read(path: Path) -> str:
    if not path.is_file():
        errors.append(f"missing file: {path.relative_to(ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


types = read(TYPES)
base_h = read(BASE_H)
base_cpp = read(BASE_CPP)
character_h = read(CHARACTER_H)
character_cpp = read(CHARACTER_CPP)
variants = read(VARIANTS)
launcher = read(LAUNCHER)
tz = read(TZ)

# Pass45 must no longer pretend every weapon is just semi/auto. The source model carries an explicit
# mechanical action family and an opt-in burst selector capability without inventing burst support for real assets.
for needle in (
    "enum class EOCWeaponActionType",
    "GasOperated",
    "DelayedBlowback",
    "Blowback",
    "ShortRecoil",
    "BoltAction",
    "PumpAction",
    "LeverAction",
    "BeltFed",
    "LauncherSingleShot",
    "Burst3",
    "EOCWeaponActionType ActionType",
    "bool bSupportsBurst3 = false",
):
    req(needle in types, f"weapon action/selector source contract missing: {needle}")

# Public weapon state must expose the action family and determine supported selector positions from tuning data.
for needle in (
    "GetWeaponActionType() const",
    "SupportsFireMode(EOCFireMode Mode) const",
    "case EOCFireMode::SemiAutomatic: return Tuning.bSupportsSemiAutomatic",
    "case EOCFireMode::Burst3: return Tuning.bSupportsBurst3",
    "case EOCFireMode::Automatic: return Tuning.bSupportsAutomatic",
):
    req(needle in base_h, f"weapon selector/action API missing: {needle}")

# Initial selector and B-key cycling are data-driven. Burst is a valid architecture position but remains skipped
# when the exact weapon tuning does not explicitly enable it.
for needle in (
    "Tuning.bSupportsBurst3 ? EOCFireMode::Burst3 : EOCFireMode::SemiAutomatic",
    "static constexpr EOCFireMode SelectorOrder[]",
    "EOCFireMode::SemiAutomatic",
    "EOCFireMode::Burst3",
    "EOCFireMode::Automatic",
    "SupportsFireMode(Candidate)",
    "MulticastWeaponStateAudio(EOCWeaponAudioEvent::FireModeSwitch",
    "ForceNetUpdate();",
):
    req(needle in base_cpp, f"data-driven selector cycle contract missing: {needle}")

expected_actions = {
    'TEXT("OC_AR1")': "EOCWeaponActionType::GasOperated",
    'TEXT("OC_SMG1")': "EOCWeaponActionType::DelayedBlowback",
    'TEXT("OC_PST1")': "EOCWeaponActionType::ShortRecoil",
    'TEXT("OC_SNP1")': "EOCWeaponActionType::BoltAction",
    'TEXT("OC_SG1")': "EOCWeaponActionType::PumpAction",
    'TEXT("OC_LMG1")': "EOCWeaponActionType::BeltFed",
    'TEXT("R13_M14")': "EOCWeaponActionType::GasOperated",
    'TEXT("R13_MAC10")': "EOCWeaponActionType::Blowback",
    'TEXT("R13_TEC9")': "EOCWeaponActionType::Blowback",
    'TEXT("R13_LEVER4570")': "EOCWeaponActionType::LeverAction",
}

for weapon_id, action in expected_actions.items():
    start = variants.find(weapon_id)
    req(start >= 0, f"weapon variant missing from action matrix: {weapon_id}")
    if start < 0:
        continue
    end = variants.find("ConfigureBuiltInTuning(T);", start)
    req(end >= 0 and action in variants[start:end],
        f"weapon action mismatch for {weapon_id}: expected {action}")

req('TEXT("OC_RPG1")' in launcher and "EOCWeaponActionType::LauncherSingleShot" in launcher,
    "anti-armor launcher action truth is not LauncherSingleShot")

# Do not invent a burst-capable production selector merely because the architecture now supports one.
req("bSupportsBurst3 = true" not in variants and "bSupportsBurst3=true" not in launcher,
    "a current weapon claims 3-round burst without an explicitly accepted selector configuration")

# Burst3 has an authoritative finite sequence before any production asset is allowed to enable it. Trigger release
# does not truncate an accepted three-shot sequence; sprint/reload/equip/drop/death use StopServerFireTimer as the
# hard cancellation path. This avoids the usual human invention where 'burst' is secretly just very short auto.
for needle in (
    "int32 ServerBurstShotsRemaining = 0",
    "ServerBurstShotsRemaining = FMath::Min(3, CurrentWeapon->GetAmmoInMagazine());",
    "const bool bBurstActive = ServerBurstShotsRemaining > 0;",
    "--ServerBurstShotsRemaining;",
    "if (ServerBurstShotsRemaining <= 0)",
    "PASS45_BURST3_SEQUENCE_READY",
    "finite_shots=3",
    "release_cancel=0",
):
    req(needle in character_h + character_cpp, f"authoritative Burst3 sequence contract missing: {needle}")

stop_start = character_cpp.find("void AOCCharacter::StopServerFireTimer()")
stop_end = character_cpp.find("void AOCCharacter::ReloadPressed()", stop_start)
req(stop_start >= 0 and stop_end > stop_start and "ServerBurstShotsRemaining = 0;" in character_cpp[stop_start:stop_end],
    "hard fire-stop path does not clear pending Burst3 sequence")

release_start = character_cpp.find("if (!bHeld)")
release_end = character_cpp.find("// Do not restart or stack a burst", release_start)
req(release_start >= 0 and release_end > release_start and
    "if (ServerBurstShotsRemaining <= 0)" in character_cpp[release_start:release_end] and
    "ClearTimer(ServerFireTimerHandle)" in character_cpp[release_start:release_end],
    "trigger release no longer preserves an already accepted finite Burst3 sequence")

req("RUNTIME REJECTED 2026-08-26" in tz,
    "canonical Pass45 TZ lost the latest factual runtime rejection")

if errors:
    print("PASS45 WEAPON ACTION MATRIX: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 WEAPON ACTION MATRIX: PASS")
print("- weapon tuning separates mechanical action from broad weapon class")
print("- supported selector positions are exposed and cycled from weapon tuning data")
print("- current variants declare gas/delayed-blowback/blowback/short-recoil/bolt/pump/lever/belt/launcher truth")
print("- Burst3 owns an authoritative finite three-shot sequence, but no current production weapon falsely enables it")
print("STATUS: SOURCE SEQUENCING CODED; manual bolt/pump/lever presentation and local UE 5.8 runtime behavior remain pending")
