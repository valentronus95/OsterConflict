#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
PROFILES_H = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public" / "OCWeaponPresentationProfiles.h"
PROFILES_CPP = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCWeaponPresentationProfiles.cpp"
PRESENTATION_H = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public" / "OCFirstPersonWeaponPresentationSubsystem.h"
PRESENTATION_CPP = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCFirstPersonWeaponPresentationSubsystem.cpp"
ADS_CPP = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCWeaponADSValidation.cpp"
TZ = ROOT / "PASS45_RUNTIME_RECOVERY_TZ.md"
LATEST_RUNTIME_EVIDENCE = ROOT / "RUNTIME_EVIDENCE" / "2026-08-27_PASS45_REJECTED" / "README.md"

errors: list[str] = []


def read(path: Path) -> str:
    if not path.is_file():
        errors.append(f"missing file: {path.relative_to(ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


profiles_h = read(PROFILES_H)
profiles_cpp = read(PROFILES_CPP)
presentation_h = read(PRESENTATION_H)
presentation_cpp = read(PRESENTATION_CPP)
ads_cpp = read(ADS_CPP)
tz = read(TZ)
latest_runtime_evidence = read(LATEST_RUNTIME_EVIDENCE)

for weapon_id in (
    "OC_AR1", "OC_SMG1", "OC_PST1", "OC_SNP1", "OC_SG1", "OC_LMG1",
    "R13_M14", "R13_MAC10", "R13_TEC9", "R13_LEVER4570", "OC_RPG1",
):
    req(f'FName(TEXT("{weapon_id}"))' in profiles_cpp,
        f"first-person profile registry missing weapon id: {weapon_id}")

for needle in (
    "ADSWeaponOffset",
    "ADSWeaponRotationOffset",
    "ADSArmsOffset",
    "ADSArmsRotationOffset",
    "ADSRearSightSocket",
    "ADSFrontSightSocket",
    "ADSOpticSocket",
    "bool bADSCalibrated = false",
):
    req(needle in profiles_h, f"per-weapon ADS profile field missing: {needle}")

# No current asset has factual socket/sight calibration evidence yet. A future accepted calibration must update
# this verifier together with the exact socket names and UE evidence rather than silently toggling READY.
req("bADSCalibrated = true" not in profiles_cpp,
    "ADS profile was promoted to calibrated without updating the factual calibration verifier/evidence contract")

for needle in (
    "bWasAiming",
    "ValidateADSAlignment",
):
    req(needle in presentation_h, f"ADS transition/validation state missing: {needle}")

for needle in (
    "if (bADS && !State.bWasAiming && bDeclaredProfile)",
    "ValidateADSAlignment(Character, *Weapon, FindProductionWeaponVisual(*Weapon), Profile);",
    "State.bWasAiming = bADS;",
):
    req(needle in presentation_cpp, f"ADS entry validation hook missing: {needle}")

for needle in (
    'TEXT("oc.Weapon.ADS.Debug")',
    "PASS45_ADS_PROFILE_UNCALIBRATED",
    "no_fake_ready=1",
    "PASS45_ADS_ALIGNMENT_FAIL",
    "PASS45_ADS_ALIGNMENT_SAMPLE",
    "Profile.bADSCalibrated",
    "DoesSocketExist(Profile.ADSOpticSocket)",
    "DoesSocketExist(Profile.ADSRearSightSocket)",
    "DoesSocketExist(Profile.ADSFrontSightSocket)",
    "GetSocketTransform(Profile.ADSOpticSocket, RTS_World)",
    "GetSocketLocation(Profile.ADSRearSightSocket)",
    "GetSocketLocation(Profile.ADSFrontSightSocket)",
    "AngularErrorDegrees",
    "CameraToSightLineCm",
    "DrawDebugLine",
    "runtime_visual_acceptance=pending",
):
    req(needle in ads_cpp, f"ADS alignment diagnostic contract missing: {needle}")

req("FTimerHandle" not in ads_cpp,
    "ADS diagnostics introduced a gameplay timer instead of observing presentation state")

for needle in (
    "PASS45_ADS_PROFILE_UNCALIBRATED",
    "oc.Weapon.ADS.Debug",
    "bADSCalibrated",
):
    req(needle in tz, f"canonical Pass45 TZ lost ADS calibration truth: {needle}")

# Runtime truth is versioned evidence, not a hard-coded historical date in the source-contract verifier.
# The 2026-08-27 screenshots explicitly reject AK ADS/hand presentation and outrank green source tests.
for needle in (
    "RUNTIME REJECTED",
    "2026-08-27",
    "AK47",
):
    req(needle in latest_runtime_evidence,
        f"latest runtime rejection evidence lost required ADS truth: {needle}")

if errors:
    print("PASS45 WEAPON ADS ALIGNMENT: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 WEAPON ADS ALIGNMENT: PASS")
print("- all current weapon ids resolve through the explicit first-person profile registry")
print("- ADS profiles carry optional rear/front/optic sight references and a separate factual calibration flag")
print("- entering ADS runs fail-visible calibration diagnostics; uncalibrated profiles cannot impersonate READY")
print("- calibrated profiles can sample camera-vs-sight angular and line-offset error with optional debug rays")
print("- latest 2026-08-27 runtime evidence remains authoritative: AK ADS visual acceptance is still REJECTED")
print("STATUS: ADS VALIDATION ARCHITECTURE SOURCE-CODED; exact per-weapon sight sockets/offsets and UE 5.8 visual acceptance remain pending")
