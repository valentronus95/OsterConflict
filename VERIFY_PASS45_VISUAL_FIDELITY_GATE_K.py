#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS45 GATE K SOURCE VERIFY FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        raise SystemExit(f"PASS45 GATE K SOURCE VERIFY FAIL: {label}: missing {needle!r}")


def forbid(text: str, needle: str, label: str) -> None:
    if needle in text:
        raise SystemExit(f"PASS45 GATE K SOURCE VERIFY FAIL: {label}: forbidden {needle!r}")


world = read(SRC / "Private" / "OCWorldSectorOster.cpp")
stadium = read(SRC / "Private" / "OCR13StadiumSurfaceSubsystem.cpp")
surface_h = read(SRC / "Public" / "OCAuthoredWorldSurfaceUpgradeSubsystem.h")
surface = read(SRC / "Private" / "OCAuthoredWorldSurfaceUpgradeSubsystem.cpp")
stability = read(SRC / "Private" / "OCWorldGeometryStabilitySubsystem.cpp")
guard_h = read(SRC / "Public" / "OCVisualFidelityGateKSubsystem.h")
guard = read(SRC / "Private" / "OCVisualFidelityGateKSubsystem.cpp")
foliage_guard = read(SRC / "Private" / "OCFoliageRuntimeGuardSubsystem.cpp")
launcher = read(ROOT / "RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd")
runtime_verify = read(ROOT / "VERIFY_PASS45_GATE_K_RUNTIME_LOG.py")
visual_perf = read(ROOT / "VERIFY_VISUAL_QUALITY_TICK_BUDGET_PASS_39.py")
workflow = read(ROOT / ".github" / "workflows" / "pass45-visual-fidelity-gate-k.yml")

# Source topology still carries Engine Cube transforms, and the authoritative stadium still contains BasicShape
# blockout. This remains factual open work. Verified families are upgraded before runtime acceptance rather than
# being allowed to remain visible as those source cubes.
require(world, '/Engine/BasicShapes/Cube.Cube', "current world source topology")
require(world, '/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial', "current world source material gap")
require(stadium, '/Engine/BasicShapes/Cube.Cube', "current authoritative stadium BasicShape gap")
require(stadium, '/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial', "current authoritative stadium material gap")

# Obsolete foliage/debug presentation is no longer accepted merely because it is hidden.
for needle in (
    "DestroySourceGroundCoverProxies",
    "DestroyDeveloperVisualMarkers",
    "Proxy->DestroyComponent();",
    "Marker->DestroyComponent();",
    "Label->DestroyComponent();",
    "PASS45_GROUND_COVER_PRIMITIVES_DESTROYED",
    "PASS45_DEVELOPER_WORLD_MARKERS_DESTROYED",
    "groundProxyComponents=0",
    "developerMarkers=0",
):
    require(foliage_guard, needle, "runtime obsolete-proxy destruction")
forbid(foliage_guard, "PASS10_GROUND_COVER_PROXY_RETIRED", "obsolete hide-only foliage evidence")

# Roads/Sidewalks, the five park-path proxies and the visible general Fences family must be converted to committed
# authored meshes before Gate K. Bounds-origin compensation and long-axis matching preserve factual geo/topology
# instead of blindly stretching a mesh across whichever axis happens to be longest.
for needle in (
    "UOCAuthoredWorldSurfaceUpgradeSubsystem",
    "Before visual acceptance and before the Pass12 12-second stability baseline",
    "exactly five central-park path transforms",
    "ParkPaths",
    "SM_Stonepath_Var01",
    "ParkDetails remains reserved",
    "visible Fences family",
):
    require(surface_h, needle, "authored world upgrade header")
