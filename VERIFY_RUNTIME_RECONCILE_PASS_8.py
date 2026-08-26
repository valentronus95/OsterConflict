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

# Pass 7 frontend/vehicle truth survives, but Pass45 supersedes the obsolete all-exact 11-weapon rack gate.
require(t["frontend"], 'SettingsPanel->SetBrushColor(FLinearColor(0.045f, 0.055f, 0.066f, 1.0f));', "opaque settings")
require(t["deploy"], '"DeployEnterBattle", "У БІЙ"', "single START semantics")
require(t["loading"], 'Scrim->SetBrushColor(FLinearColor(0.006f, 0.009f, 0.012f, 1.0f));', "opaque deployment loading")
require(t["museum_guard"], 'PASS7_MUSEUM_BASES_READY', "Museum BASE runtime marker")
for marker in (
    'PASS7_PRODUCTION_VEHICLES_READY',
    'PASS7_PRODUCTION_VEHICLE_RUNTIME_FAIL',
    'PASS45_REQUIRED_AVAILABLE_WEAPONS_READY',
    'PASS45_REQUIRED_AVAILABLE_WEAPON_RUNTIME_FAIL',
    'PASS45_EXACT_WEAPON_CONTENT_GAP',
    'validation_only=1 mutation=0',
):
    require(t["vehicle_validator"], marker, "current fail-closed runtime evidence")
for stale in ('PASS7_PRODUCTION_WEAPONS_READY', 'PASS7_PRODUCTION_WEAPON_RUNTIME_FAIL'):
    forbid(t["vehicle_validator"], stale, "obsolete all-exact weapon runtime gate")

for marker in (
    'PASS7_PRODUCTION_VEHICLES_READY',
    'PASS45_REQUIRED_AVAILABLE_WEAPONS_READY',
    'PASS36_WEAPON_MATERIAL_AUDIT_READY',
    'PASS7_MUSEUM_BASES_READY',
):
    require(t["launcher"], marker, "launcher runtime evidence gate")
for stale in ('PASS7_PRODUCTION_WEAPONS_READY', 'PASS7_PRODUCTION_WEAPON_RUNTIME_FAIL'):
    forbid(t["launcher"], stale, "obsolete exact-only launcher weapon gate")

# Compact HUD recovered from Pass 6.
for marker in (
    'constexpr float MinimapOuterSize = 184.0f',
    'constexpr float MinimapInnerSize = 172.0f',
    'MarkerFont.Size = 15',
):
    require(t["minimap"], marker, "compact minimap")
for marker in ('FVector2D(360.0f, 190.0f)', 'Messages.Num() - 5', 'EKeys::Y', 'EKeys::U'):
    require(t["chat"], marker, "compact Y/U chat")

# Foliage remains HISM-based, collision-free and incremental; later passes may make it sparser.
grid = re.search(r'constexpr\s+float\s+GridStep\s*=\s*([0-9.]+)f\s*;', t["foliage"])
batch = re.search(r'constexpr\s+int32\s+CellsPerBatch\s*=\s*(\d+)\s*;', t["foliage"])
if not grid or not 900.0 <= float(grid.group(1)) <= 5000.0:
    raise SystemExit("PASS 8 FAIL: foliage grid is missing or outside the supported incremental range")
if not batch or not 1 <= int(batch.group(1)) <= 48:
    raise SystemExit("PASS 8 FAIL: foliage batch is missing or exceeds the performance ceiling")
require(t["foliage"], 'UHierarchicalInstancedStaticMeshComponent', "HISM foliage")
require(t["foliage"], 'SetCollisionEnabled(ECollisionEnabled::NoCollision)', "foliage collision")
require(t["foliage"], 'SetCastShadow(false)', "foliage shadows")

# Frontend travel must not toggle persistent viewport rendering off.
require(t["viewport"], 'const bool bStartupShell = !bHasGameplayPawn', "pawn-less startup shell")
require(t["viewport"], 'SetWorldRenderingSuppressed(false)', "world rendering remains enabled")
require(t["viewport"], 'R13_MenuWorldBlocker', "travel blocker")
require(t["viewport"], 'R13_MenuBackground', "travel background")
for marker in (
    'SetWorldRenderingSuppressed(bFrontendMenu)',
    'SetWorldRenderingSuppressed(bPreGamePresentationVisible)',
    'SetWorldRenderingSuppressed(true)',
):
    forbid(t["viewport"], marker, "persistent viewport suppression")

# Production vehicle proxies become inert; M2 orientation/muzzle follows the real imported mesh.
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

# StaticMesh production weapons participate in first-person presentation; skeletal animation stays separated.
require(t["fp_h"], 'UPrimitiveComponent* FindProductionWeaponVisual', "mesh-agnostic FP header")
require(t["fp_h"], 'FindProductionSkeletalWeaponVisual', "skeletal animation helper")
require(t["fp"], 'Weapon.GetComponents<UPrimitiveComponent>', "mesh-agnostic FP lookup")
require(t["fp"], 'Cast<UStaticMeshComponent>(ProductionVisual)', "StaticMesh FP path")
require(t["fp"], 'Weapon->SetActorRelativeLocation(WeaponLocation)', "weapon actor FP pose")
require(t["fp"], 'Weapon->SetActorRelativeRotation(WeaponRotation)', "weapon actor FP rotation")

# Muzzle/tracer resolves the actual firing weapon.
for marker in (
    'ResolveFiringWeapon', 'TActorIterator<AOCCharacter>', 'Character->GetCurrentWeapon()',
    'ResolveWeaponMuzzle', 'TryResolveSocketMuzzle', 'TryResolveBoundsMuzzle',
    'FMath::Min(DistanceToEnd, 900.0f)', 'const FVector VisualStart = ResolveWeaponMuzzle',
    'const FVector VisualMuzzle = ResolveWeaponMuzzle',
):
    require(t["fx"], marker, "actual firing-weapon FX")
forbid(t["fx"], 'ResolveLocalWeaponMuzzle', "obsolete local-only FX resolver")

# Recovered compatibility owner retains legacy cleanup/axis-normalization behavior.
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

print("RUNTIME RECONCILE PASS 8 + PASS45 WEAPON TRUTH SOURCE CONTRACT PASS")
print("- Pass 7 frontend/Museum/production vehicle contracts remain intact")
print("- Pass45 required-available rack replaces impossible all-exact weapon readiness without relabelling fallback production")
print("- compact minimap/chat, bounded foliage, vehicle proxy and first-person weapon contracts remain intact")
print("STATUS: SOURCE VERIFIED ONLY; UE 5.8 compile/runtime acceptance still required")
