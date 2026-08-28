#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
HEADER = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public" / "OCWeaponAudioComponent.h"
CPP = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCWeaponAudioComponent.cpp"
TZ = ROOT / "PASS45_RUNTIME_RECOVERY_TZ.md"

TRACKED_AUDIO = [
    ROOT / "OsterConflict" / "Content" / "R13" / "Audio" / "gunfire_sfx.uasset",
    ROOT / "OsterConflict" / "Content" / "R13" / "Audio" / "gunreload1.uasset",
    ROOT / "OsterConflict" / "Content" / "R13" / "Audio" / "assaultriflereload1.uasset",
    ROOT / "OsterConflict" / "Content" / "R13" / "Audio" / "shotguncock.uasset",
    ROOT / "OsterConflict" / "Content" / "R13" / "Audio" / "snd_bullethit.uasset",
    ROOT / "OsterConflict" / "Content" / "AK-47" / "Sound" / "AK-47" / "Cues" / "AK47_Fire_Cue.uasset",
    ROOT / "OsterConflict" / "Content" / "AK-47" / "Sound" / "AK-47" / "Cues" / "Reload_Cue.uasset",
    ROOT / "OsterConflict" / "Content" / "AK-47" / "Sound" / "AK-47" / "Cues" / "AK47_Empty_Cue.uasset",
]

errors: list[str] = []


def read(path: Path) -> str:
    if not path.is_file():
        errors.append(f"missing file: {path.relative_to(ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


header = read(HEADER)
cpp = read(CPP)
tz = read(TZ)

for path in TRACKED_AUDIO:
    req(path.is_file(), f"tracked repository audio missing: {path.relative_to(ROOT)}")

for needle in (
    "EnsureRepositoryFallbackProfile()",
    "RepositoryFallbackProfile",
    "bRepositoryFallbackAttempted",
):
    req(needle in header, f"weapon audio fallback header contract missing: {needle}")

for needle in (
    "/Game/AK-47/Sound/AK-47/Cues/AK47_Fire_Cue.AK47_Fire_Cue",
    "/Game/AK-47/Sound/AK-47/Cues/Reload_Cue.Reload_Cue",
    "/Game/AK-47/Sound/AK-47/Cues/AK47_Empty_Cue.AK47_Empty_Cue",
    "/Game/R13/Audio/gunfire_sfx.gunfire_sfx",
    "/Game/R13/Audio/gunreload1.gunreload1",
    "/Game/R13/Audio/assaultriflereload1.assaultriflereload1",
    "/Game/R13/Audio/shotguncock.shotguncock",
    "/Game/R13/Audio/snd_bullethit.snd_bullethit",
    "PASS45_WEAPON_AUDIO_FALLBACK_READY",
    "PASS45_WEAPON_AUDIO_CONTENT_GAP",
    "if (!HasRequestedNearShot(ShotProfile))",
    "ShotProfile = EnsureRepositoryFallbackProfile();",
    "if (!Tail) Tail = Pick(*NearSet, EventSeed + 17);",
    "StateProfile = EnsureRepositoryFallbackProfile();",
    "ImpactProfile = EnsureRepositoryFallbackProfile();",
    "EOCWeaponActionType::PumpAction",
    "RepositoryFallbackProfile->PumpCycle.Add(Pump)",
):
    req(needle in cpp, f"weapon audio fallback source contract missing: {needle}")

start = cpp.find("UOCWeaponAudioProfile* UOCWeaponAudioComponent::EnsureRepositoryFallbackProfile()")
end = cpp.find("EOCAcousticEnvironment UOCWeaponAudioComponent::DetectEnvironmentAt", start)
fallback_block = cpp[start:end] if start >= 0 and end > start else ""
req(bool(fallback_block), "cannot isolate repository audio fallback function")
for forbidden in (
    "AmmoInMagazine",
    "ReserveAmmo",
    "TryFireServer",
    "RoundsPerMinute",
    "SetActorLocation",
    "ApplyDamage",
):
    req(forbidden not in fallback_block,
        f"audio presentation fallback illegally mutates gameplay/authority contract: {forbidden}")

for needle in (
    "repository weapon-audio fallback",
    "PASS45_WEAPON_AUDIO_FALLBACK_READY",
    "AUDIO CONTENT GAP",
    "RUNTIME REJECTED",
    "2026-08-27",
):
    req(needle in tz, f"canonical Pass45 TZ lost current audio truth: {needle}")

if errors:
    print("PASS45 WEAPON AUDIO FALLBACK: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 WEAPON AUDIO FALLBACK: PASS")
print("- exact AK project audio is preferred when no assigned profile supplies the event")
print("- other silent weapons receive a repository-owned factual shot fallback instead of disappearing acoustically")
print("- assigned authored event sets still win; fallback is event-local and presentation-only")
print("- pump action can reuse the tracked shotgun cock asset; bolt/lever remain explicit content gaps until exact audio exists")
print("STATUS: SOURCE-CODED; local UE 5.8 load/audibility/mix/content-quality acceptance remains pending")
