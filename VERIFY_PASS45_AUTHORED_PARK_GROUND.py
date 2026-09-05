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
world_header = read(SRC / "Public" / "OCWorldSectorOster.h")
world = read(SRC / "Private" / "OCWorldSectorOster.cpp")
gate = read(SRC / "Private" / "OCVisualFidelityGateKSubsystem.cpp")
tactical = read(SRC / "Private" / "OCTacticalMapSubsystem.cpp")
read(PLANE)
read(GRASS)

for needle in (
    "created directly by AOCWorldSectorOster",
    "ParkCentralGround, ParkNorthCivicGround and CollegeRecreationGround",
    "source surface-top elevation",
    "primary_authoring=1 / normalization_bridge=0",
    "does not constitute UE 5.8 visual acceptance",
):
    require(header, needle, "authored-ground ownership")

for owner in ("ParkCentralGround", "ParkNorthCivicGround", "CollegeRecreationGround"):
    require(world_header, f"TObjectPtr<UInstancedStaticMeshComponent> {owner};", f"{owner} primary member")
    require(world, f'{owner} = MakeISM(TEXT("{owner}"), TEXT("BlockAll"));', f"{owner} primary component")

for needle in (
    'AddBox(ParkCentralGround, Park + FVector(0, 0, 3), FVector(20500, 16000, 6));',
    'AddBox(ParkNorthCivicGround, NorthCivic + FVector(0,0,4), FVector(8500, 7200, 8));',
    'AddBox(CollegeRecreationGround, College + FVector(-4900, 7000, 10), FVector(6100, 3300, 12), Yaw);',
    "PASS45_PARK_GROUND_PRIMARY_OWNERS_READY",
    "primary_authoring=1 normalization_bridge=0 authored_surface_upgrade_pending=1",
):
    require(world, needle, "direct green-ground source contract")
forbid(world, "AddBox(ParkGeometry,", "legacy ParkGeometry source resurrection")

for needle in (
    "/Game/AdvancedVillagePack/Meshes/SM_Plane_1x1.SM_Plane_1x1",
    "/Game/Mega_Street_Props_Pack/Street_Props_pack_V2/Materials/Instances/M_Grass_Inst.M_Grass_Inst",
    "AuthoredUpgradeDelaySeconds = 0.70f",
    'FindISM(Sector, TEXT("ParkGeometry"))',
    'FindISM(Sector, TEXT("ParkCentralGround"))',
    'FindISM(Sector, TEXT("ParkNorthCivicGround"))',
    'FindISM(Sector, TEXT("CollegeRecreationGround"))',
    "LegacyGeometry->GetInstanceCount() != 0",
    "primary_source_required=1 normalization_bridge=0",
    "BuildPlan",
    "RestorePlan",
    "ApplyPlan",
    "SourceTopZ",
    "NewLocation.Z = SourceTopZ - NewTopOffsetZ",
    "SetStaticMesh(AuthoredMesh)",
    "SetMaterial(Slot, GrassMaterial)",
    "PASS45_AUTHORED_PARK_GROUND_READY",
    "exact_semantic_owners=3",
    "primary_authoring=1 normalization_bridge=0",
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
    "primary_authoring=0",
):
    forbid(impl, forbidden, "authored-ground scope/ownership")

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
print("- three green-ground owners are direct AOCWorldSectorOster primary components; no normalization bridge remains")
print("- tracked SM_Plane_1x1 + M_Grass_Inst upgrade only those three exact owners")
print("- XY footprint, yaw and source surface-top elevation remain bounds-preserved with transactional rollback")
print("- tactical-map projection remains component-bounds based; UE 5.8 visual acceptance remains open")
