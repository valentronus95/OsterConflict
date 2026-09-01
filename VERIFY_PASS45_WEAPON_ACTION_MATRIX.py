#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent

def read(rel: str) -> str:
    path = ROOT / rel
    if not path.is_file():
        raise SystemExit(f"PASS45 WEAPON ACTION MATRIX: FAIL\n[FAIL] missing file: {rel}")
    return path.read_text(encoding="utf-8", errors="replace")

errors: list[str] = []

def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


types = read("OsterConflict/Source/OsterConflict/Public/OCWeaponTypes.h")
base_h = read("OsterConflict/Source/OsterConflict/Public/OCWeaponBase.h")
base_cpp = read("OsterConflict/Source/OsterConflict/Private/OCWeaponBase.cpp")
manual_action_cpp = read("OsterConflict/Source/OsterConflict/Private/OCWeaponManualActionPresentation.cpp")
character_h = read("OsterConflict/Source/OsterConflict/Public/OCCharacter.h")
character_cpp = read("OsterConflict/Source/OsterConflict/Private/OCCharacter.cpp")
variants = read("OsterConflict/Source/OsterConflict/Private/OCWeaponVariants.cpp")
launcher = read("OsterConflict/Source/OsterConflict/Private/OCAntiArmorLauncher.cpp")
presentation_h = read("OsterConflict/Source/OsterConflict/Public/OCFirstPersonWeaponPresentationSubsystem.h")
presentation_cpp = read("OsterConflict/Source/OsterConflict/Private/OCFirstPersonWeaponPresentationSubsystem.cpp")
ads_cpp = read("OsterConflict/Source/OsterConflict/Private/OCWeaponADSValidation.cpp")
profiles_h = read("OsterConflict/Source/OsterConflict/Public/OCWeaponPresentationProfiles.h")
profiles_cpp = read("OsterConflict/Source/OsterConflict/Private/OCWeaponPresentationProfiles.cpp")
animation_profiles_h = read("OsterConflict/Source/OsterConflict/Public/OCWeaponAnimationProfiles.h")
animation_profiles_cpp = read("OsterConflict/Source/OsterConflict/Private/OCWeaponAnimationProfiles.cpp")
audio_types = read("OsterConflict/Source/OsterConflict/Public/OCAudioTypes.h")
audio_profile = read("OsterConflict/Source/OsterConflict/Public/OCWeaponAudioProfile.h")
audio_component = read("OsterConflict/Source/OsterConflict/Private/OCWeaponAudioComponent.cpp")
tz = read("PASS45_RUNTIME_RECOVERY_TZ.md")

for needle in (
    "enum class EOCWeaponActionType", "GasOperated", "DelayedBlowback", "Blowback", "ShortRecoil",
    "BoltAction", "PumpAction", "LeverAction", "BeltFed", "LauncherSingleShot", "Burst3",
    "EOCWeaponActionType ActionType", "float ManualActionCycleSeconds = 0.0f", "bool bSupportsBurst3 = false",
):
    req(needle in types, f"weapon action/selector source contract missing: {needle}")

for needle in (
    "GetWeaponActionType() const", "GetManualActionCycleDuration() const", "IsActionCycling() const",
    "SupportsFireMode(EOCFireMode Mode) const", "ReplicatedUsing=OnRep_ActionCycling",
    "bool bActionCycling = false", "void OnRep_ActionCycling();", "FTimerHandle ManualActionTimerHandle",
):
    req(needle in base_h, f"weapon selector/action API missing: {needle}")

for needle in (
    "DOREPLIFETIME(AOCWeaponBase, bActionCycling)", "RequiresManualActionCycle() const",
    "EOCWeaponActionType::BoltAction", "EOCWeaponActionType::PumpAction", "EOCWeaponActionType::LeverAction",
    "BeginManualActionCycleServer()", "FinishManualActionCycleServer()", "PASS45_MANUAL_ACTION_CYCLE_READY",
):
    req(needle in base_cpp, f"authoritative manual-action gate missing: {needle}")

