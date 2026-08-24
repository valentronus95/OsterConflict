#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS36 VERIFY FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        raise SystemExit(f"PASS36 VERIFY FAIL: {label}: missing {needle!r}")


def forbid(text: str, needle: str, label: str) -> None:
    if needle in text:
        raise SystemExit(f"PASS36 VERIFY FAIL: {label}: forbidden {needle!r}")


dense_h = read(SRC / "Public" / "OCDenseGroundFoliageSubsystem.h")
dense = read(SRC / "Private" / "OCDenseGroundFoliageSubsystem.cpp")
foliage_guard_h = read(SRC / "Public" / "OCFoliageRuntimeGuardSubsystem.h")
foliage_guard = read(SRC / "Private" / "OCFoliageRuntimeGuardSubsystem.cpp")
weapon_h = read(SRC / "Public" / "OCRealWeaponFallbackSubsystem.h")
weapon = read(SRC / "Private" / "OCRealWeaponFallbackSubsystem.cpp")

for needle in (
    "float PopulationMinX",
    "float PopulationMaxX",
    "float ActiveGridStep",
    "int32 ActiveCellsPerBatch",
    "bool bLowCPUProfile",
):
    require(dense_h, needle, "profile-aware foliage state")

for needle in (
    "LowCPUHalfExtentCm = 10000.0f",
    "LowCPUGridStepCm = 1500.0f",
    "LowCPUCellsPerBatch = 8",
    "LowCPUGrassCullEndCm = 8500",
    'World.URL.GetOption(TEXT("PerfProfile="), TEXT(""))',
    "PopulationMinX = Museum.X - LowCPUHalfExtentCm",
    "PopulationMaxX = Museum.X + LowCPUHalfExtentCm",
    "PopulationMinY = Museum.Y - LowCPUHalfExtentCm",
    "PopulationMaxY = Museum.Y + LowCPUHalfExtentCm",
    "PASS36_LOWCPU_FOLIAGE_SCOPE_READY",
    "PASS36_LOWCPU_FOLIAGE_COMPLETE",
    "PASS42_LOWCPU_FOLIAGE_SCOPE_EXPANDED",
):
    require(dense, needle, "bounded LowCPU foliage")

require(foliage_guard_h, "ValidateDenseFoliage(int32 MinGrassInstances", "profile-aware foliage validation signature")
for needle in (
    "const int32 MinGrassInstances = bLowCPU ? 48 : 250",
    "PASS36_LOWCPU_FOLIAGE_RUNTIME_READY",
    "full_sector_population=0",
    "ValidationAccumulator < 0.25f",
    "PASS42_FOLIAGE_GUARD_THROTTLED_READY",
):
    require(foliage_guard, needle, "LowCPU foliage runtime guard")

# Pass 44 supersedes the old grey BasicShapeMaterial "repair". A missing/default slot is content evidence,
# not something the runtime may paint over and then call production-ready.
require(weapon_h, "AuditAndRepairWeaponMaterials", "weapon material audit state")
for needle in (
    "IsMissingOrDefaultMaterial",
    "/Engine/EngineMaterials/DefaultMaterial",
    "/Engine/BasicShapes/BasicShapeMaterial",
    'AuthoredMaterialGapTag(TEXT("OC_WeaponAuthoredMaterialGap"))',
    "PASS44_WEAPON_AUTHORED_MATERIAL_GAP",
    "PASS44_WEAPON_AUTHORED_MATERIAL_READY",
    "PASS44_WEAPON_RACK_AUTHORED_MATERIAL_GAP",
    "basicshape_repair=0",
    "PASS36_WEAPON_MATERIAL_AUDIT_READY",
    'RuntimeBaseRackTag(TEXT("OC_RuntimeBaseWeaponRack"))',
    "Component->SetCastShadow(false)",
    "reason=material_gap_audited",
):
    require(weapon, needle, "truth-only weapon material audit")

for forbidden in (
    "MaterialRecoveryBase",
    "UMaterialInstanceDynamic::Create",
    "PASS36_WEAPON_MATERIAL_RECOVERED",
    "Component->SetMaterial(Slot",
):
    forbid(weapon_h + weapon, forbidden, "Pass 44 must not disguise missing authored materials")

require(weapon, "HasProductionVisual(*Weapon)", "production visual preservation")
require(weapon, "ApplyRealFallback", "existing real-mesh fallback preservation")

print("WEAPON MATERIAL + LOWCPU PERFORMANCE PASS 36/42/44 SOURCE CONTRACT PASS")
print("- LowCPU foliage stays bounded around the museum/BASE and cannot progressively fill the whole sector")
print("- foliage acceptance scans are throttled and stop after convergence")
print("- missing/default weapon materials are reported as authored-content gaps, never painted grey with BasicShapeMaterial")
print("- a fully audited rack with material gaps stops its scan immediately instead of repeating for the whole budget")
print("- generic real-mesh fallback remains playable but is not production-art acceptance")
print("STATUS: SOURCE VERIFIED; actual UE 5.8 FPS/material acceptance remains runtime-only")
