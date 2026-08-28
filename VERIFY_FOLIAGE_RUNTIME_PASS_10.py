from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict/Source/OsterConflict"

WORLD = SRC / "Private/OCWorldSectorOster.cpp"
DENSE_H = SRC / "Public/OCDenseGroundFoliageSubsystem.h"
DENSE = SRC / "Private/OCDenseGroundFoliageSubsystem.cpp"
GUARD_H = SRC / "Public/OCFoliageRuntimeGuardSubsystem.h"
GUARD_CPP = SRC / "Private/OCFoliageRuntimeGuardSubsystem.cpp"
LAUNCHER = ROOT / "RUN_R14_FOLIAGE_RUNTIME_ACCEPTANCE.cmd"
LATEST_RUNTIME_EVIDENCE = ROOT / "RUNTIME_EVIDENCE/2026-08-27_PASS45_REJECTED/README.md"
LANDSCAPE_MATERIAL = ROOT / "OsterConflict/Content/AdvancedVillagePack/Materials/M_Inst_Landscape.uasset"
LANDSCAPE_FOLIAGE_VARIANTS = tuple(
    ROOT / f"OsterConflict/Content/AdvancedVillagePack/Materials/M_Inst_Landscape_Foliage_Var0{i}.uasset"
    for i in range(1, 4)
)


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
dense_h = read(DENSE_H)
dense = read(DENSE)
guard_h = read(GUARD_H)
guard = read(GUARD_CPP)
launcher = read(LAUNCHER)
latest_runtime_evidence = read(LATEST_RUNTIME_EVIDENCE)

# Source zoning is still present for tactical/world-layout compatibility, but PASS45 item 31 no longer accepts
# "hidden forever" as retirement. These Cube proxies must be physically destroyed before runtime acceptance.
for needle in (
    'GrassMown = MakeISM(TEXT("GrassMown"), TEXT("NoCollision"));',
    'GrassRough = MakeISM(TEXT("GrassRough"), TEXT("NoCollision"));',
    'GrassWetland = MakeISM(TEXT("GrassWetland"), TEXT("NoCollision"));',
):
    require(world, needle, "source ground-cover zoning")

# Block 0 may not regress to one uniform golf-course treatment. The dense owner already has factual semantic
# maintained-vs-rough behavior: civic zones use tighter/shorter grass and fewer weeds/flowers; ordinary roadside
# terrain can use all grass variants, taller scale and more ground plants. Guard these differences explicitly.
for needle in (
    "bool IsMaintainedCivicZone(const FVector& Point)",
    "AOCWorldSectorOster::ParkAnchor()",
    "AOCWorldSectorOster::CollegeAnchor()",
    "AOCWorldSectorOster::StadiumAnchor()",
    "IsInside2DBox(Point, Park, 10500.0f, 8000.0f)",
    "IsInside2DBox(Point, College + FVector(0.0f, 4200.0f, 0.0f), 7000.0f, 6500.0f)",
    "IsInside2DBox(Point, Stadium, 7600.0f, 5600.0f)",
    "const bool bMaintained = IsMaintainedCivicZone(BaseLocation);",
    "RandomStream.RandRange(bLowCPUProfile ? 3 : 4, bLowCPUProfile ? 4 : 5)",
    "RandomStream.RandRange(bLowCPUProfile ? 2 : 3, bLowCPUProfile ? 4 : 5)",
    "RandomStream.RandRange(0, FMath::Min(1, VariantCount - 1))",
    "RandomStream.RandRange(0, VariantCount - 1)",
    "RandomStream.FRandRange(0.68f, 0.94f)",
    "RandomStream.FRandRange(0.82f, 1.18f)",
    "const float PlantChance = bMaintained ? 0.03f : 0.12f;",
    "const float FlowerChance = bMaintained ? 0.008f : 0.025f;",
):
    require(dense, needle, "Block0 maintained-versus-rough visual zoning")

