#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private"
PUB = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public"


def read(path: Path) -> str:
    return path.read_text(encoding="utf-8", errors="replace")


pickup = read(SRC / "OCPickupGunTruck.cpp")
armed = read(SRC / "OCArmedVehicleBase.cpp")
armed_h = read(PUB / "OCArmedVehicleBase.h")
character = read(SRC / "OCCharacter.cpp")
errors = []


def req(cond: bool, msg: str) -> None:
    if not cond:
        errors.append(msg)


for needle in (
    "/Game/Production/Vehicles/HMMWV/SM_HMMWV_UA.SM_HMMWV_UA",
    "/Game/Production/Weapons/M2/SM_M2_Browning.SM_M2_Browning",
    "PASS45_HMMWV_PROPORTIONAL_VISUAL_READY",
    "PASS45_M2_AUTHORED_PIVOT_READY",
):
    req(needle in pickup, f"missing production HMMWV/M2 contract: {needle}")

# Exact HMMWV identity is fail-closed. A missing HMMWV shell may not become a pickup, and a missing M2
# may not be represented by another machine gun. This protects the user's one-owner/no-duplicate identity rule.
for needle in (
    "const bool bRequiresHMMWV = ShouldUseHMMWVProductionVisual();",
    "!bUsingProductionVehicle && Chassis && !bRequiresHMMWV",
    "PASS45_HMMWV_PRODUCTION_VISUAL_GAP",
    "pickup_substitution=0",
    "primitive_chassis_visible=0",
    "!bUsingMountedGunAsset && !bRequiresHMMWV",
    "PASS45_HMMWV_M2_PRODUCTION_VISUAL_GAP",
    "other_gun_substitution=0",
):
    req(needle in pickup, f"fail-closed HMMWV/M2 identity guard missing: {needle}")

# 2026-08-27 runtime rejection: exact M2 floated above the HMMWV after bounds/longest-axis correction.
# The imported M2 has an authored receiver/mount pivot; exact production must use it unchanged.
for needle in (
    "AddAuthoredPivotTurretVisual(this, M2Parent, M2, 165.0f",
    "Visual->SetRelativeLocation(FVector::ZeroVector);",
    "Visual->SetRelativeRotation(FRotator::ZeroRotator);",
    "bounds_recenter=0",
    "longest_axis_guess=0",
    "m2_authored_pivot=1",
):
    req(needle in pickup, f"exact M2 authored-pivot guard missing: {needle}")

exact_m2_call = pickup.find("AddAuthoredPivotTurretVisual(this, M2Parent, M2, 165.0f")
req(exact_m2_call >= 0, "exact M2 does not use authored-pivot helper")
req("AddGroundedTurretVisual(this, M2Parent, M2, 165.0f" not in pickup,
    "exact M2 regressed to bounds-grounding/longest-axis heuristic")

for needle in (
    'TurretPivot = CreateDefaultSubobject<USceneComponent>(TEXT("TurretPivot"))',
    'BarrelPivot = CreateDefaultSubobject<USceneComponent>(TEXT("BarrelPivot"))',
    "BarrelPivot->SetupAttachment(TurretPivot)",
    'GunnerCameraPivot = CreateDefaultSubobject<USceneComponent>(TEXT("GunnerCameraPivot"))',
    "GunnerCameraPivot->SetupAttachment(TurretPivot)",
    "MuzzlePoint->SetupAttachment(BarrelPivot)",
):
    req(needle in armed, f"M2 hierarchy source contract missing: {needle}")

req("USceneComponent* M2Parent = BarrelPivot.Get();" in pickup,
    "production M2 is not owned by the pitch hierarchy")
req("if (!M2Parent) M2Parent = TurretPivot.Get();" in pickup,
    "M2 hierarchy has no safe yaw-root fallback")
req("DisableVisualProxy(TurretBaseMesh);" in pickup and "DisableVisualProxy(BarrelMesh);" in pickup,
    "primitive turret/barrel presentation is visible alongside production M2")

for needle in (
    "bContinuousTurretYaw = true;",
    "FMath::UnwindDegrees(RelativeYaw)",
    "bContinuousTurretYaw ? 1000000.0f : MaxTurretYaw",
    "IsTurretYawContinuous() const",
):
    req(needle in pickup + armed + armed_h, f"continuous HMMWV yaw contract missing: {needle}")
req("GetMaxTurretYawLimit()" in character,
    "gunner input does not consume the vehicle-specific yaw policy")

for needle in (
    "GetGunnerCameraWorldLocation() - FVector(0.0f, 0.0f, 64.0f)",
    "GunnerCameraPivot->GetComponentLocation()",
    "PASS45_HMMWV_M2_HIERARCHY_READY",
    "camera_owner=GunnerCameraPivot",
    "continuous_yaw=1",
    "hard_stop=0",
):
    req(needle in pickup + armed, f"turret-owned gunner camera contract missing: {needle}")

for needle in (
    "PASS45_HMMWV_M2_SHIELD_CONTENT_GAP",
    "separate_authored_shield=0",
    "primitive_shield_fallback=0",
):
    req(needle in pickup, f"shield content-gap truth marker missing: {needle}")
req("ProductionM2Shield" not in pickup and "M2ShieldCube" not in pickup,
    "unverified/fake M2 shield presentation was introduced")

if errors:
    print("PASS45 HMMWV M2 HIERARCHY: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 HMMWV M2 HIERARCHY: PASS")
print("- real HMMWV and authored M2 remain production owners")
print("- explicit HMMWV/M2 identity now fails closed instead of substituting a pickup or another gun")
print("- exact M2 uses its authored receiver/mount pivot; bounds recenter and longest-axis guessing are forbidden")
print("- TurretPivot owns yaw; BarrelPivot owns pitch/M2/muzzle; GunnerCameraPivot owns the mounted view")
print("- HMMWV yaw is continuous and no longer limited by the generic +/-170-degree stop")
print("- separate authored shield is explicitly NOT claimed; no primitive shield fallback is allowed")
print("STATUS: SOURCE CONTRACT ONLY; UE 5.8 runtime acceptance remains required")
