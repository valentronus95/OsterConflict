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


def require_file(path: Path, label: str) -> None:
    if not path.is_file():
        raise SystemExit(f"PASS45 GATE K SOURCE VERIFY FAIL: {label}: missing {path.relative_to(ROOT)}")


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
latest_runtime_evidence = read(ROOT / "RUNTIME_EVIDENCE" / "2026-08-27_PASS45_REJECTED" / "README.md")

# Canonical world source still carries Engine Cube topology for families that are deterministically upgraded before
# Gate K. The authoritative stadium is stricter: its visible presentation owner may no longer source Engine BasicShapes.
require(world, '/Engine/BasicShapes/Cube.Cube', "current world source topology")
require(world, '/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial', "current world source material gap")
forbid(stadium, '/Engine/BasicShapes/', "authoritative stadium BasicShape regression")
for needle in (
    "/Game/AdvancedVillagePack/Meshes/SM_Plane_1x1.SM_Plane_1x1",
    "/Game/Mega_Street_Props_Pack/Street_Props_pack_V2/Meshes/SM_Curb_1.SM_Curb_1",
    "/Game/Mega_Street_Props_Pack/Street_Props_Pack_V1/Mesh/SM_Sign_1.SM_Sign_1",
    "/Game/Mega_Street_Props_Pack/Street_Props_pack_V2/Materials/Instances/M_Grass_Inst.M_Grass_Inst",
    "/Game/Mega_Street_Props_Pack/Street_Props_Pack_V1/Materials/Instances/M_Color_1_Inst.M_Color_1_Inst",
    "/Game/Mega_Street_Props_Pack/Street_Props_pack_V2/Materials/Instances/M_Metal_3_Inst.M_Metal_3_Inst",
    "Target->GetStaticMesh()",
    "PASS45_STADIUM_SURFACE_PARTIAL_AUTHORED_READY",
    "authored_surface_families=7",
    "remaining_basicshape_families=0",
    "bounds_aware_box_fit=1",
    "gate_k_complete=0",
    "runtime_acceptance=0",
):
    require(stadium, needle, "authored stadium presentation")

# Stadium has one primary site owner. Its tree perimeter must use the same final KiteDemo families as the
# primary Oster world authoring; no post-BeginPlay remap or older village tree family may survive there.
for needle in (
    "/Game/KiteDemo/Environments/Trees/HillTree_02/HillTree_02.HillTree_02",
    "/Game/KiteDemo/Environments/Trees/ScotsPineTall_01/ScotsPineTall_01.ScotsPineTall_01",
    "PASS45_STADIUM_TREE_INTAKE_WIRED",
    "primary_authoring=1",
    "late_mutation=0",
    "runtime_acceptance=0",
):
    require(stadium, needle, "stadium primary tree authoring")
for retired_tree_path in (
    "/Game/AdvancedVillagePack/Meshes/SM_Tree_Var01.SM_Tree_Var01",
    "/Game/AdvancedVillagePack/Meshes/SM_Tree_Var04.SM_Tree_Var04",
):
    forbid(stadium, retired_tree_path, "retired stadium tree authoring")

# Ground and stadium authored contracts must point only at assets that are actually tracked in this repository.
require_file(
    ROOT / "OsterConflict" / "Content" / "AdvancedVillagePack" / "Meshes" / "SM_Plane_1x1.uasset",
    "tracked authored ground/pitch mesh",
)
require_file(
    ROOT / "OsterConflict" / "Content" / "KiteDemo" / "Environments" / "GroundTiles" / "Grass" / "M_Ground_Grass2.uasset",
    "tracked authored ground material",
)
for path, label in (
    (
        ROOT / "OsterConflict" / "Content" / "Mega_Street_Props_Pack" / "Street_Props_pack_V2" / "Meshes" / "SM_Curb_1.uasset",
        "tracked stadium bar/line mesh",
    ),
    (
        ROOT / "OsterConflict" / "Content" / "Mega_Street_Props_Pack" / "Street_Props_Pack_V1" / "Mesh" / "SM_Sign_1.uasset",
        "tracked stadium entrance mesh",
    ),
    (
        ROOT / "OsterConflict" / "Content" / "Mega_Street_Props_Pack" / "Street_Props_pack_V2" / "Materials" / "Instances" / "M_Grass_Inst.uasset",
        "tracked stadium turf material",
    ),
    (
        ROOT / "OsterConflict" / "Content" / "Mega_Street_Props_Pack" / "Street_Props_Pack_V1" / "Materials" / "Instances" / "M_Color_1_Inst.uasset",
        "tracked stadium color base material",
    ),
    (
        ROOT / "OsterConflict" / "Content" / "Mega_Street_Props_Pack" / "Street_Props_pack_V2" / "Materials" / "Instances" / "M_Metal_3_Inst.uasset",
        "tracked stadium sports metal material",
    ),
):
    require_file(path, label)

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

