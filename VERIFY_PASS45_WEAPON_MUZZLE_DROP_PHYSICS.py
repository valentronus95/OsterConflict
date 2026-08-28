#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
WEAPON_CPP = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCWeaponBase.cpp"
WEAPON_H = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public" / "OCWeaponBase.h"
CHARACTER_CPP = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCCharacter.cpp"
CHARACTER_H = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public" / "OCCharacter.h"
LAUNCHER_CPP = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCAntiArmorLauncher.cpp"
LAUNCHER_H = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public" / "OCAntiArmorLauncher.h"
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


weapon_cpp = read(WEAPON_CPP)
weapon_h = read(WEAPON_H)
character_cpp = read(CHARACTER_CPP)
character_h = read(CHARACTER_H)
launcher_cpp = read(LAUNCHER_CPP)
launcher_h = read(LAUNCHER_H)
tz = read(TZ)
latest_runtime_evidence = read(LATEST_RUNTIME_EVIDENCE)

# Historical runtime evidence showed tracers/muzzle flash below the visible barrel because the old code
# reused the player-camera trace origin for both hit authority and presentation. Keep camera/view-ray hit
# authority, but require presentation to reconcile to the visible production weapon.
for needle in (
    "ResolvePresentationMuzzleOrigin",
    'FName ProductionTag(TEXT("OC_ProductionWeaponVisual"))',
    "Component->Bounds",
):
    req(needle in weapon_h, f"production muzzle resolver contract missing: {needle}")

for needle in (
    "const FVector PresentationMuzzleOrigin = ResolvePresentationMuzzleOrigin(TraceOrigin, SafeDirection);",
    "LineTraceSingleByChannel(PelletHit, TraceOrigin, TraceEnd",
    "MulticastFireTraceFX(PresentationMuzzleOrigin, RepresentativeTraceEnd",
    "DetectEnvironmentAt(PresentationMuzzleOrigin)",
    "MulticastShotAudio(PresentationMuzzleOrigin, RepresentativeTraceEnd",
):
    req(needle in weapon_cpp, f"base weapon muzzle/aim reconciliation missing: {needle}")

req("MulticastFireTraceFX(TraceOrigin, RepresentativeTraceEnd" not in weapon_cpp,
    "base weapon reverted to camera-origin muzzle/tracer presentation")
req("DetectEnvironmentAt(TraceOrigin)" not in weapon_cpp,
    "base weapon reverted to camera-origin shot-audio presentation")

# Pass45 now physically retires the second Character-side held-input recoil timer. Recoil, crosshair expansion
# and camera shake are all derived from the one server-confirmed shot multicast owned by AOCWeaponBase.
for needle in (
    "virtual void Tick(float DeltaSeconds) override;",
    "ApplyConfirmedLocalShotRecoil",
    "RecoverConfirmedLocalShotRecoil",
    "ConfirmedLocalRecoilPitchOffset",
    "ConfirmedLocalRecoilYawOffset",
    "GetConfirmedLocalRecoilPitchOffset",
):
    req(needle in weapon_h, f"confirmed-shot recoil state contract missing: {needle}")

for needle in (
    "PrimaryActorTick.bCanEverTick = true",
    "RecoverConfirmedLocalShotRecoil(DeltaSeconds)",
    "ApplyConfirmedLocalShotRecoil();",
    "LastConfirmedLocalShotTime",
    "ConfirmedRecoilRecoveryDelay",
    "ConfirmedRecoilRecoverySpeed",
    "LocalShooter->NotifyConfirmedWeaponShotPresentation();",
    "CadenceTolerance",
):
    req(needle in weapon_cpp, f"confirmed-shot recoil/cadence implementation missing: {needle}")

for needle in (
    "NotifyConfirmedWeaponShotPresentation();",
    "CurrentWeapon->GetConfirmedLocalRecoilPitchOffset()",
    "ClientStartCameraShake",
):
    req(needle in character_h + character_cpp,
        f"Character confirmed-shot presentation migration missing: {needle}")

for retired in (
    "LocalFireFeedbackTimerHandle",
    "StartLocalFireFeedback",
    "StopLocalFireFeedback",
    "ApplyLocalShotFeedback",
    "RecoverLocalRecoil",
    "bLocalFireHeld",
    "CurrentRecoilPitchOffset",
    "CurrentRecoilYawOffset",
    "LastLocalShotTime",
):
    req(retired not in character_h + character_cpp,
        f"retired Character held-input recoil path still exists: {retired}")

