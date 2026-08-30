#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
PLANE = ROOT / "OsterConflict" / "Content" / "AdvancedVillagePack" / "Meshes" / "SM_Plane_1x1.uasset"
GRASS = ROOT / "OsterConflict" / "Content" / "Mega_Street_Props_Pack" / "Street_Props_pack_V2" / "Materials" / "Instances" / "M_Grass_Inst.uasset"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS45 AUTHORED PARK GROUND VERIFY FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        raise SystemExit(f"PASS45 AUTHORED PARK GROUND VERIFY FAIL: {label}: missing {needle!r}")


def forbid(text: str, needle: str, label: str) -> None:
    if needle in text:
        raise SystemExit(f"PASS45 AUTHORED PARK GROUND VERIFY FAIL: {label}: forbidden {needle!r}")


header = read(SRC / "Public" / "OCParkGroundAuthoredUpgradeSubsystem.h")
impl = read(SRC / "Private" / "OCParkGroundAuthoredUpgradeSubsystem.cpp")
normalizer = read(SRC / "Private" / "OCParkGeometryOwnerNormalizationSubsystem.cpp")
world = read(SRC / "Private" / "OCWorldSectorOster.cpp")
gate = read(SRC / "Private" / "OCVisualFidelityGateKSubsystem.cpp")
tactical = read(SRC / "Private" / "OCTacticalMapSubsystem.cpp")
read(PLANE)
read(GRASS)

for needle in (
    "UOCParkGroundAuthoredUpgradeSubsystem",
    "ParkCentralGround, ParkNorthCivicGround and CollegeRecreationGround",
    "source surface-top elevation",
    "rolls back the whole",
    "primary_authoring=0",
    "not UE 5.8 visual acceptance",
):
    require(header, needle, "authored-ground ownership")

for needle in (
    "/Game/AdvancedVillagePack/Meshes/SM_Plane_1x1.SM_Plane_1x1",
    "/Game/Mega_Street_Props_Pack/Street_Props_pack_V2/Materials/Instances/M_Grass_Inst.M_Grass_Inst",
    "AuthoredUpgradeDelaySeconds = 0.70f",
    'FindISM(Sector, TEXT("ParkGeometry"))',
    'FindISM(Sector, TEXT("ParkCentralGround"))',
    'FindISM(Sector, TEXT("ParkNorthCivicGround"))',
    'FindISM(Sector, TEXT("CollegeRecreationGround"))',
    "LegacyGeometry->GetInstanceCount() != 0",
    "BuildPlan",
    "RestorePlan",
    "ApplyPlan",
    "SourceBounds.BoxExtent.X * 2.0f * SourceScale.X",
    "SourceBounds.BoxExtent.Y * 2.0f * SourceScale.Y",
    "NewNativeSize.Z > 1.0f",
    "SourceTopZ",
    "NewTopOffsetZ",
    "NewLocation.Z = SourceTopZ - NewTopOffsetZ",
    "SetStaticMesh(AuthoredMesh)",
    "EmptyOverrideMaterials()",
    "SetMaterial(Slot, GrassMaterial)",
    "UpdateInstanceTransform(0, Plan.NewTransform",
    "RestorePlan(Plans[RollbackIndex])",
    "PASS45_AUTHORED_PARK_GROUND_CONTENT_GAP",
    "PASS45_AUTHORED_PARK_GROUND_FAIL",
    "PASS45_AUTHORED_PARK_GROUND_READY",
    "exact_semantic_owners=3",
    "basicshape_meshes=0",
    "basicshape_material_overrides=0",
    "source_surface_top_preserved=1",
    "xy_footprint_preserved=1",
    "yaw_preserved=1",
    "bounds_aware_surface_fit=1",
    "park_green_semantics_preserved=1",
    "transactional_preflight=1",
    "rollback_on_write_failure=1",
    "tactical_map_xy_bounds_preserved=1",
    "primary_authoring=0",
    "gate_k_complete=0",
    "runtime_acceptance=0",
):
    require(impl, needle, "exact authored-ground upgrade")

for forbidden in (
    'FindISM(Sector, TEXT("ParkDetails"))',
    'FindISM(Sector, TEXT("ParkPaths"))',
    'FindISM(Sector, TEXT("ParkMemorialSurface"))',
    'FindISM(Sector, TEXT("ParkMemorialMonument"))',
    'FindISM(Sector, TEXT("ParkMemorialApproach"))',
    'FindISM(Sector, TEXT("ParkSkateSurface"))',
    'FindISM(Sector, TEXT("ParkSkateRamps"))',
    'FindISM(Sector, TEXT("ParkBenches"))',
    "SetVisibility(false",
    "SetHiddenInGame(true",
):
    forbid(impl, forbidden, "authored-ground scope/visibility")

# Ground semantics are not invented here: the primary source deliberately tints the shared legacy ParkGeometry green.
require(world, "Tint(ParkGeometry,        FLinearColor(0.12f, 0.31f, 0.075f));", "legacy green park-ground semantics")
for needle in (
    'AddBox(ParkGeometry, Park + FVector(0, 0, 3), FVector(20500, 16000, 6));',
    'AddBox(ParkGeometry, NorthCivic + FVector(0,0,4), FVector(8500, 7200, 8));',
    'AddBox(ParkGeometry, College + FVector(-4900, 7000, 10), FVector(6100, 3300, 12), Yaw);',
):
    require(world, needle, "indexed green-ground source contract")

# Ordering contract: semantic split runs first, authored replacement second, observation-only Gate K third.
for needle in (
    "NormalizationDelaySeconds = 0.45f",
    'TEXT("ParkCentralGround")',
    'TEXT("ParkNorthCivicGround")',
    'TEXT("CollegeRecreationGround")',
    "geometry_preserved=1",
):
    require(normalizer, needle, "ParkGeometry normalization predecessor")
require(gate, "ElapsedSeconds < 3.0f", "Gate K observation delay")
forbid(gate, "SetVisibility(false", "Gate K mutation")

for needle in (
    "ResolveSectorContentBounds",
    "Sector.GetComponents<UPrimitiveComponent>(Components)",
    "Component->Bounds.GetBox()",
):
    require(tactical, needle, "tactical-map component-bounds projection")
forbid(tactical, 'TEXT("ParkGeometry")', "legacy ParkGeometry tactical-map name dependency")

print("PASS45 AUTHORED PARK GROUND SOURCE PASS")
print("- tracked SM_Plane_1x1 + M_Grass_Inst are bound only to the three exact green-ground semantic owners")
print("- ownership normalization runs at 0.45 s; authored surface upgrade at 0.70 s; Gate K observes at 3.0 s")
print("- XY footprint, yaw and source surface-top elevation are preserved with flat-plane-aware bounds fitting")
print("- all three plans preflight before mutation and prior writes roll back on a later write failure")
print("- tactical-map projection remains component-bounds based; UE 5.8 visual acceptance remains open")
