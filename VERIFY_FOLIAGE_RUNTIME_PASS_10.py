from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict/Source/OsterConflict"

WORLD = SRC / "Private/OCWorldSectorOster.cpp"
DENSE = SRC / "Private/OCDenseGroundFoliageSubsystem.cpp"
GUARD_H = SRC / "Public/OCFoliageRuntimeGuardSubsystem.h"
GUARD_CPP = SRC / "Private/OCFoliageRuntimeGuardSubsystem.cpp"
LAUNCHER = ROOT / "RUN_R14_FOLIAGE_RUNTIME_ACCEPTANCE.cmd"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS10 FOLIAGE VERIFY FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8")


def require(text: str, needle: str, where: str) -> None:
    if needle not in text:
        raise SystemExit(f"PASS10 FOLIAGE VERIFY FAIL: {where}: missing {needle!r}")


world = read(WORLD)
dense = read(DENSE)
guard_h = read(GUARD_H)
guard = read(GUARD_CPP)
launcher = read(LAUNCHER)

# Preserve the source zoning information, but make explicit that it is only placeholder geometry.
for needle in (
    'GrassMown = MakeISM(TEXT("GrassMown"), TEXT("NoCollision"));',
    'GrassRough = MakeISM(TEXT("GrassRough"), TEXT("NoCollision"));',
    'GrassWetland = MakeISM(TEXT("GrassWetland"), TEXT("NoCollision"));',
    "Source-only placeholder: very thin instanced boxes mark vegetation zones",
    "AddGrassPatch(GrassMown, Stadium",
):
    require(world, needle, "source ground-cover zoning")

# Real visible runtime grass must remain the batched mesh/HISM owner.
for needle in (
    "UHierarchicalInstancedStaticMeshComponent",
    'TEXT("/Game/PN_FoliageCollection/Meshes/grassMesh/grass_01_01_mesh.grass_01_01_mesh")',
    'TEXT("OC_DenseGroundFoliage")',
    'TEXT("DenseGrass_%d")',
    "Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);",
    "Component->SetCastShadow(false);",
    "PopulateBatch",
    "CellsPerBatch = 88",
):
    require(dense, needle, "dense foliage owner")

for needle in (
    "UOCFoliageRuntimeGuardSubsystem",
    "UTickableWorldSubsystem",
    "RetireSourceGroundCoverProxies",
    "ValidateDenseFoliage",
):
    require(guard_h, needle, "runtime guard header")

for needle in (
    'TEXT("GrassMown")',
    'TEXT("GrassRough")',
    'TEXT("GrassWetland")',
    "Proxy->SetVisibility(false, true);",
    "Proxy->SetHiddenInGame(true, true);",
    "Proxy->SetCollisionEnabled(ECollisionEnabled::NoCollision);",
    "Proxy->SetGenerateOverlapEvents(false);",
    "Proxy->SetCanEverAffectNavigation(false);",
    "Proxy->SetCastShadow(false);",
    'TEXT("OC_DenseGroundFoliage")',
    'Name.StartsWith(TEXT("DenseGrass_"))',
    "OutGrassInstances >= 250",
    "PASS10_GROUND_COVER_PROXY_RETIRED",
    "PASS10_FOLIAGE_RUNTIME_READY",
    "PASS10_FOLIAGE_RUNTIME_FAIL",
):
    require(guard, needle, "runtime foliage guard")

for needle in (
    "RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd",
    "PASS10_GROUND_COVER_PROXY_RETIRED",
    "PASS10_FOLIAGE_RUNTIME_READY",
    "PASS10_FOLIAGE_RUNTIME_FAIL",
    "R14_CURRENT_GAMEPLAY.log",
):
    require(launcher, needle, "Windows foliage acceptance launcher")

print("FOLIAGE RUNTIME PASS 10 SOURCE CONTRACT PASS")
print("- source zoning cubes remain authoring data, not accepted gameplay presentation")
print("- real batched DenseGrass HISM remains the visible runtime owner")
print("- runtime guard retires GrassMown/GrassRough/GrassWetland proxy slabs")
print("- runtime evidence requires one dense foliage actor and >=250 real grass instances")
print("- strict Windows launcher requires Pass 10 proxy-retired + foliage-ready markers")
print("STATUS: SOURCE CONTRACT ONLY; UE 5.8 visual/runtime acceptance still required")
