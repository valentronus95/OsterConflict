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
    "weapon_catalog": SRC / "Private" / "OCPass45WeaponCatalogSpawnSubsystem.cpp",
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

# Pass 7 frontend/vehicle truth survives. GAME_RECOVERY now gives complete weapon-rack validation to the
# 23-entry catalog owner instead of the vehicle validator or the retired smaller required-available rack gate.
settings_brush = re.search(
    r'SettingsPanel->SetBrushColor\(FLinearColor\([^,]+,[^,]+,[^,]+,\s*([0-9.]+)f\)\);',
    t["frontend"],
)
if not settings_brush or float(settings_brush.group(1)) < 0.95:
    raise SystemExit("PASS 8 FAIL: settings panel is not effectively opaque (alpha < 0.95)")
require(t["deploy"], '"DeployEnterBattle", "У БІЙ"', "single START semantics")
require(t["loading"], 'Scrim->SetBrushColor(FLinearColor(0.006f, 0.009f, 0.012f, 1.0f));', "opaque deployment loading")
require(t["loading"], 'Widget->SetLoadingProgress(0.0f);', "honest zero-percent deployment start")
require(t["museum_guard"], 'PASS7_MUSEUM_BASES_READY', "Museum BASE runtime marker")

for marker in (
    'PASS7_PRODUCTION_VEHICLES_READY',
    'PASS7_PRODUCTION_VEHICLE_RUNTIME_FAIL',
    'validation_owner=vehicles_only',
):
    require(t["vehicle_validator"], marker, "vehicle-only fail-closed runtime evidence")
for stale in (
    'PASS7_PRODUCTION_WEAPONS_READY',
    'PASS7_PRODUCTION_WEAPON_RUNTIME_FAIL',
    'PASS45_REQUIRED_AVAILABLE_WEAPONS_READY',
    'PASS45_REQUIRED_AVAILABLE_WEAPON_RUNTIME_FAIL',
):
    forbid(t["vehicle_validator"], stale, "weapon validation must not return to vehicle owner")

for marker in (
    'const FWeaponCatalogEntry WeaponCatalog[]',
    'constexpr int32 CoreRackEntryCount = 7',
    'PASS45_COMPLETE_WEAPON_RACK_READY',
    'PASS45_COMPLETE_WEAPON_CATALOG_VISUAL_READY',
    'PASS45_COMPLETE_WEAPON_CATALOG_VISUAL_GAP',
    'duplicate_weapon_ids=0',
    'wrong_identity_substitution=0',
):
    require(t["weapon_catalog"], marker, "complete 23-entry weapon catalog owner")

for marker in (
    'PASS7_PRODUCTION_VEHICLES_READY',
    'PASS45_COMPLETE_WEAPON_CATALOG_VISUAL_READY',
    'PASS45_COMPLETE_WEAPON_CATALOG_VISUAL_GAP',
    'PASS36_WEAPON_MATERIAL_AUDIT_READY',
    'PASS7_MUSEUM_BASES_READY',
):
    require(t["launcher"], marker, "launcher runtime evidence gate")
for stale in (
    'PASS7_PRODUCTION_WEAPONS_READY',
    'PASS7_PRODUCTION_WEAPON_RUNTIME_FAIL',
    'PASS45_REQUIRED_AVAILABLE_WEAPONS_READY',
    'PASS45_REQUIRED_AVAILABLE_WEAPON_RUNTIME_FAIL',
):
    forbid(t["launcher"], stale, "obsolete smaller weapon runtime gate")

# Compact HUD recovered from Pass 6.
for marker in (
    'constexpr float MinimapOuterSize = 184.0f',
    'constexpr float MinimapInnerSize = 172.0f',
    'MarkerFont.Size = 15',
):
    require(t["minimap"], marker, "compact minimap")
for marker in ('FVector2D(360.0f, 190.0f)', 'Messages.Num() - 5', 'EKeys::Y', 'EKeys::U'):
    require(t["chat"], marker, "compact Y/U chat")

# Block 0 supersedes the old single GridStep/CellsPerBatch constants. Both profiles cover the same compact
# Oster map; LowCPU remains cheaper through the coarser grid/culls while generation stays incremental.
full_grid = re.search(r'constexpr\s+float\s+FullGridStepCm\s*=\s*([0-9.]+)f\s*;', t["foliage"])
low_grid = re.search(r'constexpr\s+float\s+LowCPUGridStepCm\s*=\s*([0-9.]+)f\s*;', t["foliage"])
full_batch = re.search(r'constexpr\s+int32\s+FullCellsPerBatch\s*=\s*(\d+)\s*;', t["foliage"])
low_batch = re.search(r'constexpr\s+int32\s+LowCPUCellsPerBatch\s*=\s*(\d+)\s*;', t["foliage"])
if not full_grid or not 1000.0 <= float(full_grid.group(1)) <= 5000.0:
    raise SystemExit("PASS 8 FAIL: Full foliage grid is missing or outside the supported incremental range")
if not low_grid or not 1500.0 <= float(low_grid.group(1)) <= 5000.0:
    raise SystemExit("PASS 8 FAIL: LowCPU foliage grid is missing or outside the supported incremental range")
if float(low_grid.group(1)) <= float(full_grid.group(1)):
    raise SystemExit("PASS 8 FAIL: LowCPU foliage grid must remain coarser than Full")
if not full_batch or not 1 <= int(full_batch.group(1)) <= 32:
    raise SystemExit("PASS 8 FAIL: Full foliage batch is missing or exceeds the performance ceiling")
if not low_batch or not 1 <= int(low_batch.group(1)) <= 48:
    raise SystemExit("PASS 8 FAIL: LowCPU foliage batch is missing or exceeds the performance ceiling")
for marker in (
    'UHierarchicalInstancedStaticMeshComponent',
    'SetCollisionEnabled(ECollisionEnabled::NoCollision)',
    'SetCastShadow(false)',
    'ActiveGridStep = bLowCPUProfile ? LowCPUGridStepCm : FullGridStepCm',
    'ActiveCellsPerBatch = bLowCPUProfile ? LowCPUCellsPerBatch : FullCellsPerBatch',
    'full_playable_bounds=1',
):
    require(t["foliage"], marker, "Block0 HISM foliage")

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

print("RUNTIME RECONCILE PASS 8 + GAME_RECOVERY WEAPON CATALOG SOURCE CONTRACT PASS")
print("- Pass 7 frontend/Museum/production vehicle contracts remain intact")
print("- settings panel remains effectively opaque without pinning one obsolete RGB shade")
print("- complete 23-entry weapon catalog is the single runtime rack/visual validation owner")
print("- acceptance launcher requires complete-catalog READY and rejects complete-catalog GAP")
print("- compact minimap/chat, Block0 profile-bounded foliage, vehicle proxy and first-person weapon contracts remain intact")
print("STATUS: SOURCE VERIFIED ONLY; UE 5.8 compile/runtime acceptance still required")
