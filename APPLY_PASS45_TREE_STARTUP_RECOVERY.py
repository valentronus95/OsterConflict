#!/usr/bin/env python3
from __future__ import annotations

from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parent
WORLD = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCWorldSectorOster.cpp"
TZ = ROOT / "PASS45_RUNTIME_RECOVERY_TZ.md"
HISTORY = ROOT / "PASS45_RUNTIME_RECOVERY_HISTORY.md"
RUN_ALL = ROOT / "RUN_ALL_VERIFY.py"
VERIFIER = ROOT / "VERIFY_PASS45_TREE_STARTUP_DEFERRED.py"
WORKFLOW = ROOT / ".github" / "workflows" / "pass45-tree-startup-deferred.yml"

START_HEAD = "eee743320bb9474d59621e8a7580eaecab700bba"
BRANCH = "fix/pass45-runtime-rejection-material-closure-20260826"
PR = "#94"


def fail(message: str) -> None:
    raise SystemExit(f"PASS45 TREE STARTUP PATCH FAIL: {message}")


def replace_once(text: str, old: str, new: str, label: str) -> str:
    count = text.count(old)
    if count != 1:
        fail(f"{label}: expected exactly one source match, found {count}")
    return text.replace(old, new, 1)


def apply_source_patch() -> None:
    world = WORLD.read_text(encoding="utf-8")

    world = replace_once(
        world,
        '#include "Engine/StaticMesh.h"\n#include "Materials/MaterialInterface.h"',
        '#include "Engine/AssetManager.h"\n#include "Engine/StaticMesh.h"\n#include "Engine/StreamableManager.h"\n#include "Materials/MaterialInterface.h"\n#include "Misc/CommandLine.h"\n#include "Misc/Parse.h"\n#include "UObject/SoftObjectPath.h"',
        "async-load includes",
    )

    old_constructor_tree_load = '''    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> DeciduousTreeMesh(
        TEXT("/Game/KiteDemo/Environments/Trees/HillTree_02/HillTree_02.HillTree_02"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Pine01Mesh(
        TEXT("/Game/KiteDemo/Environments/Trees/ScotsPine_01/ScotsPine_01.ScotsPine_01"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Pine03Mesh(
        TEXT("/Game/KiteDemo/Environments/Trees/ScotsPineTall_01/ScotsPineTall_01.ScotsPineTall_01"));

    if (CubeMesh.Succeeded())
    {
        Ground->SetStaticMesh(CubeMesh.Object);
        UInstancedStaticMeshComponent* CubeComponents[] =
        {
            Roads, Sidewalks, ParkPaths, Buildings, ResidentialRoofs, ResidentialDetails,
            LandmarkBlocks, LandmarkRoofs, LandmarkWindows, LandmarkDetails,
            Fences, WoodFences, MetalFences, LightSheetFences, StadiumGeometry, StadiumDetails,
            ParkGeometry, ParkCentralGround, ParkNorthCivicGround, CollegeRecreationGround,
            ParkDetails, ParkMemorialPlaza, ParkMemorialSurface, ParkMemorialMonument,
            ParkMemorialApproach, ParkSkateFitness, ParkSkateSurface, ParkSkateRamps, ParkBenches,
            GrassMown, GrassRough, GrassWetland,
            Waterways, Bridges, ReferenceMarkers
        };
        for (UInstancedStaticMeshComponent* Component : CubeComponents)
        {
            Component->SetStaticMesh(CubeMesh.Object);
        }
    }

    if (DeciduousTreeMesh.Succeeded()) AuthoredDeciduousTrees->SetStaticMesh(DeciduousTreeMesh.Object);
    if (Pine01Mesh.Succeeded()) AuthoredPine01Trees->SetStaticMesh(Pine01Mesh.Object);
    if (Pine03Mesh.Succeeded()) AuthoredPine03Trees->SetStaticMesh(Pine03Mesh.Object);

    UInstancedStaticMeshComponent* AuthoredTrees[] =
'''
    new_constructor_tree_load = '''    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));

    if (CubeMesh.Succeeded())
    {
        Ground->SetStaticMesh(CubeMesh.Object);
        UInstancedStaticMeshComponent* CubeComponents[] =
        {
            Roads, Sidewalks, ParkPaths, Buildings, ResidentialRoofs, ResidentialDetails,
            LandmarkBlocks, LandmarkRoofs, LandmarkWindows, LandmarkDetails,
            Fences, WoodFences, MetalFences, LightSheetFences, StadiumGeometry, StadiumDetails,
            ParkGeometry, ParkCentralGround, ParkNorthCivicGround, CollegeRecreationGround,
            ParkDetails, ParkMemorialPlaza, ParkMemorialSurface, ParkMemorialMonument,
            ParkMemorialApproach, ParkSkateFitness, ParkSkateSurface, ParkSkateRamps, ParkBenches,
            GrassMown, GrassRough, GrassWetland,
            Waterways, Bridges, ReferenceMarkers
        };
        for (UInstancedStaticMeshComponent* Component : CubeComponents)
        {
            Component->SetStaticMesh(CubeMesh.Object);
        }
    }

    // PASS45 P0 startup recovery: do not synchronously resolve the large KiteDemo tree packages from the
    // native actor constructor/CDO. UE 5.8 was compiling HillTree_02/ScotsPine dependencies before the first
    // rendered frame and could sit indefinitely in "Waiting for static meshes to be ready" after material errors.
    // Exact tree identities remain owned by this actor, but their risky asset load is deferred/quarantined below.
    UInstancedStaticMeshComponent* AuthoredTrees[] =
'''
    world = replace_once(world, old_constructor_tree_load, new_constructor_tree_load, "constructor tree sync load")

    old_beginplay_tree_block = '''    // PASS45 item 27: the final player-facing tree family is selected during primary sector construction.
    // There is no late world-subsystem remap, transform rewrite or second mutating tree owner after BeginPlay.
    const int32 TreeInstances =
        (AuthoredDeciduousTrees ? AuthoredDeciduousTrees->GetInstanceCount() : 0) +
        (AuthoredPine01Trees ? AuthoredPine01Trees->GetInstanceCount() : 0) +
        (AuthoredPine03Trees ? AuthoredPine03Trees->GetInstanceCount() : 0);
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_REGIONAL_TREE_INTAKE_WIRED deciduous=HillTree_02 pine=ScotsPine_01 tall_pine=ScotsPineTall_01 families=3 instances=%d primary_authoring=1 late_mutation=0 imported_materials=1 runtime_acceptance=0"),
        TreeInstances);
'''
    new_beginplay_tree_block = '''    // PASS45 item 27 / 2026-08-31 P0 startup recovery. The exact tree family remains single-owner under this actor,
    // but UE 5.8 must never synchronously load these large KiteDemo packages from the constructor/CDO. The latest
    // factual Quick Normal run reached HillTree_02 static-mesh compilation after incompatible material diagnostics
    // and never produced a usable first frame. Default runtime therefore quarantines this family until its UE 5.8
    // material/static-mesh compatibility is repaired. Developers may opt in with -Pass45LoadKiteDemoTrees; that path
    // uses FStreamableManager and still carries runtime_acceptance=0 until direct visual evidence exists.
    const FSoftObjectPath DeciduousTreePath(
        TEXT("/Game/KiteDemo/Environments/Trees/HillTree_02/HillTree_02.HillTree_02"));
    const FSoftObjectPath Pine01TreePath(
        TEXT("/Game/KiteDemo/Environments/Trees/ScotsPine_01/ScotsPine_01.ScotsPine_01"));
    const FSoftObjectPath Pine03TreePath(
        TEXT("/Game/KiteDemo/Environments/Trees/ScotsPineTall_01/ScotsPineTall_01.ScotsPineTall_01"));
    const bool bAllowDeferredKiteDemoTreeLoad =
        FParse::Param(FCommandLine::Get(), TEXT("Pass45LoadKiteDemoTrees"));

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_REGIONAL_TREE_INTAKE_WIRED deciduous=HillTree_02 pine=ScotsPine_01 tall_pine=ScotsPineTall_01 families=3 instances=0 primary_authoring=1 late_mutation=0 async_initialization=1 startup_sync_tree_loads=0 opt_in=%d imported_materials=1 material_compatibility=pending runtime_acceptance=0"),
        bAllowDeferredKiteDemoTreeLoad ? 1 : 0);

    if (!bAllowDeferredKiteDemoTreeLoad)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("PASS45_KITEDEMO_TREE_STARTUP_QUARANTINED assets=HillTree_02,ScotsPine_01,ScotsPineTall_01 constructor_sync_loads=0 reason=ue58_material_static_mesh_compile_blocker normal_game_can_render_first=1 runtime_acceptance=0"));
    }
    else
    {
        const TArray<FSoftObjectPath> TreePaths = { DeciduousTreePath, Pine01TreePath, Pine03TreePath };
        const TWeakObjectPtr<AOCWorldSectorOster> WeakThis(this);
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_KITEDEMO_TREE_ASYNC_LOAD_REQUESTED assets=3 constructor_sync_loads=0 main_thread_wait_requested=0 runtime_acceptance=0"));

        UAssetManager::GetStreamableManager().RequestAsyncLoad(
            TreePaths,
            FStreamableDelegate::CreateLambda(
                [WeakThis, DeciduousTreePath, Pine01TreePath, Pine03TreePath]()
                {
                    AOCWorldSectorOster* Sector = WeakThis.Get();
                    if (!Sector) return;

                    UStaticMesh* DeciduousMesh = Cast<UStaticMesh>(DeciduousTreePath.ResolveObject());
                    UStaticMesh* Pine01Mesh = Cast<UStaticMesh>(Pine01TreePath.ResolveObject());
                    UStaticMesh* Pine03Mesh = Cast<UStaticMesh>(Pine03TreePath.ResolveObject());
                    if (!DeciduousMesh || !Pine01Mesh || !Pine03Mesh ||
                        !Sector->AuthoredDeciduousTrees || !Sector->AuthoredPine01Trees || !Sector->AuthoredPine03Trees)
                    {
                        UE_LOG(LogTemp, Error,
                            TEXT("PASS45_KITEDEMO_TREE_ASYNC_LOAD_CONTENT_GAP deciduous=%d pine01=%d pine03=%d runtime_acceptance=0"),
                            DeciduousMesh ? 1 : 0,
                            Pine01Mesh ? 1 : 0,
                            Pine03Mesh ? 1 : 0);
                        return;
                    }

                    Sector->AuthoredDeciduousTrees->SetStaticMesh(DeciduousMesh);
                    Sector->AuthoredPine01Trees->SetStaticMesh(Pine01Mesh);
                    Sector->AuthoredPine03Trees->SetStaticMesh(Pine03Mesh);
                    Sector->BuildVegetation();

                    const int32 TreeInstances =
                        Sector->AuthoredDeciduousTrees->GetInstanceCount() +
                        Sector->AuthoredPine01Trees->GetInstanceCount() +
                        Sector->AuthoredPine03Trees->GetInstanceCount();
                    UE_LOG(LogTemp, Display,
                        TEXT("PASS45_KITEDEMO_TREE_ASYNC_LOAD_READY assets=3 instances=%d primary_authoring=1 secondary_owner=0 constructor_sync_loads=0 material_compatibility=pending runtime_acceptance=0"),
                        TreeInstances);
                }));
    }
'''
    world = replace_once(world, old_beginplay_tree_block, new_beginplay_tree_block, "BeginPlay tree intake")

    old_vegetation_prefix = '''void AOCWorldSectorOster::BuildVegetation()
{
    enum class ETreeFamily : uint8 { Deciduous, Pine };

    auto AddAuthoredTree = [this](const FVector& Base, const float Scale, const ETreeFamily Family, const int32 Salt)
    {
        if (!IsPointInsidePlayableAuthoringBounds(Base, 600.0f)) return;

        UInstancedStaticMeshComponent* Component = AuthoredDeciduousTrees;
        float BaseHeightCm = 1650.0f;
        if (Family == ETreeFamily::Pine)
        {
            Component = (Salt & 1) == 0 ? AuthoredPine01Trees : AuthoredPine03Trees;
            BaseHeightCm = 2150.0f;
        }

        const float Yaw = FMath::Fmod(
            FMath::Abs(Base.X * 0.013f + Base.Y * 0.019f + static_cast<float>(Salt) * 47.0f), 360.0f);
        const float WidthScale = 0.94f + 0.035f * static_cast<float>(FMath::Abs(Salt) % 5);
        AddGroundedTree(Component, Base, BaseHeightCm * Scale, Yaw, WidthScale);
    };

    auto AddGrassPatch = [this](UInstancedStaticMeshComponent* Family, const FVector& Center, const FVector& Size, float Yaw)
    {
        // Source-only placeholder: very thin instanced boxes mark vegetation zones. Final S16C uses foliage/PCG meshes.
        AddBox(Family, Center + FVector(0,0,2.0f), FVector(Size.X, Size.Y, 4.0f), Yaw);
    };

    // S16B ground-cover zoning. Clean mown lawns are limited to maintained civic/sports spaces.
    const FVector Park = ParkAnchor();
    const FVector College = CollegeAnchor();
    const FVector Stadium = StadiumAnchor();
    AddGrassPatch(GrassMown, Park + FVector(0, 0, 0), FVector(19000, 14500, 4), 6.0f);
    AddGrassPatch(GrassMown, Stadium + FVector(0, 0, 0), FVector(14500, 9800, 4), 0.0f);
    AddGrassPatch(GrassMown, College + FVector(0, 5200, 0), FVector(12500, 7600, 4), 2.0f);

    // Road verges/private lots: irregular, partly mown grass rather than uniform golf-course lawn.
    const FVector RoughPatches[] = {
        FVector(-52000, 30000, 0), FVector(-52000,-25000,0), FVector(45000,30000,0),
        FVector(42000,-35000,0), FVector(-15000,70000,0), FVector(16000,-65000,0)
    };
    for (int32 I=0; I<UE_ARRAY_COUNT(RoughPatches); ++I)
        AddGrassPatch(GrassRough, RoughPatches[I], FVector(31000,22000,4), static_cast<float>((I%3)-1)*8.0f);

    // Pass 44 removes the old Desna/Oster wetland proxies outside the compact map. Water-edge vegetation
'''
    new_vegetation_prefix = '''void AOCWorldSectorOster::BuildVegetation()
{
    enum class ETreeFamily : uint8 { Deciduous, Pine };

    const bool bTreeMeshesReady =
        AuthoredDeciduousTrees && AuthoredDeciduousTrees->GetStaticMesh() &&
        AuthoredPine01Trees && AuthoredPine01Trees->GetStaticMesh() &&
        AuthoredPine03Trees && AuthoredPine03Trees->GetStaticMesh();

    // Constructor/CDO call: build only the lightweight source ground-cover zoning. Never dereference KiteDemo tree
    // bounds here. The exact tree meshes are either quarantined for normal runtime or initialized later by the same
    // actor through the explicit opt-in async path after a first frame can be produced.
    if (!bTreeMeshesReady)
    {
        auto AddGrassPatch = [this](UInstancedStaticMeshComponent* Family, const FVector& Center, const FVector& Size, float Yaw)
        {
            // Source-only placeholder: very thin instanced boxes mark vegetation zones. Final S16C uses foliage/PCG meshes.
            AddBox(Family, Center + FVector(0,0,2.0f), FVector(Size.X, Size.Y, 4.0f), Yaw);
        };

        const FVector Park = ParkAnchor();
        const FVector College = CollegeAnchor();
        const FVector Stadium = StadiumAnchor();
        AddGrassPatch(GrassMown, Park + FVector(0, 0, 0), FVector(19000, 14500, 4), 6.0f);
        AddGrassPatch(GrassMown, Stadium + FVector(0, 0, 0), FVector(14500, 9800, 4), 0.0f);
        AddGrassPatch(GrassMown, College + FVector(0, 5200, 0), FVector(12500, 7600, 4), 2.0f);

        const FVector RoughPatches[] = {
            FVector(-52000, 30000, 0), FVector(-52000,-25000,0), FVector(45000,30000,0),
            FVector(42000,-35000,0), FVector(-15000,70000,0), FVector(16000,-65000,0)
        };
        for (int32 I=0; I<UE_ARRAY_COUNT(RoughPatches); ++I)
            AddGrassPatch(GrassRough, RoughPatches[I], FVector(31000,22000,4), static_cast<float>((I%3)-1)*8.0f);

        UE_LOG(LogTemp, Display,
            TEXT("PASS45_TREE_STARTUP_DEFERRED_READY constructor_tree_meshes=0 ground_cover_only=1 tree_bounds_access=0 startup_sync_tree_loads=0 quarantine_default=1 runtime_acceptance=0"));
        return;
    }

    const int32 ExistingTreeInstances =
        AuthoredDeciduousTrees->GetInstanceCount() +
        AuthoredPine01Trees->GetInstanceCount() +
        AuthoredPine03Trees->GetInstanceCount();
    if (ExistingTreeInstances > 0)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_TREE_ASYNC_INSTANCE_BUILD_SKIPPED existing_instances=%d duplicate_tree_authoring=0 runtime_acceptance=0"),
            ExistingTreeInstances);
        return;
    }

    auto AddAuthoredTree = [this](const FVector& Base, const float Scale, const ETreeFamily Family, const int32 Salt)
    {
        if (!IsPointInsidePlayableAuthoringBounds(Base, 600.0f)) return;

        UInstancedStaticMeshComponent* Component = AuthoredDeciduousTrees;
        float BaseHeightCm = 1650.0f;
        if (Family == ETreeFamily::Pine)
        {
            Component = (Salt & 1) == 0 ? AuthoredPine01Trees : AuthoredPine03Trees;
            BaseHeightCm = 2150.0f;
        }

        const float Yaw = FMath::Fmod(
            FMath::Abs(Base.X * 0.013f + Base.Y * 0.019f + static_cast<float>(Salt) * 47.0f), 360.0f);
        const float WidthScale = 0.94f + 0.035f * static_cast<float>(FMath::Abs(Salt) % 5);
        AddGroundedTree(Component, Base, BaseHeightCm * Scale, Yaw, WidthScale);
    };

    const FVector Park = ParkAnchor();
    const FVector College = CollegeAnchor();
    const FVector Stadium = StadiumAnchor();

    // Pass 44 removes the old Desna/Oster wetland proxies outside the compact map. Water-edge vegetation
'''
    world = replace_once(world, old_vegetation_prefix, new_vegetation_prefix, "BuildVegetation startup split")

    WORLD.write_text(world, encoding="utf-8", newline="\n")

    tz = TZ.read_text(encoding="utf-8")
    tz = replace_once(
        tz,
        "Latest factual gameplay evidence: 2026-08-27  \nLatest runtime verdict: **RUNTIME REJECTED 2026-08-27**  ",
        "Latest factual gameplay evidence: 2026-08-31  \nLatest runtime verdict: **RUNTIME REJECTED 2026-08-31**  ",
        "TZ latest factual runtime header",
    )
    newest_section = '''## 0A. Latest P0 startup blocker — 2026-08-31 Quick Normal black screen

The latest factual local UE 5.8 evidence is now a startup rejection, newer than the 2026-08-27 rendered visual pack:

- `START_HERE.cmd` -> `1. ЗВИЧАЙНА ГРА` completed the incremental C++ build with `Result: Succeeded`;
- the direct `OsterConflict_Runtime` game process then opened a black window and never produced usable gameplay/UI;
- the log reached `PASS45_RENDER_BUDGET_READY` and `PASS45_DAYLIGHT_EXPOSURE_CONTRACT_READY`, then entered KiteDemo tree material/static-mesh work;
- observed material diagnostics include `Failed to compile Material for platform PCD3D_SM5`, `Node TransformPosition input must be a 3-component vector` and `SpeedTree node not currently supported for Skeletal Meshes`;
- the final blocking path visibly includes `Building static mesh HillTree_02` / `Waiting for static meshes to be ready`;
- source audit proves `AOCWorldSectorOster` synchronously resolved `HillTree_02`, `ScotsPine_01` and `ScotsPineTall_01` via `ConstructorHelpers::FObjectFinder` in the native actor constructor/CDO and immediately consumed tree bounds during `BuildVegetation()`.

P0 recovery rule from this evidence:

- no KiteDemo production-tree package may be synchronously resolved from the native constructor/CDO;
- normal runtime must be able to render its first frame without touching those rejected UE 5.8 material/static-mesh compile dependencies;
- the exact tree family remains a factual runtime/content gap while quarantined; source identity alone is not acceptance;
- an opt-in deferred async load may be used only for targeted repair evidence and remains `runtime_acceptance=0` until the material/static-mesh path and direct UE 5.8 visual result pass;
- PR #94 remains OPEN / UNMERGED.

The source recovery for this blocker is tracked by `VERIFY_PASS45_TREE_STARTUP_DEFERRED.py`. A new local Quick Normal launch is required before any startup/runtime status can improve.

'''
    anchor = "## 1. Latest factual runtime state — 2026-08-27\n"
    if newest_section not in tz:
        if anchor not in tz:
            fail("TZ section-1 anchor missing")
        tz = tz.replace(anchor, newest_section + anchor, 1)
    TZ.write_text(tz, encoding="utf-8", newline="\n")

    verifier = r'''#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
WORLD = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCWorldSectorOster.cpp"


def fail(message: str) -> None:
    raise SystemExit(f"PASS45 TREE STARTUP DEFERRED FAIL: {message}")


source = WORLD.read_text(encoding="utf-8", errors="replace")
try:
    constructor = source.split("AOCWorldSectorOster::AOCWorldSectorOster()", 1)[1].split(
        "void AOCWorldSectorOster::BeginPlay()", 1
    )[0]
except IndexError:
    fail("could not isolate AOCWorldSectorOster native constructor")

paths = (
    "/Game/KiteDemo/Environments/Trees/HillTree_02/HillTree_02.HillTree_02",
    "/Game/KiteDemo/Environments/Trees/ScotsPine_01/ScotsPine_01.ScotsPine_01",
    "/Game/KiteDemo/Environments/Trees/ScotsPineTall_01/ScotsPineTall_01.ScotsPineTall_01",
)
for path in paths:
    if path in constructor:
        fail(f"KiteDemo tree path is still synchronously reachable from constructor/CDO: {path}")
    if path not in source:
        fail(f"exact tree identity disappeared instead of being safely deferred: {path}")

for forbidden in (
    "FObjectFinder<UStaticMesh> DeciduousTreeMesh",
    "FObjectFinder<UStaticMesh> Pine01Mesh",
    "FObjectFinder<UStaticMesh> Pine03Mesh",
):
    if forbidden in source:
        fail(f"synchronous tree finder survived: {forbidden}")

for required in (
    '#include "Engine/AssetManager.h"',
    '#include "Engine/StreamableManager.h"',
    'FParse::Param(FCommandLine::Get(), TEXT("Pass45LoadKiteDemoTrees"))',
    "RequestAsyncLoad(",
    "ResolveObject()",
    "PASS45_TREE_STARTUP_DEFERRED_READY",
    "PASS45_KITEDEMO_TREE_STARTUP_QUARANTINED",
    "PASS45_KITEDEMO_TREE_ASYNC_LOAD_REQUESTED",
    "PASS45_KITEDEMO_TREE_ASYNC_LOAD_READY",
    "startup_sync_tree_loads=0",
    "material_compatibility=pending",
    "runtime_acceptance=0",
):
    if required not in source:
        fail(f"startup-deferred tree contract missing {required!r}")

veg = source.split("void AOCWorldSectorOster::BuildVegetation()", 1)[1]
for required in (
    "const bool bTreeMeshesReady",
    "if (!bTreeMeshesReady)",
    "ground_cover_only=1",
    "tree_bounds_access=0",
    "return;",
):
    if required not in veg:
        fail(f"BuildVegetation no longer protects constructor-time tree bounds: {required!r}")

if veg.find("if (!bTreeMeshesReady)") > veg.find("AddGroundedTree(Component"):
    fail("tree readiness guard occurs after authored-tree bounds/instance path")

if "LoadObject<UStaticMesh>" in source and any(path in source for path in paths):
    # Exact paths are permitted only as FSoftObjectPath values; reject any direct static-mesh LoadObject use.
    for line in source.splitlines():
        if "LoadObject<UStaticMesh>" in line and "KiteDemo" in line:
            fail("direct synchronous LoadObject for KiteDemo tree returned")

print("PASS45 TREE STARTUP DEFERRED PASS")
print("- native constructor/CDO no longer contains HillTree_02/ScotsPine synchronous object finders")
print("- normal runtime quarantines the rejected UE 5.8 tree compile path before first-frame startup")
print("- exact assets remain available only through explicit opt-in FStreamableManager async loading")
print("- BuildVegetation constructs ground-cover zoning without touching tree bounds until meshes are loaded")
print("STATUS: SOURCE STARTUP RECOVERY ONLY; UE 5.8 Quick Normal rerun is still mandatory")
'''
    VERIFIER.write_text(verifier, encoding="utf-8", newline="\n")

    workflow = '''name: Pass 45 tree startup deferred

on:
  push:
    paths:
      - "OsterConflict/Source/OsterConflict/Private/OCWorldSectorOster.cpp"
      - "VERIFY_PASS45_TREE_STARTUP_DEFERRED.py"
      - ".github/workflows/pass45-tree-startup-deferred.yml"
      - "RUN_ALL_VERIFY.py"
  pull_request:
    paths:
      - "OsterConflict/Source/OsterConflict/Private/OCWorldSectorOster.cpp"
      - "VERIFY_PASS45_TREE_STARTUP_DEFERRED.py"
      - ".github/workflows/pass45-tree-startup-deferred.yml"
      - "RUN_ALL_VERIFY.py"

jobs:
  verify:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - uses: actions/setup-python@v5
        with:
          python-version: "3.x"
      - name: Verify constructor-safe deferred tree startup
        run: python VERIFY_PASS45_TREE_STARTUP_DEFERRED.py
'''
    WORKFLOW.parent.mkdir(parents=True, exist_ok=True)
    WORKFLOW.write_text(workflow, encoding="utf-8", newline="\n")

    run_all = RUN_ALL.read_text(encoding="utf-8")
    entry = "    'VERIFY_PASS45_TREE_STARTUP_DEFERRED.py',\n"
    if entry not in run_all:
        marker = "    'VERIFY_PASS45_FAST_PREVIEW_PROGRESS.py',\n"
        if marker not in run_all:
            fail("RUN_ALL verifier insertion anchor missing")
        run_all = run_all.replace(marker, marker + entry, 1)
    RUN_ALL.write_text(run_all, encoding="utf-8", newline="\n")

    print("PASS45 tree startup source patch staged")