for needle in (
    "/Game/Scene_RoadsideConstruction/Assets/Custom/Urb_Roa_Asphalt_01/SM_Urb_Roa_Asphalt_01.SM_Urb_Roa_Asphalt_01",
    "/Game/Scene_RoadsideConstruction/Assets/Custom/Urb_Roa_Sidewalk_01/SM_Urb_Roa_Sidewalk_01.SM_Urb_Roa_Sidewalk_01",
    "/Game/AdvancedVillagePack/Meshes/SM_Stonepath_Var01.SM_Stonepath_Var01",
    "/Game/AdvancedVillagePack/Meshes/SM_Fence_Var01.SM_Fence_Var01",
    "IsEngineCube",
    "BuildExpectedParkPathProxySpecs",
    "SeparateParkPathFamily",
    'FindISM(Sector, TEXT("ParkPaths"))',
    'NewObject<UInstancedStaticMeshComponent>(Sector, TEXT("ParkPaths"))',
    "SourceIndices.Num() != 5",
    "SourceTransforms.Num() != 5",
    "RemainingInSidewalks != 0",
    "park_path_preflight_not_exactly_five",
    "PASS45_PARK_PATH_OWNERSHIP_READY",
    "sidewalk_park_path_matches=0",
    "const FVector DesiredSizeCm = OldScale * 100.0f;",
    "const bool bDesiredLongAxisY",
    "bNativeLongAxisY != bDesiredLongAxisY",
    "NewBounds.Origin.X * NewScale.X",
    "Component->SetStaticMesh(AuthoredMesh);",
    "Component->EmptyOverrideMaterials();",
    "UpdateInstanceTransform",
    "ElapsedSeconds < 0.75f",
    'FindISM(Sector, TEXT("Fences"))',
    "PASS45_AUTHORED_WORLD_SURFACE_CONTENT_GAP",
    "PASS45_AUTHORED_WORLD_SURFACE_FAIL",
    "PASS45_AUTHORED_ROAD_SURFACE_READY",
    "PASS45_AUTHORED_PARK_PATH_SURFACE_READY",
    "park_paths_mesh=SM_Stonepath_Var01",
    "park_path_instances=%d",
    "bounds_aware_upgrade=1",
    "PASS45_AUTHORED_WORLD_FENCE_READY",
    "fence_mesh=SM_Fence_Var01",
    "basicshape_meshes=0",
    "basicshape_material_overrides=0",
    "topology_preserved=1",
):
    require(surface, needle, "authored Roads/Sidewalks/ParkPaths/Fences upgrade")
forbid(surface, 'FindISM(Sector, TEXT("ParkDetails"))', "ParkDetails must not own the authored park-path replacement")

# There are factually five source park-path Cube transforms today: four central alleys + the CultureParkNorth link.
# They remain deterministic source topology, but runtime ownership must remove all five from Sidewalks before either
# family is upgraded. This catches an accidental sixth path or a regression that leaves one of the five in Sidewalks.
park_begin = world.find("void AOCWorldSectorOster::BuildCentralPark()")
park_end = world.find("\nvoid AOCWorldSectorOster::BuildCollegeSector()", park_begin)
if park_begin < 0 or park_end < 0:
    raise SystemExit("PASS45 GATE K SOURCE VERIFY FAIL: cannot isolate BuildCentralPark")
park_source = world[park_begin:park_end]
if park_source.count("AddBox(Sidewalks,") != 5:
    raise SystemExit(
        "PASS45 GATE K SOURCE VERIFY FAIL: BuildCentralPark must contain exactly five park-path Sidewalk source proxies"
    )
if surface.count("Specs.Add({") != 5:
    raise SystemExit(
        "PASS45 GATE K SOURCE VERIFY FAIL: ParkPaths migration must describe exactly five park-path transforms"
    )
for needle in (
    "Park + FVector(0, 0, 14)",
    "Park + FVector(0, -300, 14)",
    "Park + FVector(1800, 900, 14)",
    "Park + FVector(-2300, 1300, 14)",
    "Mid + FVector(0, 0, 15)",
    "CultureParkNorthAnchor()",
):
    require(surface, needle, "exact ParkPaths migration signature")

# Pass12 no longer certifies Roads/Sidewalks by demanding BasicShape Color MIDs. Ground remains an explicit
# legacy terrain gap while road/sidewalk stability requires authored mesh identity and non-BasicShape material.
for needle in (
    "HasColorMID(FindPrimitive(Sector, TEXT(\"Ground\"))",
    "HasAuthoredSurface(",
    "SM_Urb_Roa_Asphalt_01",
    "SM_Urb_Roa_Sidewalk_01",
    "authored_surface_basicshape_material_",
    "PASS45_WORLD_MATERIAL_BASELINE_READY ground_legacy_mid=1 authored_surface_families=2 basicshape_road_materials=0",
    "PASS45_WORLD_MATERIAL_STABLE ground_legacy_mid=1 authored_surface_families=2",
):
    require(stability, needle, "Pass12 authored surface stability migration")

# Gate K observes final presentation after cleanup and cannot mutate a bad scene into a pass.
for needle in (
    "UOCVisualFidelityGateKSubsystem",
    "UTickableWorldSubsystem",
    "This does not mutate scenery",
):
    require(guard_h, needle, "Gate K header")