for weapon_id, action, cycle in (
    ('TEXT("OC_SNP1")', "EOCWeaponActionType::BoltAction", "T.ManualActionCycleSeconds = 1.10f"),
    ('TEXT("OC_SG1")', "EOCWeaponActionType::PumpAction", "T.ManualActionCycleSeconds = 0.72f"),
    ('TEXT("R13_LEVER4570")', "EOCWeaponActionType::LeverAction", "T.ManualActionCycleSeconds = 0.85f"),
):
    start = variants.find(weapon_id)
    end = variants.find("ConfigureBuiltInTuning(T);", start)
    block = variants[start:end] if start >= 0 and end > start else ""
    req(bool(block), f"manual-action weapon missing: {weapon_id}")
    req(action in block, f"action mismatch for {weapon_id}: {action}")
    req(cycle in block, f"cycle timing missing for {weapon_id}: {cycle}")

req('TEXT("OC_RPG1")' in launcher and "EOCWeaponActionType::LauncherSingleShot" in launcher,
    "launcher action truth is not LauncherSingleShot")
req("bSupportsBurst3 = true" not in variants and "bSupportsBurst3=true" not in launcher,
    "a current production weapon falsely enables unaccepted Burst3")

for needle in (
    "ServerBurstShotsRemaining", "FMath::Min(3, CurrentWeapon->GetAmmoInMagazine())",
    "--ServerBurstShotsRemaining", "PASS45_BURST3_SEQUENCE_READY",
):
    req(needle in character_h + character_cpp, f"finite authoritative Burst3 contract missing: {needle}")

# First-person presentation may observe the replicated gate but must not own a second gameplay timer.
# The rejected whole-weapon/arms manual-action fallback is now physically retired rather than left dormant.
req("bWasActionCycling" in presentation_h,
    "manual-action replicated-gate observation state is missing")
for forbidden in ("ActionCycleStartTime", "bAuthoredManualActionActive", "FTimerHandle"):
    req(forbidden not in presentation_h,
        f"retired/manual-action-only presentation state returned: {forbidden}")
req("bool PlayWeaponAnimation" in presentation_h,
    "animation bridge no longer reports whether a production sequence actually started")

for needle in (
    "IsActionCycling()", "GetManualActionCycleDuration()", "GetWeaponActionType()",
    "EOCWeaponAudioEvent::ManualActionCycle",
    "OCResolveWeaponAnimationProfile", "HasManualActionAnimation()", "ManualActionAnimationObjectPath",
    "PASS45_MANUAL_ACTION_AUTHORED_SOURCE_BRIDGE_READY",
    "PASS45_MANUAL_ACTION_AUTHORED_SOURCE_BRIDGE_FAIL",
    "PASS45_MANUAL_ACTION_AUTHORED_CONTENT_GAP",
    "bAuthoredManualActionStarted",
    "procedural_fallback=0", "baseline_transform_preserved=1",
    "authored_moving_part=0", "second_gameplay_timer=0", "runtime_acceptance=0",
):
    req(needle in presentation_cpp, f"fail-closed manual-action presentation contract missing: {needle}")

for forbidden in (
    "PASS45_MANUAL_ACTION_PRESENTATION_READY",
    "PASS45_MANUAL_ACTION_PROCEDURAL_FALLBACK_ACTIVE",
    "bManualActionCueDeclared",
    "ManualActionWeaponLocation", "ManualActionWeaponRotation",
    "ManualActionArmsLocation", "ManualActionArmsRotation",
    "whole_transform_only=1",
):
    req(forbidden not in presentation_cpp,
        f"retired procedural manual-action fallback returned to presentation source: {forbidden}")

# The authored bridge is conditional: current exact manual-action sequence slots remain empty, but when a verified
# sequence is later committed the runtime consumer must load it, verify compatible skeletal playback and play it.
# Missing/incompatible authored content preserves baseline transforms and remains a visible content gap.
for needle in (
    '#include "OCWeaponAnimationProfiles.h"',
    "LoadObject<UAnimSequence>",
    "PlayWeaponAnimation(*Weapon, ManualActionSequence, State, ResetDelay)",
    "bAuthoredManualActionStarted = PlayWeaponAnimation",
    "replicated_gate=1 second_gameplay_timer=0 runtime_acceptance=0",
):
    req(needle in presentation_cpp, f"manual-action authored source bridge wiring missing: {needle}")

