#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS45 PARK PRIMARY OWNERS VERIFY FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        raise SystemExit(f"PASS45 PARK PRIMARY OWNERS VERIFY FAIL: {label}: missing {needle!r}")


def forbid(text: str, needle: str, label: str) -> None:
    if needle in text:
        raise SystemExit(f"PASS45 PARK PRIMARY OWNERS VERIFY FAIL: {label}: forbidden {needle!r}")


world_h = read(SRC / "Public" / "OCWorldSectorOster.h")
world = read(SRC / "Private" / "OCWorldSectorOster.cpp")
ground_h = read(SRC / "Public" / "OCParkGroundAuthoredUpgradeSubsystem.h")
ground = read(SRC / "Private" / "OCParkGroundAuthoredUpgradeSubsystem.cpp")
hard_h = read(SRC / "Public" / "OCParkHardscapeAuthoredUpgradeSubsystem.h")
hard = read(SRC / "Private" / "OCParkHardscapeAuthoredUpgradeSubsystem.cpp")
gate = read(SRC / "Private" / "OCVisualFidelityGateKSubsystem.cpp")
tactical = read(SRC / "Private" / "OCTacticalMapSubsystem.cpp")

retired = [
    SRC / "Public" / "OCParkGeometryOwnerNormalizationSubsystem.h",
    SRC / "Private" / "OCParkGeometryOwnerNormalizationSubsystem.cpp",
    SRC / "Public" / "OCParkSemanticOwnerNormalizationSubsystem.h",
    SRC / "Private" / "OCParkSemanticOwnerNormalizationSubsystem.cpp",
    ROOT / "VERIFY_PASS45_PARK_GEOMETRY_OWNER_NORMALIZATION.py",
    ROOT / "VERIFY_PASS45_PARK_SEMANTIC_OWNER_NORMALIZATION.py",
]
for path in retired:
    if path.exists():
        raise SystemExit(
            f"PASS45 PARK PRIMARY OWNERS VERIFY FAIL: retired normalization bridge still exists: {path.relative_to(ROOT)}"
        )

primary_owners = (
    "ParkCentralGround",
    "ParkNorthCivicGround",
    "CollegeRecreationGround",
    "ParkMemorialSurface",
    "ParkMemorialMonument",
    "ParkSkateSurface",
    "ParkSkateRamps",
)
for owner in primary_owners:
    require(world_h, f"TObjectPtr<UInstancedStaticMeshComponent> {owner};", f"{owner} UPROPERTY")
    require(world, f'{owner} = MakeISM(TEXT("{owner}"), TEXT("BlockAll"));', f"{owner} default subobject")

for legacy in ("ParkGeometry", "ParkDetails", "ParkMemorialPlaza", "ParkSkateFitness"):
    require(world, f'{legacy} = MakeISM(TEXT("{legacy}"), TEXT("BlockAll"));', f"{legacy} quarantine component")

for needle in (
    'AddBox(ParkCentralGround, Park + FVector(0, 0, 3), FVector(20500, 16000, 6));',
    'AddBox(ParkNorthCivicGround, NorthCivic + FVector(0,0,4), FVector(8500, 7200, 8));',
    'AddBox(CollegeRecreationGround, College + FVector(-4900, 7000, 10), FVector(6100, 3300, 12), Yaw);',
    'AddBox(ParkMemorialSurface, Park + FVector(-600, 200, 28), FVector(3100, 2500, 56));',
    'AddBox(ParkMemorialMonument, Park + FVector(-600, 200, 230), FVector(260, 260, 400));',
    'AddBox(ParkSkateSurface, Park + FVector(6100, -4100, 18), FVector(4300, 2600, 36));',
    'AddBoxRotated(ParkSkateRamps, Park + FVector(6100, -4100, 120)',
    'AddBoxRotated(ParkSkateRamps, Park + FVector(7400, -3500, 95)',
    "PASS45_PARK_PRIMARY_SEMANTIC_OWNERS_READY",
    "PASS45_PARK_GROUND_PRIMARY_OWNERS_READY",
    "primary_authoring=1 normalization_bridge=0",
    "remaining_content_gap_instances=3",
):
    require(world, needle, "direct source ownership")

for forbidden in (
    "AddBox(ParkGeometry,",
    "AddBox(ParkMemorialPlaza,",
    "AddBox(ParkSkateFitness,",
    "AddBoxRotated(ParkSkateFitness,",
    "NewObject<UInstancedStaticMeshComponent>(Sector",
):
    forbid(world, forbidden, "legacy/late owner resurrection")

for needle in (
    "LegacyGeometryCount == 0",
    "LegacyMemorialCount == 0",
    "LegacySkateCount == 0",
    "CentralGroundCount == 1",
    "NorthGroundCount == 1",
    "MemorialSurfaceCount == ExpectedMemorialSurface",
    "MemorialMonumentCount == ExpectedMemorialMonument",
    "SkateSurfaceCount == ExpectedSkateSurface",
    "SkateRampsCount == ExpectedSkateRamps",
    "CollegeGroundCount == 1",
    "static_assert(ExpectedSemanticDetails == 23",
):
    require(world, needle, "fail-closed primary count contract")

for text, label in ((ground_h, "ground header"), (hard_h, "hardscape header")):
    require(text, "primary_authoring=1 / normalization_bridge=0", label)
    forbid(text, "temporary", label)
for text, label in ((ground, "ground upgrader"), (hard, "hardscape upgrader")):
    require(text, "primary_source_required=1 normalization_bridge=0", label)
    require(text, "primary_authoring=1 normalization_bridge=0", label)
    forbid(text, "primary_authoring=0", label)

# The three unresolved monument/ramp instances stay deliberately visible until factual authored content exists.
for needle in (
    "ParkMemorialMonument",
    "ParkSkateRamps",
    "remaining_content_gap_instances=3",
    "gate_k_complete=0",
    "runtime_acceptance=0",
):
    require(hard, needle, "content-gap preservation")

# Gate K is still observation-only and tactical projection remains component-bounds based, not name-bound.
for needle in ("ElapsedSeconds < 3.0f", "CountVisibleBasicShapes", "PASS45_GATE_K_RUNTIME_FAIL"):
    require(gate, needle, "Gate K observation")
forbid(gate, "SetVisibility(false", "Gate K mutation")
for needle in (
    "ResolveSectorContentBounds",
    "Sector.GetComponents<UPrimitiveComponent>(Components)",
    "Component->Bounds.GetBox()",
):
    require(tactical, needle, "tactical-map bounds contract")
forbid(tactical, 'TEXT("ParkGeometry")', "legacy ParkGeometry tactical-map dependency")

print("PASS45 PARK PRIMARY SEMANTIC OWNERS SOURCE PASS")
print("- exact ground/memorial/skate owners are AOCWorldSectorOster default subobjects")
print("- ParkGeometry/ParkMemorialPlaza/ParkSkateFitness remain zero-instance quarantine only")
print("- both historical normalization bridges are physically retired")
print("- authored ground/hardscape presentation upgrades consume direct primary owners")
print("- monument + two ramps remain explicit CONTENT GAP; Gate K/runtime acceptance stay open")