req("ShouldNeutralizeLegacyLocalRecoil" not in weapon_h + weapon_cpp,
    "temporary local-recoil neutralization shim survived after Character feedback retirement")
req("This multicast exists only after TryFireServer accepted a factual shot" in weapon_cpp,
    "confirmed-shot presentation source no longer documents factual-shot ownership")

# A deliberate player drop must be an authority-simulated rigid body. Static rack pickups remain authored
# placements and are not globally forced into physics merely because bIsWorldPickup is true.
for needle in (
    "SetRootComponent(WeaponMesh)",
    "WeaponRoot->SetupAttachment(WeaponMesh)",
    "WeaponRoot->SetAbsolute(false, false, true)",
    "SetReplicateMovement(true)",
    "WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics)",
    "WeaponMesh->SetSimulatePhysics(true)",
    "WeaponMesh->SetEnableGravity(true)",
    "WeaponMesh->WakeAllRigidBodies()",
):
    req(needle in weapon_cpp, f"dropped-weapon rigid-body contract missing: {needle}")

req("DropToWorldServer" in weapon_cpp and "InheritedVelocity" in weapon_cpp,
    "dropped weapon does not preserve basic carrier motion on release")

# The anti-armor launcher had its own camera-origin projectile path and emitted no shot audio. It must use
# the same production muzzle truth, and a failed projectile spawn must not consume ammo or pretend a shot happened.
for needle in (
    "ResolvePresentationMuzzleOrigin(TraceOrigin, Dir)",
    "AOCAntiArmorProjectile* Projectile",
    "if (!Projectile)",
    "--AmmoInMagazine;",
    "MulticastFireTraceFX(MuzzleOrigin, PresentationEnd",
    "DetectEnvironmentAt(MuzzleOrigin)",
    "MulticastShotAudio(MuzzleOrigin, PresentationEnd",
    "PASS45_LAUNCHER_CONFIRMED_SHOT",
    "PASS45_LAUNCHER_PRODUCTION_VISUAL_FAIL",
):
    req(needle in launcher_cpp, f"anti-armor confirmed-shot/muzzle contract missing: {needle}")

spawn_pos = launcher_cpp.find("AOCAntiArmorProjectile* Projectile")
ammo_pos = launcher_cpp.find("--AmmoInMagazine;")
req(spawn_pos >= 0 and ammo_pos > spawn_pos,
    "anti-armor launcher consumes ammo before confirming projectile spawn")
req("LauncherAudioEventCounter" in launcher_h,
    "anti-armor confirmed shot has no dedicated presentation event sequence")
req("TraceOrigin+Dir*90.0f" not in launcher_cpp and "MulticastFireTraceFX(TraceOrigin" not in launcher_cpp,
    "anti-armor launcher regressed to camera-origin projectile/FX")

# Runtime authority must follow the newest evidence pack, never a hard-coded historical rejection date.
req("RUNTIME REJECTED 2026-08-27" in tz,
    "canonical Pass45 TZ lost the latest 2026-08-27 runtime rejection")
for needle in ("RUNTIME REJECTED", "2026-08-27"):
    req(needle in latest_runtime_evidence,
        f"latest Pass45 runtime evidence lost required authority marker: {needle}")

if errors:
    print("PASS45 WEAPON FIRING + MUZZLE + DROP PHYSICS: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 WEAPON FIRING + MUZZLE + DROP PHYSICS: PASS")
print("- base hitscan keeps view-ray hit authority but muzzle/tracer/audio presentation resolves from production weapon")
print("- Character held-input recoil timer/state is physically retired; confirmed shots own recoil/crosshair/camera shake")
print("- bounded server cadence tolerance prevents tiny timer jitter from killing automatic fire early")
print("- deliberate player drops enable authority gravity/collision/rigid-body simulation")
print("- anti-armor projectile/FX/audio originate from production muzzle and ammo commits only after spawn")
print("- latest factual runtime authority remains RUNTIME REJECTED 2026-08-27")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 build, recoil feel, drop settling, muzzle alignment and rendered firing remain authoritative")
