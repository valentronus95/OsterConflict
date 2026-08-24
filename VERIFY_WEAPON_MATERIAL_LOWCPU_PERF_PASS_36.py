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

# Keep the old full-profile constants for compatibility, but LowCPU must no longer iterate all 1.92 km.
# Pass 42 deliberately expands the bounded museum play window from 150 x 150 m to 200 x 200 m without
# returning to the historical full-sector population path.
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

# Guard must validate the smaller recovery profile instead of forcing the old >=250 full-sector contract.
require(foliage_guard_h, "ValidateDenseFoliage(int32 MinGrassInstances", "profile-aware foliage validation signature")
for needle in (
    "const int32 MinGrassInstances = bLowCPU ? 48 : 250",
    "PASS36_LOWCPU_FOLIAGE_RUNTIME_READY",
    'full_sector_population=0',
    "ValidationAccumulator < 0.25f",
    "PASS42_FOLIAGE_GUARD_THROTTLED_READY",
):
    require(foliage_guard, needle, "LowCPU foliage runtime guard")

# Material recovery is allowed only for null/default slots. Authored materials must never be replaced.
for needle in (
    "TObjectPtr<UMaterialInterface> MaterialRecoveryBase",
    "AuditAndRepairWeaponMaterials",
):
    require(weapon_h, needle, "weapon material recovery state")

for needle in (
    "IsMissingOrDefaultMaterial",
    "/Engine/EngineMaterials/DefaultMaterial",
    "if (!IsMissingOrDefaultMaterial(Current)) continue;",
    "/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial",
    "PASS36_WEAPON_MATERIAL_RECOVERED",
    "authored_materials_preserved=1",
    "PASS36_WEAPON_MATERIAL_AUDIT_READY",
    'RuntimeBaseRackTag(TEXT("OC_RuntimeBaseWeaponRack"))',
    "Component->SetCastShadow(false)",
):
    require(weapon, needle, "weapon material audit/recovery")

forbid(weapon, "Component->SetMaterial(Slot, MaterialRecoveryBase)",
       "recovery must use a per-weapon MID and must not overwrite valid authored slots globally")

# There must still be an exact production visual check before generic real-mesh fallback selection.
require(weapon, "HasProductionVisual(*Weapon)", "production visual preservation")
require(weapon, "ApplyRealFallback", "existing real-mesh fallback preservation")

print("WEAPON MATERIAL + LOWCPU PERFORMANCE PASS 36/42 SOURCE CONTRACT PASS")
print("- LowCPU foliage stays bounded around the museum/BASE and cannot progressively fill the whole sector")
print("- Pass 42 expands that bounded window to 200 x 200 m with 85 m grass cull distance")
print("- foliage acceptance scans are throttled to 4 Hz and stop rescanning retired source proxies")
print("- null/default weapon material slots receive a non-white runtime recovery material")
print("- authored weapon materials are explicitly preserved")
print("- static BASE rack visuals stop casting unnecessary dynamic shadows")
print("STATUS: SOURCE VERIFIED; actual UE 5.8 FPS/material acceptance remains runtime-only")