# These tracked assets are evidence that the landscape pack contains a base landscape material and dedicated
# foliage material variants. Presence is guarded, but the verifier deliberately does not require applying foliage
# material instances to the giant Ground plane without UE material-domain/runtime evidence.
if not LANDSCAPE_MATERIAL.is_file():
    raise SystemExit("PASS10 FOLIAGE VERIFY FAIL: tracked base landscape material is missing")
for material_path in LANDSCAPE_FOLIAGE_VARIANTS:
    if not material_path.is_file():
        raise SystemExit(
            f"PASS10 FOLIAGE VERIFY FAIL: tracked landscape foliage variant is missing: {material_path.name}"
        )

# Developer-only world labels/markers may remain as source semantic/debug data, but they may never survive as
# runtime scenery. World BeginPlay already hides them immediately; the guard now destroys them entirely.
for needle in (
    'ReferenceMarkers = MakeISM(TEXT("ReferenceMarkers"), TEXT("NoCollision"));',
    'MuseumLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("MuseumLabel"));',
    'if (ReferenceMarkers) ReferenceMarkers->SetVisibility(false, true);',
    'if (Label) Label->SetVisibility(false, true);',
):
    require(world, needle, "developer marker source semantics")

# Real visible runtime grass must remain the batched mesh/HISM owner. LowCPU changes density/render budget,
# never the factual central-Oster spatial scope.
for needle in (
    "UHierarchicalInstancedStaticMeshComponent",
    'TEXT("/Game/PN_FoliageCollection/Meshes/grassMesh/grass_01_01_mesh.grass_01_01_mesh")',
    'TEXT("OC_DenseGroundFoliage")',
    'TEXT("DenseGrass_%d")',
    "Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);",
    "Component->SetCastShadow(false);",
    "PopulateBatch",
    "CompactMinX = -78000.0f",
    "CompactMaxX =  18000.0f",
    "CompactMinY = -12000.0f",
    "CompactMaxY =  82000.0f",
    "full_playable_bounds=1",
    "museum_only=0",
):
    require(dense, needle, "dense foliage owner")

for name, ceiling in (("FullCellsPerBatch", 32), ("LowCPUCellsPerBatch", 48)):
    match = re.search(rf"constexpr\s+int32\s+{name}\s*=\s*(\d+)\s*;", dense)
    if not match or not 1 <= int(match.group(1)) <= ceiling:
        raise SystemExit(
            f"PASS10 FOLIAGE VERIFY FAIL: {name} is missing or exceeds the {ceiling}-cell performance ceiling"
        )
require(dense, "ActiveCellsPerBatch = bLowCPUProfile ? LowCPUCellsPerBatch : FullCellsPerBatch;",
        "profile-specific batched population budget")

# PASS45 Block 0 regression: a cell-center trace is only a cheap preflight. Every actual randomized candidate
# must be independently traced and rejected on road/sidewalk/path/building/plaza/foundation/water surfaces.
# Otherwise an accepted cell near a hard or water surface can spill grass several metres onto it.
for needle in (
    "int32 ProcessedCells = 0;",
    "int32 CandidateTraceAttempts = 0;",
    "int32 CandidateAccepted = 0;",
    "int32 CandidateRejectedBlocked = 0;",
    "int32 CandidateRejectedTrace = 0;",
    "int32 CandidateRejectedBounds = 0;",
):
    require(dense_h, needle, "Block0 foliage state contract")

for needle in (
    "auto ResolveCandidateSurface =",
    "XY.X < PopulationMinX || XY.X > PopulationMaxX",
    "XY.Y < PopulationMinY || XY.Y > PopulationMaxY",
    "++CandidateTraceAttempts;",
    "World->LineTraceSingleByChannel(",
    "CandidateHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams",
    "CandidateHit.ImpactNormal.Z < 0.72f || IsBlockedSurface(CandidateHit)",
    "++CandidateRejectedBlocked;",
    "++CandidateRejectedTrace;",
    "++CandidateRejectedBounds;",
    "++CandidateAccepted;",
    "candidate_surface_guard=1",
    "water_surface_guard=1",
    "candidate_traces=%d",
    "candidate_accepted=%d",
    "candidate_rejected_blocked=%d",
    "candidate_rejected_trace=%d",
    "candidate_rejected_bounds=%d",
):
    require(dense, needle, "Block0 per-candidate surface guard")

