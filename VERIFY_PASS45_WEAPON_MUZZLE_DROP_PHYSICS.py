#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
WEAPON_CPP = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCWeaponBase.cpp"
WEAPON_H = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public" / "OCWeaponBase.h"
LAUNCHER_CPP = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCAntiArmorLauncher.cpp"
LAUNCHER_H = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public" / "OCAntiArmorLauncher.h"
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


weapon_cpp = read(WEAPON_CPP)
weapon_h = read(WEAPON_H)
launcher_cpp = read(LAUNCHER_CPP)
launcher_h = read(LAUNCHER_H)
tz = read(TZ)

# 2026-08-26 runtime evidence showed tracers/muzzle flash below the visible barrel because the old code
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

# The canonical TZ must retain the latest factual runtime authority while this source work remains unverified.
req("RUNTIME REJECTED 2026-08-26" in tz,
    "canonical Pass45 TZ lost the latest 2026-08-26 runtime rejection")

if errors:
    print("PASS45 WEAPON MUZZLE + DROP PHYSICS: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 WEAPON MUZZLE + DROP PHYSICS: PASS")
print("- base hitscan keeps view-ray hit authority but muzzle/tracer/audio presentation resolves from production weapon")
print("- deliberate player drops enable authority gravity/collision/rigid-body simulation")
print("- anti-armor projectile/FX/audio originate from production muzzle and ammo commits only after spawn")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 build, drop settling, muzzle alignment and rendered firing remain authoritative")
