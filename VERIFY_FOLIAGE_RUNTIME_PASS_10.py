from pathlib import Path
import re

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


def forbid(text: str, needle: str, where: str) -> None:
    if needle in text:
        raise SystemExit(f"PASS10 FOLIAGE VERIFY FAIL: {where}: forbidden {needle!r}")


world = read(WORLD)
dense = read(DENSE)
guard_h = read(GUARD_H)
guard = read(GUARD_CPP)
launcher = read(LAUNCHER)

# Source zoning is still present for tactical/world-layout compatibility, but PASS45 item 31 no longer accepts
# "hidden forever" as retirement. These Cube proxies must be physically destroyed before runtime acceptance.
for needle in (
    'GrassMown = MakeISM(TEXT("GrassMown"), TEXT("NoCollision"));',
    'GrassRough = MakeISM(TEXT("GrassRough"), TEXT("NoCollision"));',
    'GrassWetland = MakeISM(TEXT("GrassWetland"), TEXT("NoCollision"));',
):
    require(world, needle, "source ground-cover zoning")

# Developer-only world labels/markers may remain as source semantic/debug data, but they may never survive as
# runtime scenery. World BeginPlay already hides them immediately; the guard now destroys them entirely.
for needle in (
    'ReferenceMarkers = MakeISM(TEXT("ReferenceMarkers"), TEXT("NoCollision"));',
    'MuseumLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("MuseumLabel"));',
    'if (ReferenceMarkers) ReferenceMarkers->SetVisibility(false, true);',
    'if (Label) Label->SetVisibility(false, true);',
):
    require(world, needle, "developer marker source semantics")

# Real visible runtime grass must remain the batched mesh/HISM owner. Pass 36 may reduce the LowCPU spatial
# budget after measured performance collapse, but it must remain bounded/batched and authored-mesh based.
for needle in (
    "UHierarchicalInstancedStaticMeshComponent",
    'TEXT("/Game/PN_FoliageCollection/Meshes/grassMesh/grass_01_01_mesh.grass_01_01_mesh")',
    'TEXT("OC_DenseGroundFoliage")',
    'TEXT("DenseGrass_%d")',
    "Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);",
    "Component->SetCastShadow(false);",
    "PopulateBatch",
):
    require(dense, needle, "dense foliage owner")
batch = re.search(r"constexpr\s+int32\s+CellsPerBatch\s*=\s*(\d+)\s*;", dense)
if not batch or not 1 <= int(batch.group(1)) <= 48:
    raise SystemExit("PASS10 FOLIAGE VERIFY FAIL: dense foliage batch is missing or exceeds the performance ceiling")

for needle in (
    "UOCFoliageRuntimeGuardSubsystem",
    "UTickableWorldSubsystem",
    "DestroySourceGroundCoverProxies",
    "DestroyDeveloperVisualMarkers",
    "ValidateDenseFoliage",
):
    require(guard_h, needle, "runtime guard header")
forbid(guard_h, "RetireSourceGroundCoverProxies", "obsolete hide-only ground proxy contract")

for needle in (
    'TEXT("GrassMown")',
    'TEXT("GrassRough")',
    'TEXT("GrassWetland")',
    'TEXT("ReferenceMarkers")',
    'TEXT("MuseumLabel")',
    'TEXT("StadiumLabel")',
    'TEXT("ParkLabel")',
    'TEXT("CollegeLabel")',
    'TEXT("KrushelnytskaStreetLabel")',
    "Proxy->DestroyComponent();",
    "Marker->DestroyComponent();",
    "Label->DestroyComponent();",
    'TEXT("OC_DenseGroundFoliage")',
    'Name.StartsWith(TEXT("DenseGrass_"))',
    "OutGrassInstances >= MinGrassInstances",
    "const int32 MinGrassInstances = bLowCPU ? 48 : 250;",
    "PASS45_GROUND_COVER_PRIMITIVES_DESTROYED",
    "PASS45_DEVELOPER_WORLD_MARKERS_DESTROYED",
    "groundProxyComponents=0",
    "developerMarkers=0",
    "PASS45_VISUAL_CLEANUP_PARTIAL_READY",
    "gate_k_complete=0",
    "PASS10_FOLIAGE_RUNTIME_READY",
    "PASS10_FOLIAGE_RUNTIME_FAIL",
    "PASS36_LOWCPU_FOLIAGE_RUNTIME_READY",
):
    require(guard, needle, "runtime foliage/visual cleanup guard")
forbid(guard, "PASS10_GROUND_COVER_PROXY_RETIRED", "obsolete hide-only runtime evidence")

# Pass 10 still protects the original full-profile >=250 contract. Pass 36 deliberately gives the measured
# LowCPU acceptance path its own lower floor so the guard does not force expensive full-sector population back.
require(guard, 'bLowCPU ? TEXT("LowCPU") : TEXT("Full")', "runtime profile evidence")
require(guard, 'full_sector_population=0', "bounded LowCPU evidence")

for needle in (
    "RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd",
    "PASS45_GROUND_COVER_PRIMITIVES_DESTROYED",
    "PASS45_DEVELOPER_WORLD_MARKERS_DESTROYED",
    "PASS10_FOLIAGE_RUNTIME_READY",
    "PASS10_FOLIAGE_RUNTIME_FAIL",
    "Gate K remains open",
    "R14_CURRENT_GAMEPLAY.log",
):
    require(launcher, needle, "Windows foliage acceptance launcher")
forbid(launcher, "PASS10_GROUND_COVER_PROXY_RETIRED", "obsolete hide-only launcher evidence")

print("FOLIAGE RUNTIME PASS 10 + PASS45 ITEM 31 PARTIAL SOURCE CONTRACT PASS")
print("- source zoning/debug components are physically destroyed rather than merely hidden at runtime")
print("- real batched DenseGrass HISM remains the visible runtime owner with a bounded performance budget")
print("- five developer text labels and ReferenceMarkers cannot survive as player-facing scenery")
print("- full profile preserves >=250 real grass instances; LowCPU uses its explicit bounded >=48 contract")
print("- Gate K remains explicitly OPEN while other BasicShape/proxy core families still exist")
print("STATUS: SOURCE CONTRACT ONLY; UE 5.8 visual/runtime acceptance still required")