# All three randomized foliage families must pass the same guard before AddInstance.
if dense.count("if (!ResolveCandidateSurface(CandidateXY, CandidateLocation)) continue;") < 1:
    raise SystemExit("PASS10 FOLIAGE VERIFY FAIL: grass candidate can bypass per-instance surface validation")
if dense.count("if (ResolveCandidateSurface(CandidateXY, CandidateLocation))") < 2:
    raise SystemExit("PASS10 FOLIAGE VERIFY FAIL: plant/flower candidate can bypass per-instance surface validation")

for blocked_term in (
    'TEXT("road")', 'TEXT("street")', 'TEXT("sidewalk")', 'TEXT("pavement")',
    'TEXT("asphalt")', 'TEXT("concrete")', 'TEXT("path")', 'TEXT("building")',
    'TEXT("house")', 'TEXT("landmark")', 'TEXT("fence")', 'TEXT("plaza")',
    'TEXT("stadium")', 'TEXT("parking")', 'TEXT("foundation")', 'TEXT("water")',
    'TEXT("river")', 'TEXT("lake")', 'TEXT("pond")', 'TEXT("canal")', 'TEXT("reservoir")',
):
    require(dense, blocked_term, "blocked-surface vocabulary")

for blocked_tag in (
    'TEXT("Water")', 'TEXT("River")', 'TEXT("Lake")', 'TEXT("Pond")',
    'TEXT("Canal")', 'TEXT("Reservoir")', 'TEXT("NoFoliage")',
):
    require(dense, blocked_tag, "blocked-surface actor tags")

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

# Pass 10 protects the full-profile >=250 contract. Under PASS45 Block 0, LowCPU may reduce density/cull range
# and retain its explicit >=48 floor, but it is forbidden to shrink spatial scope back to a museum-only crop.
require(guard, 'bLowCPU ? TEXT("LowCPU") : TEXT("Full")', "runtime profile evidence")
for needle in (
    "full_playable_bounds=1",
    "full_sector_population=1",
    "density_policy_only=1",
):
    require(guard, needle, "Block0 LowCPU full-sector evidence")
forbid(guard, "full_sector_population=0", "retired LowCPU spatial-crop evidence")

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

# Source checks cannot erase the user's newest direct UE evidence.
for needle in ("RUNTIME REJECTED", "2026-08-27"):
    require(latest_runtime_evidence, needle, "latest runtime evidence authority")

print("FOLIAGE RUNTIME PASS 10 + PASS45 BLOCK0 SOURCE CONTRACT PASS")
print("- source zoning/debug components are physically destroyed rather than merely hidden at runtime")
print("- real batched DenseGrass HISM covers the 960m x 940m compact playable bounds with profile-specific budgets")
print("- maintained civic lawns and rough ordinary terrain are source-guarded as distinct clump/variant/scale/weed policies")
print("- tracked landscape foliage variants are preserved as content evidence but are not blindly applied to the Ground plane")
print("- LowCPU is density/cull policy only and cannot crop the factual full-sector population scope")
print("- every randomized grass/plant/flower candidate is independently traced before AddInstance")
print("- road/sidewalk/path/building/plaza/foundation/water spill is fail-closed at the final candidate position")
print("- water/river/lake/pond/canal/reservoir component names and actor tags are explicit no-foliage surfaces")
print("- five developer text labels and ReferenceMarkers cannot survive as player-facing scenery")
print("- full profile preserves >=250 real grass instances; LowCPU retains its explicit >=48 floor")
print("- latest factual runtime verdict remains RUNTIME REJECTED 2026-08-27")
print("- Gate K remains explicitly OPEN while major ground/other proxy visual work remains")
print("STATUS: SOURCE CONTRACT ONLY; UE 5.8 visual/runtime acceptance still required")
