#!/usr/bin/env python3
from pathlib import Path
import re

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
    "float PopulationMinY",
    "float PopulationMaxY",
    "float ActiveGridStep",
    "int32 ActiveCellsPerBatch",
    "bool bLowCPUProfile",
):
    require(dense_h, needle, "profile-aware foliage state")

# Pass45 Block 0 deliberately retired the old Museum-only LowCPU crop. Both profiles populate the same compact
# Oster bounds; LowCPU reduces density/batching/cull distance instead of deleting most of the city.
for needle in (
    "CompactMinX = -78000.0f",
    "CompactMaxX =  18000.0f",
    "CompactMinY = -12000.0f",
    "CompactMaxY =  82000.0f",
    "FullGridStepCm = 1000.0f",
    "LowCPUGridStepCm = 1500.0f",
    "FullCellsPerBatch = 32",
    "LowCPUCellsPerBatch = 48",
    "FullGrassCullEndCm = 18000",
    "LowCPUGrassCullEndCm = 14000",
    "FullPlantCullEndCm = 12000",
    "LowCPUPlantCullEndCm = 9000",
    "FullFlowerCullEndCm = 8000",
    "LowCPUFlowerCullEndCm = 6000",
    'World.URL.GetOption(TEXT("PerfProfile="), TEXT(""))',
    "PopulationMinX = CompactMinX",
    "PopulationMaxX = CompactMaxX",
    "PopulationMinY = CompactMinY",
    "PopulationMaxY = CompactMaxY",
    "ActiveGridStep = bLowCPUProfile ? LowCPUGridStepCm : FullGridStepCm",
    "ActiveCellsPerBatch = bLowCPUProfile ? LowCPUCellsPerBatch : FullCellsPerBatch",
    "PASS45_BLOCK0_FULL_MAP_GRASS_SCOPE_READY",
    "PASS45_BLOCK0_FOLIAGE_BUDGET_READY",
    "PASS45_BLOCK0_FULL_MAP_GRASS_READY",
    "full_playable_bounds=1",
    "museum_only=0",
):
    require(dense, needle, "Pass45 full-map LowCPU foliage policy")

full_grid = re.search(r"FullGridStepCm\s*=\s*([0-9.]+)f", dense)
low_grid = re.search(r"LowCPUGridStepCm\s*=\s*([0-9.]+)f", dense)
if not full_grid or not low_grid:
    raise SystemExit("PASS36 VERIFY FAIL: profile-aware foliage grid constants missing")
if float(low_grid.group(1)) <= float(full_grid.group(1)):
    raise SystemExit("PASS36 VERIFY FAIL: LowCPU grid must be coarser than Full grid")

for forbidden in (
    "LowCPUHalfExtentCm = 10000.0f",
    "PopulationMinX = Museum.X - LowCPUHalfExtentCm",
    "PopulationMaxX = Museum.X + LowCPUHalfExtentCm",
    "PopulationMinY = Museum.Y - LowCPUHalfExtentCm",
    "PopulationMaxY = Museum.Y + LowCPUHalfExtentCm",
    "full_sector_population=0",
):
    forbid(dense + foliage_guard, forbidden, "retired Museum-only LowCPU foliage contract")

# The foliage guard now owns both minimum density and factual spatial distribution. Completion/count alone may
# not mint PASS36 READY on a 960x940m map.
for needle in (
    "ValidateDenseFoliage(",
    "int32 MinGrassInstances",
    "int32& OutOccupiedBins",
    "int32 OutQuadrantOccupied[4]",
    "bool& bOutEdgeReach",
):
    require(foliage_guard_h, needle, "profile-aware spatial foliage validation signature")

for needle in (
    'Block0PopulationCompleteTag(TEXT("OC_Block0FullMapGrassComplete"))',
    "ActorHasTag(Block0PopulationCompleteTag)",
    "const int32 MinGrassInstances = bLowCPU ? 48 : 250",
    "CoverageBinsPerAxis = 4",
    "MinOccupiedBins = 12",
    "MinOccupiedBinsPerQuadrant = 2",
    "EdgeToleranceFraction = 0.20f",
    "GetInstanceTransform(Index, InstanceTransform, true)",
    "OutOccupiedBins >= MinOccupiedBins",
    "bOutEdgeReach",
    "PASS45_BLOCK0_SPATIAL_GRASS_COVERAGE_READY",
    "PASS45_BLOCK0_SPATIAL_GRASS_COVERAGE_FAIL",
    "full_playable_distribution=1",
    "block0_spatial_grass_distribution_insufficient",
    "PASS36_LOWCPU_FOLIAGE_RUNTIME_READY",
    "full_sector_population=1",
    "population_complete=1",
    "density_policy_only=1",
    "spatial_coverage=1",
    "edge_reach=1",
    "full_map_foliage_population_incomplete",
    "ValidationAccumulator < 0.25f",
    "PASS42_FOLIAGE_GUARD_THROTTLED_READY",
):
    require(foliage_guard, needle, "LowCPU/full-map spatial foliage runtime guard")

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
print("- LowCPU foliage covers the same compact 960x940m playable Oster bounds as Full profile and reduces density/cull budget instead of spatially cropping the city")
print("- runtime foliage READY requires completion, minimum density, 4x4 spatial coverage, quadrant coverage and map-edge reach")
print("- PASS36 cannot false-pass when accepted grass is concentrated in a small crop")
print("- foliage acceptance scans are throttled and stop after convergence")
print("- missing/default weapon materials are reported as authored-content gaps, never painted grey with BasicShapeMaterial")
print("- a fully audited rack with material gaps stops its scan immediately instead of repeating for the whole budget")
print("- generic real-mesh fallback remains playable but is not production-art acceptance")
print("STATUS: SOURCE VERIFIED; actual UE 5.8 FPS/material/visual acceptance remains runtime-only")