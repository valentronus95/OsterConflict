#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
CONTENT = ROOT / "OsterConflict" / "Content"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS45 ITEM16/18 CONTENT GAP TRUTH: FAIL\n[FAIL] missing file: {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


errors: list[str] = []


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


animation_profiles = read(SRC / "Private" / "OCWeaponAnimationProfiles.cpp")
animation_header = read(SRC / "Public" / "OCWeaponAnimationProfiles.h")
audio_component = read(SRC / "Private" / "OCWeaponAudioComponent.cpp")
variants = read(SRC / "Private" / "OCWeaponVariants.cpp")
material_audit = read(ROOT / "VERIFY_PASS45_WEAPON_MATERIAL_DEPENDENCY_AUDIT.py")
tz = read(ROOT / "PASS45_RUNTIME_RECOVERY_TZ.md")

# Item 16: exact authored bolt/pump/lever moving-part sequences are not currently committed/wired.
# Keep the slots mandatory but empty until a compatible sequence exists; unrelated AK/reload motion is not evidence.
manual_action_profiles = (
    ('FName(TEXT("OC_SNP1"))', "M700", "BoltAction"),
    ('FName(TEXT("OC_SG1"))', "Remington870", "PumpAction"),
    ('FName(TEXT("R13_LEVER4570"))', "LeverAction", "LeverAction"),
)
for weapon_id, label, _action in manual_action_profiles:
    start = animation_profiles.find(weapon_id)
    end = animation_profiles.find("},", start)
    block = animation_profiles[start:end + 2] if start >= 0 and end > start else ""
    req(bool(block), f"manual-action animation profile missing: {label}")
    req('true, TEXT(""), true' in block,
        f"{label} must require articulated/manual-action coverage while exact sequence remains a content gap")
    req("AK-47_Fire_W" not in block and "AK-47_Reload_W" not in block,
        f"{label} borrowed unrelated AK animation to manufacture source readiness")

for needle in (
    "ManualActionAnimationObjectPath",
    "bRequiresManualActionAnimation",
    "HasManualActionAnimation()",
    "HasRequiredManualActionCoverage()",
):
    req(needle in animation_header, f"manual-action exact-content contract missing: {needle}")

pump_sound = CONTENT / "R13" / "Audio" / "shotguncock.uasset"
req(pump_sound.is_file(), "tracked pump mechanical cue disappeared: R13/Audio/shotguncock.uasset")
req('/Game/R13/Audio/shotguncock.shotguncock' in audio_component,
    "tracked pump mechanical cue is no longer loaded")
req("RepositoryFallbackProfile->PumpCycle.Add(Pump)" in audio_component,
    "tracked pump mechanical cue is not routed into PumpCycle")
req("RepositoryFallbackProfile->BoltCycle.Add" not in audio_component,
    "BoltCycle acquired an unverified generic fallback")
req("RepositoryFallbackProfile->LeverCycle.Add" not in audio_component,
    "LeverCycle acquired an unverified generic fallback")
req("PASS45_WEAPON_AUDIO_CONTENT_GAP" in audio_component,
    "manual-action missing audio no longer fails visibly")

# Item 18: distinguish current runtime weapon availability from broader exact-production catalog debt.
# M16/M4 is named by the canonical TZ as an exact-production gap but is not a current runtime class/content path;
# it must therefore never be counted inside a fabricated current-rack READY total.
for needle in (
    "Remington 870",
    "M249",
    "M16/M4",
):
    req(needle in tz, f"canonical exact-production content-gap ledger lost: {needle}")

req("M16" not in variants and "M4" not in variants,
    "M16/M4 unexpectedly entered current runtime weapon variants; material/rack gates must be extended before READY")

content_paths_lower = [p.as_posix().lower() for p in CONTENT.rglob("*") if p.is_file()]
req(not any("m16" in p or "/m4" in p or "m4_" in p for p in content_paths_lower),
    "M16/M4-like content appeared; canonical exact-production gap requires explicit intake/validation before status changes")

# The current required-available material validator intentionally audits implemented runtime weapons. It must not
# pretend M16/M4 is part of that runtime set, while Remington/M249 remain explicit exact payload gaps there.
req('(\"Remington870\", Path(\"Production/Weapons/Remington870/SM_Remington870.uasset\"))' in material_audit,
    "Remington870 exact-production gap disappeared from runtime material audit")
req('(\"M249\", Path(\"Production/Weapons/M249/SM_M249.uasset\"))' in material_audit,
    "M249 exact-production gap disappeared from runtime material audit")
req('("M16' not in material_audit and '("M4' not in material_audit,
    "catalog-only M16/M4 debt was incorrectly folded into the current runtime rack validator")

for needle in (
    "Replace procedural manual-action cues with accepted authored moving-part/skeletal presentation",
    "populate real bolt/pump/lever sound content",
    "Explicit exact-production gaps unless later content closes them: Remington 870, M249, M16/M4.",
    "RUNTIME REJECTED 2026-08-27",
):
    req(needle in tz, f"canonical Pass45 open-gap truth missing: {needle}")

if errors:
    print("PASS45 ITEM16/18 CONTENT GAP TRUTH: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 ITEM16/18 CONTENT GAP TRUTH: PASS")
print("- M700/Remington870/LeverAction still require exact authored moving-part sequences; no unrelated animation is substituted")
print("- Remington pump uses the tracked shotguncock mechanical cue; bolt/lever audio remain explicit content gaps")
print("- Remington870/M249 are current-runtime exact-production gaps; M16/M4 remains separate catalog exact-production debt")
print("STATUS: FAIL-HONEST SOURCE LEDGER ONLY; items 16/18 remain open pending exact content and local UE 5.8 acceptance")