def append_history(source_sha: str) -> None:
    if len(source_sha) != 40:
        fail(f"invalid source SHA for history: {source_sha}")

    history = HISTORY.read_text(encoding="utf-8")
    history = history.replace(
        "- Latest factual local runtime verdict carried by PR #94: **RUNTIME REJECTED 2026-08-27**.",
        "- Latest factual local runtime verdict carried by PR #94: **RUNTIME REJECTED 2026-08-31**.",
        1,
    )
    section = f'''\n## Work cycle — 2026-08-31 P0 Quick Normal black-screen / KiteDemo tree startup recovery\n\n- Canonical checklist relevance: P0 UE 5.8 startup/runtime recovery plus item 27 vegetation truth. This cycle fixes the startup ownership/loading defect only; it does not accept the quarantined tree materials or item 27 visuals.\n- Branch / PR: `{BRANCH}` / {PR}.\n- Start GitHub head: `{START_HEAD}`. PR remained OPEN / UNMERGED.\n- Substantive source head: `{source_sha}` — `PASS45: defer KiteDemo tree startup load`.\n- Latest factual local runtime evidence: `START_HERE.cmd` -> `1. ЗВИЧАЙНА ГРА` completed the incremental C++ build with `Result: Succeeded`, then the direct game process stayed on a black window and had to be terminated. The log reached `PASS45_RENDER_BUDGET_READY` and `PASS45_DAYLIGHT_EXPOSURE_CONTRACT_READY`, emitted KiteDemo PCD3D_SM5 material compile failures including `Node TransformPosition input must be a 3-component vector` and `SpeedTree node not currently supported for Skeletal Meshes`, then entered `Building static mesh HillTree_02` / `Waiting for static meshes to be ready`.\n- Root cause confirmed in source: `AOCWorldSectorOster` synchronously loaded `HillTree_02`, `ScotsPine_01` and `ScotsPineTall_01` with native-constructor `ConstructorHelpers::FObjectFinder`. `BuildVegetation()` immediately depended on those meshes/bounds. This made the world-sector CDO pull the rejected KiteDemo material/static-mesh compile chain before a usable first frame. The preceding Stein-prepass isolation commit had independently recorded the same runtime-module/CDO tree-load coupling under NullRHI.\n- Production correction: the three KiteDemo paths are removed from the native constructor/CDO. Constructor-time `BuildVegetation()` now creates only lightweight ground-cover zoning and explicitly returns before any tree-bounds access. Normal runtime quarantines the exact KiteDemo tree family by default so first-frame startup does not touch the rejected compile chain.\n- Controlled diagnostic route: `-Pass45LoadKiteDemoTrees` is an explicit opt-in only. It resolves the same exact three paths through `FStreamableManager::RequestAsyncLoad` after `BeginPlay`, keeps the same `AOCWorldSectorOster` owner, creates no second mutation subsystem and still records `material_compatibility=pending runtime_acceptance=0`.\n- Regression guard/workflow: `VERIFY_PASS45_TREE_STARTUP_DEFERRED.py` + `.github/workflows/pass45-tree-startup-deferred.yml`; cumulative `RUN_ALL_VERIFY.py` includes the guard. The guard rejects any return of the three KiteDemo paths or named tree `FObjectFinder`s inside the constructor/CDO and requires the quarantine/async evidence contract.\n- Canonical TZ updated to `RUNTIME REJECTED 2026-08-31` with a dedicated latest-startup-blocker section.\n- Runtime state after source correction: **RUNTIME PENDING / latest factual verdict remains RUNTIME REJECTED 2026-08-31**. No new local UE 5.8 Quick Normal launch has yet proven that the black-screen startup is gone.\n- Runtime acceptance: **not credited**. PR #94 remains OPEN / UNMERGED and must not merge before a new current-head UE 5.8 runtime acceptance.\n- Official checklist accounting remains **22/36 = 61.1%**, **38.9% remaining**. Item 27 remains unchecked because the exact tree family is intentionally quarantined pending UE 5.8 material/static-mesh repair and direct visual acceptance.\n'''
    if "P0 Quick Normal black-screen / KiteDemo tree startup recovery" not in history:
        history = history.rstrip() + "\n" + section
    HISTORY.write_text(history, encoding="utf-8", newline="\n")
    print(f"PASS45 history appended for source head {source_sha}")


def main() -> None:
    if len(sys.argv) < 2:
        fail("usage: apply | history <source_sha>")
    if sys.argv[1] == "apply":
        apply_source_patch()
        return
    if sys.argv[1] == "history" and len(sys.argv) == 3:
        append_history(sys.argv[2])
        return
    fail("usage: apply | history <source_sha>")


if __name__ == "__main__":
    main()
