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
# The generic legacy whole-transform fallback code may remain available for future non-required experiments,
# but the exact production M700/870/Lever profiles must opt out while authored moving-part content is absent.
for needle in ("bWasActionCycling", "ActionCycleStartTime", "bAuthoredManualActionActive"):
    req(needle in presentation_h, f"manual-action presentation state missing: {needle}")
req("FTimerHandle" not in presentation_h,
    "first-person manual-action presentation introduced a second gameplay timer")
req("bool PlayWeaponAnimation" in presentation_h,
    "animation bridge no longer reports whether a production sequence actually started")

for needle in (
    "IsActionCycling()", "GetManualActionCycleDuration()", "GetWeaponActionType()",
    "bManualActionCueDeclared", "ManualActionWeaponLocation", "ManualActionArmsLocation",
    "EOCWeaponAudioEvent::ManualActionCycle",
    "OCResolveWeaponAnimationProfile", "HasManualActionAnimation()", "ManualActionAnimationObjectPath",
    "PASS45_MANUAL_ACTION_AUTHORED_SOURCE_BRIDGE_READY",
    "PASS45_MANUAL_ACTION_AUTHORED_SOURCE_BRIDGE_FAIL",
    "PASS45_MANUAL_ACTION_PROCEDURAL_FALLBACK_ACTIVE",
    "PASS45_MANUAL_ACTION_AUTHORED_CONTENT_GAP",
    "!State.bAuthoredManualActionActive",
    "whole_transform_only=1", "authored_moving_part=0", "second_gameplay_timer=0", "runtime_acceptance=0",
):
    req(needle in presentation_cpp, f"fail-honest manual-action presentation contract missing: {needle}")

req("PASS45_MANUAL_ACTION_PRESENTATION_READY" not in presentation_cpp,
    "procedural whole-transform fallback is falsely labelled production READY")

# The authored bridge is conditional: current exact manual-action sequence slots remain empty, but when a verified
# sequence is later committed the runtime consumer must load it, verify compatible skeletal playback and only then
# suppress any non-required fallback. It still cannot claim runtime acceptance from source wiring alone.
for needle in (
    '#include "OCWeaponAnimationProfiles.h"',
    "LoadObject<UAnimSequence>",
    "PlayWeaponAnimation(*Weapon, ManualActionSequence, State, ResetDelay)",
    "State.bAuthoredManualActionActive = true",
    "State.bAuthoredManualActionActive = false",
    "replicated_gate=1 second_gameplay_timer=0 runtime_acceptance=0",
):
    req(needle in presentation_cpp, f"manual-action authored source bridge wiring missing: {needle}")

for needle in (
    "ManualActionWeaponLocation", "ManualActionWeaponRotation", "ManualActionArmsLocation",
    "ManualActionArmsRotation", "bManualActionCueDeclared",
):
    req(needle in profiles_h, f"manual-action profile field missing: {needle}")
for needle in (
    "MakeM700Profile", "MakeRemington870Profile", "MakeLeverActionProfile",
    'FName(TEXT("OC_SNP1"))', 'FName(TEXT("OC_SG1"))', 'FName(TEXT("R13_LEVER4570"))',
):
    req(needle in profiles_cpp, f"manual-action profile declaration missing: {needle}")

# Required production manual-action weapons must not use the old whole-weapon/arms sine cue as a visual substitute.
# Keep each exact profile fail-closed until its authored sequence exists.
for function_name in ("MakeM700Profile", "MakeRemington870Profile", "MakeLeverActionProfile"):
    start = profiles_cpp.find(f"FOCFirstPersonWeaponProfile {function_name}()")
    end = profiles_cpp.find("return Profile;", start)
    block = profiles_cpp[start:end] if start >= 0 and end > start else ""
    req(bool(block), f"manual-action presentation profile missing: {function_name}")
    req("Profile.bManualActionCueDeclared = false;" in block,
        f"{function_name} must fail closed instead of enabling the whole-transform manual-action cue")
    for forbidden in (
        "Profile.ManualActionWeaponLocation =", "Profile.ManualActionWeaponRotation =",
        "Profile.ManualActionArmsLocation =", "Profile.ManualActionArmsRotation =",
    ):
        req(forbidden not in block,
            f"{function_name} still assigns a procedural whole-transform manual-action displacement: {forbidden}")

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

# Pump has one real repository-owned mechanical cue today; bolt and lever remain explicit gaps.
pump_asset = ROOT / "OsterConflict" / "Content" / "R13" / "Audio" / "shotguncock.uasset"
req(pump_asset.is_file(), "tracked Remington pump mechanical sound is missing: R13/Audio/shotguncock.uasset")
req('/Game/R13/Audio/shotguncock.shotguncock' in audio_component,
    "PumpAction fallback no longer points at the tracked shotguncock asset")
req("RepositoryFallbackProfile->PumpCycle.Add(Pump)" in audio_component,
    "tracked pump sound is not routed into PumpCycle")
req("RepositoryFallbackProfile->BoltCycle.Add" not in audio_component,
    "bolt cycle acquired an unverified generic repository fallback")
req("RepositoryFallbackProfile->LeverCycle.Add" not in audio_component,
    "lever cycle acquired an unverified generic repository fallback")

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

# Guard the meaning of open item 16 rather than one punctuation-sensitive sentence. The TZ may refine wording,
# but it must still say that procedural manual-action presentation is to be replaced by accepted authored
# moving-part/skeletal presentation and that real bolt/pump/lever sound content remains required.
for needle in (
    "Replace procedural manual-action",
    "accepted authored moving-part/skeletal presentation",
    "populate real bolt/pump/lever sound content",
    "RUNTIME REJECTED",
):
    req(needle in tz, f"canonical Pass45 TZ lost open item16 semantic truth: {needle}")

if errors:
    print("PASS45 WEAPON ACTION MATRIX: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 WEAPON ACTION MATRIX: PASS")
print("- bolt/pump/lever remain authoritative replicated post-shot gates with explicit timings")
print("- M700/870/LeverAction explicitly require authored articulated manual-action animation")
print("- required production profiles no longer enable the whole-weapon/arms procedural action cue")
print("- authored manual-action animation keeps a dedicated fail-closed profile slot and production skeletal consumer")
print("- ADS socket diagnostics have one dedicated mutation-free implementation owner in OCWeaponADSValidation.cpp")
print("- PumpCycle uses tracked R13/Audio/shotguncock; bolt/lever audio remain visible content gaps")
print("STATUS: SOURCE CONTRACT FAIL-CLOSED; exact authored moving-part sequences, bolt/lever audio and local UE 5.8 acceptance remain pending")