for needle in (
    'TEXT("R13_StadionOsterAuthoritative")',
    'TEXT("/Engine/BasicShapes/")',
    "CountVisibleBasicShapes",
    "ElapsedSeconds < 3.0f",
    "PASS45_VISUAL_FIDELITY_CONTENT_GAP",
    "PASS45_GATE_K_RUNTIME_FAIL",
    "PASS45_GATE_K_RUNTIME_READY",
    "visible_basicshape_components=0",
    "gate_k_complete=1",
):
    require(guard, needle, "Gate K runtime observation")
forbid(guard, "SetVisibility(false", "Gate K must not hide rejected geometry")
forbid(guard, "DestroyComponent", "Gate K must not destroy rejected geometry")

# The main acceptance path must reject the known gap, rather than leaving Gate K in a side launcher.
for needle in (
    "VERIFY_PASS45_GATE_K_RUNTIME_LOG.py",
    "Verifying Gate K final-world visual truth",
    "Pass45 Gate K still contains visible BasicShape/proxy core content",
    "zero visible Engine BasicShape core content",
):
    require(launcher, needle, "main runtime Gate K wiring")

for needle in (
    "PASS45_GROUND_COVER_PRIMITIVES_DESTROYED",
    "PASS45_DEVELOPER_WORLD_MARKERS_DESTROYED",
    "PASS45_AUTHORED_VEGETATION_READY",
    "PASS45_AUTHORED_ROAD_SURFACE_READY",
    "PASS45_PARK_PATH_OWNERSHIP_READY",
    "PASS45_AUTHORED_PARK_PATH_SURFACE_READY",
    "PASS45_AUTHORED_WORLD_FENCE_READY",
    "PASS45_GATE_K_RUNTIME_READY",
    "PASS45_VISUAL_FIDELITY_CONTENT_GAP",
    "PASS45_GATE_K_RUNTIME_FAIL",
    "PASS45_AUTHORED_WORLD_SURFACE_CONTENT_GAP",
    "PASS45_AUTHORED_WORLD_SURFACE_FAIL",
):
    require(runtime_verify, needle, "strict Gate K log verifier")

# Gate K CI must actually run when the authored-world subsystem changes. Otherwise source truth could silently drift.
for needle in (
    "OsterConflict/Source/OsterConflict/Private/OCAuthoredWorldSurfaceUpgradeSubsystem.cpp",
    "OsterConflict/Source/OsterConflict/Public/OCAuthoredWorldSurfaceUpgradeSubsystem.h",
):
    if workflow.count(needle) < 2:
        raise SystemExit(f"PASS45 GATE K SOURCE VERIFY FAIL: workflow trigger missing pull_request/push coverage for {needle}")

# Visual cleanup must not be 'solved' by reducing native render scale or automatic quality.
require(visual_perf, "SetResolutionScaleValueEx(100.0f)", "native render scale contract")
require(visual_perf, "GameSettings->SetTextureQuality(3);", "texture quality contract")

print("PASS45 VISUAL FIDELITY GATE K SOURCE TRUTH PASS")
print("- obsolete ground-cover/debug presentation is physically removed at runtime")
print("- Roads/Sidewalks upgrade from Cube topology to tracked RoadsideConstruction authored surfaces before Pass12 baseline")
print("- exactly five park-path source proxies are separated from Sidewalks into ParkPaths and upgraded to SM_Stonepath_Var01")
print("- ParkPaths separation fails closed unless all five expected transforms move and zero remain under Sidewalks")
print("- visible AOCWorldSectorOster Fences upgrade to committed AdvancedVillagePack SM_Fence_Var01 with bounds-aware axis fitting")
print("- Gate K workflow is triggered by authored-world subsystem source/header changes")
print("- Pass12 certifies authored road materials rather than BasicShape Color MIDs")
print("- final-world Gate K is observation-only and fails closed on visible Engine BasicShape static meshes")
print("- main PASS45 runtime acceptance requires Gate K, not a side workflow")
print("- native 100% render scale / high texture contract remains intact")
print("- CURRENT CONTENT GAP: legacy Ground/ParkGeometry/ParkDetails and authoritative stadium/remaining core BasicShape families still block Gate K")
print("STATUS: ITEM 31 PARTIAL; Gate K cannot be marked complete until those authored replacements exist and runtime reports READY")