# Ground, Roads/Sidewalks, the five source-owned ParkPaths and visible general Fences are preloaded while deployment
# owns the screen and then materialized one authored family per tick before possession. This staged contract replaces
# the obsolete single-frame 0.75-second mutation window while preserving the same fail-closed final visual truth.
for needle in (
    "UOCAuthoredWorldSurfaceUpgradeSubsystem",
    "UTickableWorldSubsystem",
    "pre-spawn world preparation",
    "async",
    "one authored surface family per tick before possession",
    "player is never used as a loading screen",
    "IsWorldSurfaceReady",
    "GetWorldSurfaceProgress",
    "TSharedPtr<FStreamableHandle> PreloadHandle;",
    "PreparationStage",
):
    require(surface_h, needle, "staged authored world upgrade header")
for needle in (
    "/Game/AdvancedVillagePack/Meshes/SM_Plane_1x1.SM_Plane_1x1",
    "/Game/KiteDemo/Environments/GroundTiles/Grass/M_Ground_Grass2.M_Ground_Grass2",
    "/Game/Scene_RoadsideConstruction/Assets/Custom/Urb_Roa_Asphalt_01/SM_Urb_Roa_Asphalt_01.SM_Urb_Roa_Asphalt_01",
    "/Game/Scene_RoadsideConstruction/Assets/Custom/Urb_Roa_Sidewalk_01/SM_Urb_Roa_Sidewalk_01.SM_Urb_Roa_Sidewalk_01",
    "/Game/AdvancedVillagePack/Meshes/SM_Stonepath_Var01.SM_Stonepath_Var01",
    "/Game/AdvancedVillagePack/Meshes/SM_Fence_Var01.SM_Fence_Var01",
    "FindStaticMeshComponent",
    "UpgradeGroundSurface",
    'FindStaticMeshComponent(Sector, TEXT("Ground"))',
    "const float OldTopZ",
    "Component->SetRelativeTransform",
    "ground_top_z_preserved=1",
    "playable_footprint_preserved=1",
    "PASS45_AUTHORED_GROUND_SURFACE_READY",
    "IsEngineCube",
    "BuildExpectedParkPathProxySpecs",
    "SeparateParkPathFamily",
    'FindISM(Sector, TEXT("ParkPaths"))',
    "ExistingParkPaths->GetInstanceCount() != 5",
    "RemainingInSidewalks != 0",
    'NewObject<UInstancedStaticMeshComponent>(Sector, TEXT("ParkPaths"))',
    "SourceIndices.Num() != 5",
    "SourceTransforms.Num() != 5",
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
    "BuildSurfacePreloadPaths",
    "RequestAsyncLoad",
    "PreloadHandle->HasLoadCompleted()",
    "PreparationStage == 0",
    "PreparationStage == 1",
    "PreparationStage == 2",
    "PreparationStage == 3",
    "PreparationStage == 4",
    "PreparationStage == 5",
    "GAME_RECOVERY_SURFACE_STAGE_READY",
    "GAME_RECOVERY_SURFACE_PREP_FINISH",
    "one_family_per_tick=1",
    "pre_spawn=1",
    "sync_package_loads=0",
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
    require(surface, needle, "staged authored Ground/Roads/Sidewalks/ParkPaths/Fences upgrade")
forbid(surface, "ElapsedSeconds < 0.75f", "retired single-frame authored-world mutation window")
forbid(surface, 'FindISM(Sector, TEXT("ParkDetails"))', "ParkDetails must not own the authored park-path replacement")

# Canonical current source owns exactly five ParkPaths: four central alleys + the CultureParkNorth link.
# Sidewalks may no longer own any of those five. The upgrader keeps its exact five-transform fallback only for
# backward compatibility with older map instances, and must validate zero matching transforms remain in Sidewalks.
park_begin = world.find("void AOCWorldSectorOster::BuildCentralPark()")
park_end = world.find("\nvoid AOCWorldSectorOster::BuildCollegeSector()", park_begin)
if park_begin < 0 or park_end < 0:
    raise SystemExit("PASS45 GATE K SOURCE VERIFY FAIL: cannot isolate BuildCentralPark")
park_source = world[park_begin:park_end]
if park_source.count("AddBox(ParkPaths,") != 5:
    raise SystemExit(
        "PASS45 GATE K SOURCE VERIFY FAIL: BuildCentralPark must source-own exactly five ParkPaths proxies"
    )
if park_source.count("AddBox(Sidewalks,") != 0:
    raise SystemExit(
        "PASS45 GATE K SOURCE VERIFY FAIL: central-park path topology leaked back into Sidewalks"
    )
for needle in (
    "ExpectedParkPaths = 5",
    "PASS45_SOURCE_PARK_PATH_OWNERSHIP_READY",
    "park_path_instances=5",
    "authored_in_sidewalks=0",
    "runtime_migration_required=0",
):
    require(park_source, needle, "canonical ParkPaths source ownership")
if surface.count("Specs.Add({") != 5:
    raise SystemExit(
        "PASS45 GATE K SOURCE VERIFY FAIL: ParkPaths compatibility matcher must describe exactly five expected transforms"
    )
for needle in (
    "Park + FVector(0, 0, 14)",
    "Park + FVector(0, -300, 14)",
    "Park + FVector(1800, 900, 14)",
    "Park + FVector(-2300, 1300, 14)",
    "Mid + FVector(0,0,15)",
    "CultureParkNorthAnchor()",
):
    require(world, needle, "canonical ParkPaths source signature")
for needle in (
    "Park + FVector(0, 0, 14)",
    "Park + FVector(0, -300, 14)",
    "Park + FVector(1800, 900, 14)",
    "Park + FVector(-2300, 1300, 14)",
    "Mid + FVector(0, 0, 15)",
):
    require(surface, needle, "exact ParkPaths compatibility signature")

# Gate K semantic split regression: mixed ParkGeometry/ParkMemorialPlaza/ParkSkateFitness buckets are now
# quarantine-only. Exact ground/memorial/skate families are direct AOCWorldSectorOster primary owners, preserving
# the 23-detail contract without a late normalization bridge. Monument + two ramps remain explicit content gaps.
for needle in (
    'ParkGeometry = MakeISM(TEXT("ParkGeometry")',
    'ParkCentralGround = MakeISM(TEXT("ParkCentralGround")',
    'ParkNorthCivicGround = MakeISM(TEXT("ParkNorthCivicGround")',
    'CollegeRecreationGround = MakeISM(TEXT("CollegeRecreationGround")',
    'ParkMemorialPlaza = MakeISM(TEXT("ParkMemorialPlaza")',
    'ParkMemorialSurface = MakeISM(TEXT("ParkMemorialSurface")',
    'ParkMemorialMonument = MakeISM(TEXT("ParkMemorialMonument")',
    'ParkMemorialApproach = MakeISM(TEXT("ParkMemorialApproach")',
    'ParkSkateFitness = MakeISM(TEXT("ParkSkateFitness")',
    'ParkSkateSurface = MakeISM(TEXT("ParkSkateSurface")',
    'ParkSkateRamps = MakeISM(TEXT("ParkSkateRamps")',
    'ParkBenches = MakeISM(TEXT("ParkBenches")',
    "ExpectedMemorialSurface = 1",
    "ExpectedMemorialMonument = 1",
    "ExpectedMemorialApproach = 4",
    "ExpectedSkateSurface = 1",
    "ExpectedSkateRamps = 2",
    "ExpectedBenches = 14",
    "ExpectedSemanticDetails == 23",
    "LegacyDetailsCount == 0",
    "LegacyGeometryCount == 0",
    "LegacyMemorialCount == 0",
    "LegacySkateCount == 0",
    "PASS45_GATE_K_PARK_SEMANTIC_SPLIT_REJECTED",
    "PASS45_GATE_K_PARK_SEMANTIC_SPLIT_READY",
    "PASS45_PARK_PRIMARY_SEMANTIC_OWNERS_READY",
    "primary_authoring=1 normalization_bridge=0",
    "remaining_content_gap_instances=3",
):
    require(world, needle, "Central Park primary semantic detail split")
for forbidden in (
    "AddBox(ParkDetails,",
    "AddBoxRotated(ParkDetails,",
    "AddBox(ParkGeometry,",
    "AddBox(ParkMemorialPlaza,",
    "AddBox(ParkSkateFitness,",
    "AddBoxRotated(ParkSkateFitness,",
):
    forbid(park_source, forbidden, "legacy mixed Central Park authoring")
for needle in (
    "AddBox(ParkCentralGround,",
    "AddBox(ParkMemorialSurface,",
    "AddBox(ParkMemorialMonument,",
    "AddBox(ParkMemorialApproach,",
    "AddBox(ParkSkateSurface,",
    "AddBoxRotated(ParkSkateRamps,",
    "AddBox(ParkBenches,",
):
    require(park_source, needle, "primary semantic Central Park detail authoring")

# Pass12 rejects the old Ground Color MID contract as well. It validates Ground + the four upgraded ISM surface
# families and tracks ParkPaths/Fences counts so late mutation cannot escape the stability gate.
for needle in (
    "HasAuthoredGroundSurface(",
    "SM_Plane_1x1",
    "M_Ground_Grass2",
    "HasAuthoredSurface(",
    "SM_Urb_Roa_Asphalt_01",
    "SM_Urb_Roa_Sidewalk_01",
    "SM_Stonepath_Var01",
    "SM_Fence_Var01",
    'TEXT("ParkPaths")',
    'TEXT("Fences")',
    "authored_surface_basicshape_material_",
    "PASS45_WORLD_MATERIAL_BASELINE_READY ground_authored=1 authored_surface_families=5 basicshape_surface_materials=0",
    "PASS45_WORLD_MATERIAL_STABLE ground_authored=1 authored_surface_families=5",
):
    require(stability, needle, "Pass12 authored surface stability migration")
forbid(stability, "ground_legacy_mid=1", "retired Ground BasicShape MID acceptance")
forbid(stability, 'HasColorMID(FindPrimitive(Sector, TEXT("Ground"))', "retired Ground Color MID validation")

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
    "PASS45_GATE_K_PARK_SEMANTIC_SPLIT_READY",
    "PASS45_AUTHORED_GROUND_SURFACE_READY",
    "PASS45_AUTHORED_ROAD_SURFACE_READY",
    "PASS45_PARK_PATH_OWNERSHIP_READY",
    "PASS45_AUTHORED_PARK_PATH_SURFACE_READY",
    "PASS45_AUTHORED_WORLD_FENCE_READY",
    "PASS45_GATE_K_RUNTIME_READY",
    "PASS45_GATE_K_PARK_SEMANTIC_SPLIT_REJECTED",
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

# A source-green result cannot overrule the user's newest UE screenshots.
for needle in ("RUNTIME REJECTED", "2026-08-27"):
    require(latest_runtime_evidence, needle, "latest runtime evidence authority")

print("PASS45 VISUAL FIDELITY GATE K SOURCE TRUTH PASS")
print("- obsolete ground-cover/debug presentation is physically removed at runtime")
print("- Central Park semantic ownership is primary/fail-closed: legacy mixed buckets=0; exact groups=1/1/4/1/2/14 (23 total)")
print("- park central/north/college ground owners are direct source families; old ParkGeometry remains quarantine-only")
print("- playable Ground and authored ISM families are async-preloaded and materialized one family per tick before possession")
print("- Ground playable footprint and top-Z are preserved by bounds-aware replacement")
print("- Roads/Sidewalks upgrade from Cube topology to tracked RoadsideConstruction authored surfaces before Pass12 baseline")
print("- canonical source owns exactly five ParkPaths and zero central-park path proxies remain in Sidewalks")
print("- existing bounds-aware upgrader validates the five paths and upgrades them to SM_Stonepath_Var01")
print("- legacy runtime ParkPaths migration remains compatibility-only; current source does not depend on it")
print("- visible AOCWorldSectorOster Fences upgrade to committed AdvancedVillagePack SM_Fence_Var01 with bounds-aware axis fitting")
print("- Pass12 tracks ParkPaths/Fences and validates five authored surface families")
print("- Gate K workflow is triggered by authored-world subsystem source/header changes")
print("- final-world Gate K is observation-only and fails closed on visible Engine BasicShape static meshes")
print("- main PASS45 runtime acceptance requires Gate K, not a side workflow")
print("- native 100% render scale / high texture contract remains intact")
print("- authoritative Stadion Oster presentation now owns zero Engine BasicShape families at source level")
print("- stadium pitch/lines/sports metal/entrance use tracked authored meshes with bounds-aware fitting; runtime visual acceptance remains pending")
print("- latest factual runtime verdict remains RUNTIME REJECTED 2026-08-27")
print("- CURRENT CONTENT GAP: ParkMemorialMonument + two ParkSkateRamps instances and other non-stadium core BasicShape families still block Gate K")
print("STATUS: ITEM 32 PARTIAL; primary park ownership is source-closed, but Gate K cannot be complete until remaining exact authored replacements and runtime READY evidence exist")