# Camera-space first-person profiles must no longer expose a dormant manual-action transform API.
for forbidden in (
    "ManualActionWeaponLocation", "ManualActionWeaponRotation", "ManualActionArmsLocation",
    "ManualActionArmsRotation", "bManualActionCueDeclared",
):
    req(forbidden not in profiles_h,
        f"retired manual-action transform profile field returned: {forbidden}")

for needle in (
    "MakeM700Profile", "MakeRemington870Profile", "MakeLeverActionProfile",
    'FName(TEXT("OC_SNP1"))', 'FName(TEXT("OC_SG1"))', 'FName(TEXT("R13_LEVER4570"))',
):
    req(needle in profiles_cpp, f"manual-action weapon profile declaration missing: {needle}")

for forbidden in (
    "bManualActionCueDeclared", "ManualActionWeaponLocation", "ManualActionWeaponRotation",
    "ManualActionArmsLocation", "ManualActionArmsRotation",
):
    req(forbidden not in profiles_cpp,
        f"retired manual-action transform profile assignment returned: {forbidden}")

# Authored manual-action coverage must be a first-class animation contract. This does not claim content exists:
# current M700/870/Lever slots are deliberately empty until exact compatible sequences are committed.
for needle in (
    "ManualActionAnimationObjectPath", "bRequiresManualActionAnimation",
    "HasManualActionAnimation()", "HasRequiredManualActionCoverage()",
):
    req(needle in animation_profiles_h, f"manual-action authored animation slot missing: {needle}")

for weapon_id in ('FName(TEXT("OC_SNP1"))', 'FName(TEXT("OC_SG1"))', 'FName(TEXT("R13_LEVER4570"))'):
    start = animation_profiles_cpp.find(weapon_id)
    end = animation_profiles_cpp.find("},", start)
    block = animation_profiles_cpp[start:end + 2] if start >= 0 and end > start else ""
    req(bool(block), f"animation profile missing manual-action weapon: {weapon_id}")
    req('true, TEXT(""), true' in block,
        f"{weapon_id} must require articulated/manual-action coverage while exact authored sequence remains empty")

# Do not borrow the AK fire/reload sequences for bolt/pump/lever just to manufacture a green source status.
for weapon_id in ('FName(TEXT("OC_SNP1"))', 'FName(TEXT("OC_SG1"))', 'FName(TEXT("R13_LEVER4570"))'):
    start = animation_profiles_cpp.find(weapon_id)
    end = animation_profiles_cpp.find("},", start)
    block = animation_profiles_cpp[start:end + 2] if start >= 0 and end > start else ""
    req("AK-47_Fire_W" not in block and "AK-47_Reload_W" not in block,
        f"unrelated AK animation leaked into manual-action profile: {weapon_id}")

req("ManualActionCycle" in audio_types, "manual-action audio event enum missing")
for needle in ("BoltCycle", "PumpCycle", "LeverCycle"):
    req(needle in audio_profile, f"manual-action audio profile slot missing: {needle}")
for needle in (
    "EOCWeaponAudioEvent::ManualActionCycle", "GetWeaponActionType()",
    "EOCWeaponActionType::BoltAction", "EOCWeaponActionType::PumpAction", "EOCWeaponActionType::LeverAction",
    "Profile->BoltCycle", "Profile->PumpCycle", "Profile->LeverCycle", "MANUAL ACTION(content gap)",
):
    req(needle in audio_component, f"manual-action audio routing/content-gap truth missing: {needle}")

