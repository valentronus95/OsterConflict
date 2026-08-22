#!/usr/bin/env python3
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"

FILES = {
    "frontend": SRC / "Private" / "OCR13FrontendMenuSubsystem.cpp",
    "deploy": SRC / "Private" / "OCR13DeploymentPresentationSubsystem.cpp",
    "loading": SRC / "Private" / "OCDeploymentLoadingSubsystem.cpp",
    "museum_guard": SRC / "Private" / "OCMuseumSpawnGuardSubsystem.cpp",
    "vehicle_validator": SRC / "Private" / "OCProductionVehicleRuntimeValidationSubsystem.cpp",
    "minimap": SRC / "Private" / "OCMinimapSubsystem.cpp",
    "chat": SRC / "Private" / "OCRuntimeChatSubsystem.cpp",
    "foliage": SRC / "Private" / "OCDenseGroundFoliageSubsystem.cpp",
    "viewport": SRC / "Private" / "OCR13UIViewportStabilizerSubsystem.cpp",
    "btr": SRC / "Private" / "OCBTR.cpp",
    "pickup": SRC / "Private" / "OCPickupGunTruck.cpp",
    "fp_h": SRC / "Public" / "OCFirstPersonWeaponPresentationSubsystem.h",
    "fp": SRC / "Private" / "OCFirstPersonWeaponPresentationSubsystem.cpp",
    "fx": SRC / "Private" / "OCTransientVisualFX.cpp",
    "recovered": SRC / "Private" / "OCRuntimeAcceptancePass6Subsystem.cpp",
    "launcher": ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd",
}


