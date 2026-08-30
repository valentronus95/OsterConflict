#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS45 PARK GEOMETRY OWNER NORMALIZATION VERIFY FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        raise SystemExit(
            f"PASS45 PARK GEOMETRY OWNER NORMALIZATION VERIFY FAIL: {label}: missing {needle!r}"
        )


def forbid(text: str, needle: str, label: str) -> None:
    if needle in text:
        raise SystemExit(
            f"PASS45 PARK GEOMETRY OWNER NORMALIZATION VERIFY FAIL: {label}: forbidden {needle!r}"
        )


header = read(SRC / "Public" / "OCParkGeometryOwnerNormalizationSubsystem.h")
impl = read(SRC / "Private" / "OCParkGeometryOwnerNormalizationSubsystem.cpp")
world = read(SRC / "Private" / "OCWorldSectorOster.cpp")
gate = read(SRC / "Private" / "OCVisualFidelityGateKSubsystem.cpp")
tactical = read(SRC / "Private" / "OCTacticalMapSubsystem.cpp")

for needle in (
    "UOCParkGeometryOwnerNormalizationSubsystem",
    "fail-closed, geometry-preserving migration bridge",
    "primary_authoring=0",
    "must be retired when",
    "may never hide rejected geometry",
    "remain visible Gate K CONTENT GAP",
):
    require(header, needle, "migration-bridge ownership")

for needle in (
    "ExpectedLegacyGeometryInstances = 3",
    "NormalizationDelaySeconds = 0.45f",
    'FindISM(Sector, TEXT("ParkGeometry"))',
    "IsEngineCube",
    'TEXT("ParkCentralGround")',
    'TEXT("ParkNorthCivicGround")',
    'TEXT("CollegeRecreationGround")',
    "SourceTransforms[0]",
    "SourceTransforms[1]",
    "SourceTransforms[2]",
    "ParkCentralGround->AddInstance(SourceTransforms[0], false)",
    "ParkNorthCivicGround->AddInstance(SourceTransforms[1], false)",
    "CollegeRecreationGround->AddInstance(SourceTransforms[2], false)",
    "LegacyGeometry->ClearInstances()",
    "LegacyGeometry->GetInstanceCount() != 0",
    "PASS45_PARK_GEOMETRY_OWNER_NORMALIZATION_FAIL",
    "PASS45_PARK_GEOMETRY_OWNER_NORMALIZATION_READY",
    "source_instances_preserved=1",
    "source_instance_total=3",
    "destination_instance_total=3",
    "geometry_preserved=1",
    "mesh_preserved=1",
    "material_preserved=1",
    "collision_preserved=1",
    "authored_replacements=0",
    "content_gap=1",
    "tactical_map_bounds_preserved=1",
    "primary_authoring=0",
    "migration_bridge=1",
    "gate_k_complete=0",
    "runtime_acceptance=0",
):
    require(impl, needle, "exact ParkGeometry normalization")

for forbidden in (
    'FindISM(Sector, TEXT("ParkDetails"))',
    'FindISM(Sector, TEXT("ParkPaths"))',
    'FindISM(Sector, TEXT("ParkMemorialPlaza"))',
    'FindISM(Sector, TEXT("ParkMemorialApproach"))',
    'FindISM(Sector, TEXT("ParkSkateFitness"))',
    'FindISM(Sector, TEXT("ParkBenches"))',
    "SetVisibility(false",
    "SetHiddenInGame(true",
):
    forbid(impl, forbidden, "scope/hidden-geometry guard")

central_begin = world.find("void AOCWorldSectorOster::BuildCentralPark()")
central_end = world.find("\nvoid AOCWorldSectorOster::BuildCollegeSector()", central_begin)
college_begin = central_end + 1 if central_end >= 0 else -1
college_end = world.find("\nvoid AOCWorldSectorOster::", college_begin + 1) if college_begin >= 0 else -1
if central_begin < 0 or central_end < 0 or college_begin < 0:
    raise SystemExit("PASS45 PARK GEOMETRY OWNER NORMALIZATION VERIFY FAIL: cannot isolate park/college builders")
if college_end < 0:
    college_end = len(world)
central = world[central_begin:central_end]
college = world[college_begin:college_end]

central_contract = (
    'AddBox(ParkGeometry, Park + FVector(0, 0, 3), FVector(20500, 16000, 6));',
    'AddBox(ParkGeometry, NorthCivic + FVector(0,0,4), FVector(8500, 7200, 8));',
)
college_contract = (
    'AddBox(ParkGeometry, College + FVector(-4900, 7000, 10), FVector(6100, 3300, 12), Yaw);',
)
for needle in central_contract:
    require(central, needle, "indexed Central Park geometry source contract")
for needle in college_contract:
    require(college, needle, "indexed college recreation source contract")

if world.count("AddBox(ParkGeometry") != 3:
    raise SystemExit(
        "PASS45 PARK GEOMETRY OWNER NORMALIZATION VERIFY FAIL: ParkGeometry source count changed; bridge indexing is stale"
    )
if central.find(central_contract[0]) > central.find(central_contract[1]):
    raise SystemExit(
        "PASS45 PARK GEOMETRY OWNER NORMALIZATION VERIFY FAIL: Central Park ParkGeometry source order changed"
    )

# This is intentionally a temporary bridge. Direct primary authoring must force deliberate bridge retirement instead
# of leaving both paths alive and silently duplicating semantic ownership.
for destination_name in ("ParkCentralGround", "ParkNorthCivicGround", "CollegeRecreationGround"):
    forbid(world, destination_name, "temporary bridge retirement trigger")

# Gate K observes later than both current ownership-normalization bridges and remains mutation-free.
for needle in (
    "ElapsedSeconds < 3.0f",
    "CountVisibleBasicShapes",
    "PASS45_GATE_K_RUNTIME_FAIL",
):
    require(gate, needle, "Gate K ordering/observation")
forbid(gate, "SetVisibility(false", "Gate K mutation")

# Tactical map projection is component-bounds based rather than hard-wired to the legacy ParkGeometry name. Exact
# re-home with identical transforms therefore preserves projection geography; a future name dependency must fail.
for needle in (
    "ResolveSectorContentBounds",
    "Sector.GetComponents<UPrimitiveComponent>(Components)",
    "Component->Bounds.GetBox()",
    "BuildProjectionFromSector",
):
    require(tactical, needle, "tactical-map bounds contract")
forbid(tactical, 'TEXT("ParkGeometry")', "legacy ParkGeometry tactical-map name dependency")

print("PASS45 PARK GEOMETRY OWNER NORMALIZATION SOURCE PASS")
print("- ParkGeometry exact source contract is 3 instances: central park + north civic + college recreation")
print("- runtime bridge re-homes them 1/1/1 with mesh/material/collision/transform preserved")
print("- mixed source is cleared only after all three exact destination owners are populated")
print("- tactical-map projection remains component-bounds based and is not tied to ParkGeometry by name")
print("- exact ground owners remain visible CONTENT GAP; this bridge cannot close Gate K/runtime acceptance")