# Source routes may point at repository-owned donor object paths before import only through LoadSound guards.
# If the imported .uasset is absent, LoadSound returns null and the corresponding fallback array stays empty,
# preserving the visible content-gap state instead of manufacturing runtime acceptance.
pump_asset = ROOT / "OsterConflict" / "Content" / "R13" / "Audio" / "shotguncock.uasset"
req(pump_asset.is_file(), "tracked Remington pump mechanical sound is missing: R13/Audio/shotguncock.uasset")
for needle in (
    "/Game/PASS45/Audio/ManualAction/SW_PASS45_BoltAction_CC0_Donor.SW_PASS45_BoltAction_CC0_Donor",
    "/Game/R13/Audio/shotguncock.shotguncock",
    "/Game/PASS45/Audio/ManualAction/SW_PASS45_LeverAction_CC0_Donor.SW_PASS45_LeverAction_CC0_Donor",
    "if (USoundBase* Bolt = LoadSound",
    "RepositoryFallbackProfile->BoltCycle.Add(Bolt)",
    "if (USoundBase* Pump = LoadSound",
    "RepositoryFallbackProfile->PumpCycle.Add(Pump)",
    "if (USoundBase* Lever = LoadSound",
    "RepositoryFallbackProfile->LeverCycle.Add(Lever)",
):
    req(needle in audio_component, f"manual-action repository fallback route missing: {needle}")

for needle in ("void AOCWeaponBase::OnRep_ActionCycling()", "OwnerCharacter->IsLocallyControlled()", "ManualActionCycle"):
    req(needle in manual_action_cpp, f"remote manual-action replication/audio path missing: {needle}")

# ADS diagnostics used to be declared/called without a definition, which source-only CI could miss until UE link.
# The implementation has one dedicated owner in OCWeaponADSValidation.cpp. The presentation translation unit must
# contain only the ADS-entry call; putting the body back there reintroduces the historical LNK2005 duplicate symbol.
ads_definition = "UOCFirstPersonWeaponPresentationSubsystem::ValidateADSAlignment"
req("ValidateADSAlignment(Character, *Weapon, FindProductionWeaponVisual(*Weapon), Profile);" in presentation_cpp,
    "ValidateADSAlignment ADS entry call is missing from first-person presentation")
req(presentation_cpp.count(ads_definition) == 0,
    "duplicate ValidateADSAlignment definition returned to OCFirstPersonWeaponPresentationSubsystem.cpp")
req(ads_cpp.count(ads_definition) == 1,
    "ValidateADSAlignment must have exactly one concrete implementation in OCWeaponADSValidation.cpp")
for needle in (
    'TEXT("oc.Weapon.ADS.Debug")',
    "PASS45_ADS_PROFILE_UNCALIBRATED",
    "PASS45_ADS_ALIGNMENT_FAIL",
    "PASS45_ADS_ALIGNMENT_SAMPLE",
    "ADSOpticSocket", "ADSRearSightSocket", "ADSFrontSightSocket",
    "DoesSocketExist", "GetSocketTransform", "GetSocketLocation",
    "AngularErrorDegrees", "CameraToSightLineCm",
    "runtime_visual_acceptance=pending",
):
    req(needle in ads_cpp, f"ADS alignment diagnostic implementation missing: {needle}")
req("FTimerHandle" not in ads_cpp,
    "ADS alignment diagnostics introduced a second gameplay timer")

# Guard the meaning of open item 16 rather than one punctuation-sensitive sentence.
for needle in (
    "Replace procedural manual-action",
    "accepted authored moving-part/skeletal presentation",
    "populate real bolt/pump/lever sound content",
    "RUNTIME REJECTED 2026-08-31",
):
    req(needle in tz, f"canonical Pass45 TZ lost current open item16/runtime truth: {needle}")

if errors:
    print("PASS45 WEAPON ACTION MATRIX: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 WEAPON ACTION MATRIX: PASS")
print("- bolt/pump/lever remain authoritative replicated post-shot gates with explicit timings")
print("- M700/870/LeverAction explicitly require authored articulated manual-action animation")
print("- rejected whole-weapon/arms procedural manual-action fallback is physically retired")
print("- missing authored action content preserves baseline transforms and remains a hard-visible content gap")
print("- authored manual-action animation keeps a dedicated fail-closed profile slot and production skeletal consumer")
print("- pump uses tracked R13 audio; bolt/lever source routes target provenance-pinned repository donors and stay fail-closed until UE import")
print("STATUS: SOURCE CONTRACT FAIL-CLOSED; UE donor SoundWave import/fresh-load, authored moving-part sequences and local UE 5.8 acceptance remain pending")