def read(name: str) -> str:
    path = FILES[name]
    if not path.is_file():
        raise SystemExit(f"PASS 8 FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        raise SystemExit(f"PASS 8 FAIL: {label}: missing {needle!r}")


def forbid(text: str, needle: str, label: str) -> None:
    if needle in text:
        raise SystemExit(f"PASS 8 FAIL: {label}: forbidden {needle!r}")


t = {name: read(name) for name in FILES}

# Pass 7 must survive reconciliation.
require(t["frontend"], 'SettingsPanel->SetBrushColor(FLinearColor(0.045f, 0.055f, 0.066f, 1.0f));', "opaque settings")
require(t["deploy"], '"DeployEnterBattle", "У БІЙ"', "single START semantics")
require(t["loading"], 'Scrim->SetBrushColor(FLinearColor(0.006f, 0.009f, 0.012f, 1.0f));', "opaque deployment loading")
require(t["museum_guard"], 'PASS7_MUSEUM_BASES_READY', "Museum BASE runtime marker")
for marker in (
    'PASS7_PRODUCTION_VEHICLES_READY',
    'PASS7_PRODUCTION_WEAPONS_READY',
    'PASS7_PRODUCTION_VEHICLE_RUNTIME_FAIL',
    'PASS7_PRODUCTION_WEAPON_RUNTIME_FAIL',
):
    require(t["vehicle_validator"], marker, "Pass 7 fail-closed runtime evidence")
for marker in (
    'PASS7_PRODUCTION_VEHICLES_READY',
    'PASS7_PRODUCTION_WEAPONS_READY',
    'PASS7_MUSEUM_BASES_READY',
):
    require(t["launcher"], marker, "launcher runtime evidence gate")

# Compact HUD recovered from Pass 6.
for marker in (
    'constexpr float MinimapOuterSize = 184.0f',
    'constexpr float MinimapInnerSize = 172.0f',
    'MarkerFont.Size = 15',
):
    require(t["minimap"], marker, "compact minimap")
for marker in (
    'FVector2D(360.0f, 190.0f)',
    'Messages.Num() - 5',
    'EKeys::Y',
    'EKeys::U',
):
    require(t["chat"], marker, "compact Y/U chat")

# Denser grass remains incremental and bounded.
require(t["foliage"], 'constexpr float GridStep = 900.0f', "grass density")
require(t["foliage"], 'RandomStream.RandRange(4, 6)', "grass clumps")
batch = re.search(r'constexpr\s+int32\s+CellsPerBatch\s*=\s*(\d+)\s*;', t["foliage"])
if not batch or not 1 <= int(batch.group(1)) <= 96:
    raise SystemExit("PASS 8 FAIL: foliage batch is missing or exceeds non-blocking ceiling")
require(t["foliage"], 'SetCollisionEnabled(ECollisionEnabled::NoCollision)', "foliage collision")

# Frontend travel must not toggle the persistent viewport render flag off.
require(t["viewport"], 'const bool bStartupShell = !bHasGameplayPawn', "pawn-less startup shell")
require(t["viewport"], 'SetWorldRenderingSuppressed(false)', "world rendering remains enabled")
require(t["viewport"], 'R13_MenuWorldBlocker', "travel blocker")
require(t["viewport"], 'R13_MenuBackground', "travel background")
forbidden_viewport = (
    'SetWorldRenderingSuppressed(bFrontendMenu)',
    'SetWorldRenderingSuppressed(bPreGamePresentationVisible)',
    'SetWorldRenderingSuppressed(true)',
)
for marker in forbidden_viewport:
    forbid(t["viewport"], marker, "persistent viewport suppression")

# Production vehicle proxies become fully inert; M2 orientation/muzzle follows the real imported mesh.
for source_name in ("btr", "pickup"):
    for marker in (
        'SetCollisionEnabled(ECollisionEnabled::NoCollision)',
        'SetGenerateOverlapEvents(false)',
        'SetCanEverAffectNavigation(false)',
        'SetCastShadow(false)',
    ):
        require(t[source_name], marker, f"{source_name} inert proxy")
require(t["pickup"], 'ResolveLongAxisToForward', "M2 long-axis normalization")
require(t["pickup"], 'M2Parent = BarrelPivot', "M2 pitching parent")
require(t["pickup"], 'MuzzlePoint->SetRelativeLocation(FVector(82.5f', "M2 front-bound muzzle")
require(t["pickup"], 'DisableVisualProxy(TurretBaseMesh)', "old turret disabled")
require(t["pickup"], 'DisableVisualProxy(BarrelMesh)', "old barrel disabled")
require(t["btr"], '/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus.SM_BTR4_Bucephalus', "BTR4 production shell")

# StaticMesh production weapons participate in first-person presentation; skeletal-only animation stays separated.
require(t["fp_h"], 'UPrimitiveComponent* FindProductionWeaponVisual', "mesh-agnostic FP header")
require(t["fp_h"], 'FindProductionSkeletalWeaponVisual', "skeletal animation helper")
require(t["fp"], 'Weapon.GetComponents<UPrimitiveComponent>', "mesh-agnostic FP lookup")
require(t["fp"], 'Cast<UStaticMeshComponent>(ProductionVisual)', "StaticMesh FP path")
require(t["fp"], 'Weapon->SetActorRelativeLocation(WeaponLocation)', "weapon actor FP pose")
require(t["fp"], 'Weapon->SetActorRelativeRotation(WeaponRotation)', "weapon actor FP rotation")

# Muzzle/tracer must resolve the actual firing character/weapon, not the first local pawn.
for marker in (
    'ResolveFiringWeapon',
    'TActorIterator<AOCCharacter>',
    'Character->GetCurrentWeapon()',
    'ResolveWeaponMuzzle',
    'TryResolveSocketMuzzle',
    'TryResolveBoundsMuzzle',
    'FMath::Min(DistanceToEnd, 900.0f)',
    'const FVector VisualStart = ResolveWeaponMuzzle',
    'const FVector VisualMuzzle = ResolveWeaponMuzzle',
):
    require(t["fx"], marker, "actual firing-weapon FX")
forbid(t["fx"], 'ResolveLocalWeaponMuzzle', "obsolete local-only FX resolver")

# Recovered compatibility owner removes obsolete map-edge BASE presentation and normalizes restored static weapons.
for marker in (
    'FVector(-104000.0f, -92000.0f, 0.0f)',
    'FVector( 104000.0f,  92000.0f, 0.0f)',
    'Component->RemoveInstance(InstanceIndex)',
    'if (GameMode->IsFrontendOnlySession()) return;',
    'FQuat::FindBetweenNormals(NativeForward, FVector::ForwardVector)',
    'OC_Pass6AxisNormalized',
    'MakeHiddenStaticGeometryInert',
):
    require(t["recovered"], marker, "legacy BASE/static-weapon recovery")

print("RUNTIME RECONCILE PASS 8 SOURCE CONTRACT PASS")
print("- Pass 7 settings/loading/Museum/fail-closed runtime evidence remains intact")
print("- compact minimap/chat, denser batched foliage and travel isolation recovered")
print("- BTR/HMMWV/M2 proxy collision and orientation corrections recovered")
print("- StaticMesh first-person weapons and actual firing-weapon muzzle/tracer paths recovered")
print("- obsolete map-edge BASE geometry cleanup and static-weapon axis normalization recovered")
print("STATUS: SOURCE VERIFIED ONLY; UE 5.8 compile/runtime acceptance still required")
