#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
BASE_H = SRC / "Public" / "OCR13StadiumSurfaceSubsystem.h"
ACTIVATION_H = SRC / "Public" / "OCGameRecoveryStadiumActivationSubsystem.h"
ACTIVATION_CPP = SRC / "Private" / "OCGameRecoveryStadiumActivationSubsystem.cpp"
COORDINATOR_CPP = SRC / "Private" / "OCLandmarkStartupCoordinatorSubsystem.cpp"
DEPLOYMENT_CPP = SRC / "Private" / "OCPass45DeploymentStabilitySubsystem.cpp"
R138_CPP = SRC / "Private" / "OCR138MuseumInteractiveArchitectureSubsystem.cpp"
MUSEUM_STAGE_PATHS = {
    "R140 facade": SRC / "Private" / "OCR140MuseumFacadeDetailSubsystem.cpp",
    "R142 entrance": SRC / "Private" / "OCR142MuseumEntranceDetailSubsystem.cpp",
    "R143 vegetation": SRC / "Private" / "OCR143MuseumSiteVegetationSubsystem.cpp",
    "R144 rear exterior": SRC / "Private" / "OCR144MuseumRearExteriorDetailSubsystem.cpp",
    "R145 trees": SRC / "Private" / "OCR145MuseumTreeLayoutSubsystem.cpp",
}


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"GAME RECOVERY STADIUM PRELOAD VERIFY FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit("GAME RECOVERY STADIUM PRELOAD VERIFY FAIL: " + message)
    print("PASS:", message)


base_h = read(BASE_H)
activation_h = read(ACTIVATION_H)
activation_cpp = read(ACTIVATION_CPP)
coordinator_cpp = read(COORDINATOR_CPP)
deployment_cpp = read(DEPLOYMENT_CPP)
r138_cpp = read(R138_CPP)
museum_stages = {name: read(path) for name, path in MUSEUM_STAGE_PATHS.items()}

require("UCLASS(Abstract)" in base_h,
        "historical stadium owner remains abstract and cannot revive its eager startup path")
require("protected:" in base_h and "void ApplyStadiumSurface(UWorld& World);" in base_h,
        "canonical authored stadium build is exposed only to the recovery activation subclass")
require("UCLASS()" in activation_h and
        "UOCGameRecoveryStadiumActivationSubsystem final : public UOCR13StadiumSurfaceSubsystem" in activation_h,
        "concrete GAME_RECOVERY stadium activation subsystem exists")
require("IsStadiumPresentationReady() const" in activation_h,
        "stadium activation exposes a readiness gate")

stadium_required_paths = (
    "/Game/AdvancedVillagePack/Meshes/SM_Plane_1x1.SM_Plane_1x1",
    "/Game/Mega_Street_Props_Pack/Street_Props_pack_V2/Meshes/SM_Curb_1.SM_Curb_1",
    "/Game/Mega_Street_Props_Pack/Street_Props_Pack_V1/Mesh/SM_Sign_1.SM_Sign_1",
    "/Game/Mega_Street_Props_Pack/Street_Props_pack_V2/Materials/Instances/M_Grass_Inst.M_Grass_Inst",
    "/Game/Mega_Street_Props_Pack/Street_Props_Pack_V1/Materials/Instances/M_Color_1_Inst.M_Color_1_Inst",
    "/Game/Mega_Street_Props_Pack/Street_Props_pack_V2/Materials/Instances/M_Metal_3_Inst.M_Metal_3_Inst",
    "/Game/AdvancedVillagePack/Meshes/SM_House_Var01.SM_House_Var01",
    "/Game/AdvancedVillagePack/Meshes/SM_House_Var02.SM_House_Var02",
    "/Game/AdvancedVillagePack/Meshes/SM_Fence_Var01.SM_Fence_Var01",
    "/Game/AdvancedVillagePack/Meshes/SM_Fence_Var03.SM_Fence_Var03",
    "/Game/KiteDemo/Environments/Trees/HillTree_02/HillTree_02.HillTree_02",
    "/Game/KiteDemo/Environments/Trees/ScotsPineTall_01/ScotsPineTall_01.ScotsPineTall_01",
    "/Game/Scene_RoadsideConstruction/Assets/Custom/Urb_Roa_Ground_01/SM_Urb_Roa_Ground_01.SM_Urb_Roa_Ground_01",
    "/Game/Scene_RoadsideConstruction/Assets/Custom/Urb_Roa_Sidewalk_01/SM_Urb_Roa_Sidewalk_01.SM_Urb_Roa_Sidewalk_01",
    "/Engine/BasicShapes/Cube.Cube",
)
for path in stadium_required_paths:
    require(path in activation_cpp, f"stadium/landmark preload contains {path}")

require("RequestAsyncLoad" in activation_cpp and "FSoftObjectPath" in activation_cpp,
        "stadium payload and shared landmark prerequisite use async soft-object preload")
require("LoadObject<" not in activation_cpp,
        "recovery stadium activation path contains no blocking LoadObject")
require("UWorldSubsystem::OnWorldBeginPlay(InWorld);" in activation_cpp and
        "Super::OnWorldBeginPlay(InWorld);" not in activation_cpp,
        "recovery activation bypasses the quarantined eager base OnWorldBeginPlay")
require("HasLoadCompleted()" in activation_cpp and "AreStadiumAssetsResolved" in activation_cpp,
        "canonical stadium build runs only after preload completion and resolved-object validation")
require("ApplyStadiumSurface(*World);" in activation_cpp,
        "preloaded recovery path reuses the canonical authored stadium owner instead of duplicating it")
require("R13_StadionOsterAuthoritative" in activation_cpp,
        "readiness requires the canonical authored stadium actor")
for marker in (
    "GAME_RECOVERY_STADIUM_ASYNC_PRELOAD_BEGIN",
    "GAME_RECOVERY_STADIUM_ASYNC_PRELOAD_FAIL",
    "GAME_RECOVERY_STADIUM_PRESENTATION_READY",
    "museum_r138_collision_prerequisite=1",
    "prerequisite_residency=world_lifetime",
    "sync_fallback=0",
    "runtime_acceptance=0",
):
    require(marker in activation_cpp, f"stadium recovery marker present: {marker}")

require('/Engine/BasicShapes/Cube.Cube' in r138_cpp,
        "R138 collision prerequisite is still explicit")
require("FSoftObjectPath" in r138_cpp and "ResolveObject()" in r138_cpp,
        "R138 resolves the preloaded collision cube without a synchronous package request")
require("LoadObject<" not in r138_cpp,
        "R138 staged museum startup contains no blocking LoadObject")
require("sync_load=0" in r138_cpp and "prerequisite_resident=1" in r138_cpp,
        "R138 logs non-blocking prerequisite residency")
require('/Engine/BasicShapes/Cube.Cube' in activation_cpp,
        "R138 collision cube is resident before landmark startup may advance")

stadium_success_tail = activation_cpp.split("if (bPresentationReady)", 1)[1]
stadium_success_block = stadium_success_tail.split("else", 1)[0]
require("PreloadHandle.Reset();" not in stadium_success_block,
        "successful stadium preload retains its handle through landmark startup")
require("PreloadHandle.Reset();" in stadium_success_tail.split("else", 1)[1],
        "failed stadium presentation releases the preload handle")

museum_required_paths = (
    "/Game/Modular_Rural_Cabin/Meshes/Modular/Wall_8m.Wall_8m",
    "/Game/Modular_Rural_Cabin/Meshes/Modular/Wall_Window_4m.Wall_Window_4m",
    "/Game/Modular_Rural_Cabin/Meshes/Modular/Wall_Door_Windows_8m.Wall_Door_Windows_8m",
    "/Game/Modular_Rural_Cabin/Meshes/Modular/Wall_Top_4m.Wall_Top_4m",
    "/Game/Modular_Rural_Cabin/Meshes/Modular/Roof_Both_Ends_4m.Roof_Both_Ends_4m",
    "/Game/Modular_Rural_Cabin/Meshes/Modular/Bottom_Extender_4m.Bottom_Extender_4m",
    "/Game/Modular_Rural_Cabin/Meshes/Modular/Porch_4x4m.Porch_4x4m",
    "/Game/Modular_Rural_Cabin/Meshes/Modular/Window_Frame_Part.Window_Frame_Part",
    "/Game/Modular_Rural_Cabin/Materials/Instances/Metal_Roof.Metal_Roof",
    "/Game/Modular_Rural_Cabin/Materials/Instances/Wood_Planks_Painted_Blue.Wood_Planks_Painted_Blue",
    "/Game/KiteDemo/LevelContent/Architecture/SM_1Meter_01.SM_1Meter_01",
    "/Game/Mega_Street_Props_Pack/Street_Props_pack_V2/Materials/Instances/M_Concrete_2_Inst.M_Concrete_2_Inst",
    "/Game/KiteDemo/Environments/Trees/ScotsPineTall_01/ScotsPineTall_01.ScotsPineTall_01",
    "/Game/KiteDemo/Environments/Trees/HillTree_02/HillTree_02.HillTree_02",
    "/Game/KiteDemo/Environments/Trees/Vegetation_Debris_002/SM_Vegetation_Debris_002.SM_Vegetation_Debris_002",
    "/Engine/BasicShapes/Cube.Cube",
    "/Engine/BasicShapes/Cylinder.Cylinder",
    "/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial",
    "/Game/PN_FoliageCollection/Meshes/grassMesh/grass_01_03_mesh.grass_01_03_mesh",
    "/Game/PN_FoliageCollection/Meshes/grassMesh/grass_02_01_mesh.grass_02_01_mesh",
    "/Game/PN_FoliageCollection/Meshes/groundPlantMesh/ground_01_02.ground_01_02",
    "/Game/PN_FoliageCollection/Meshes/groundPlantMesh/ground_02_03.ground_02_03",
    "/Game/Modular_Rural_Cabin/Meshes/Foliage/SM_Pine_Tree_01.SM_Pine_Tree_01",
    "/Game/Modular_Rural_Cabin/Meshes/Foliage/SM_Pine_Tree_03.SM_Pine_Tree_03",
    "/Game/AdvancedVillagePack/Meshes/SM_Tree_Var01.SM_Tree_Var01",
)
require("OutAssets.Reserve(25);" in deployment_cpp,
        "museum preload capacity documents the complete 25-asset chain")
for path in museum_required_paths:
    require(path in deployment_cpp, f"museum full-chain preload contains {path}")
require("RequestAsyncLoad" in deployment_cpp and "AreMuseumPreloadAssetsResolved" in deployment_cpp,
        "museum chain is asynchronously loaded and validated before authoritative build")
for marker in (
    "full_chain=1",
    "resolved_assets=25",
    "prerequisite_residency=world_lifetime",
    "later_stages_resident=1",
    "synchronous_disk_load=0",
):
    require(marker in deployment_cpp, f"museum preload marker present: {marker}")

museum_success_block = deployment_cpp.split(
    "if (UOCR137MuseumPhotoModelSubsystem* MuseumSubsystem =", 1)[1].split(
        "void UOCPass45DeploymentStabilitySubsystem::EnsureDeploymentBackdrop", 1)[0]
require("MuseumPreloadHandle.Reset();" not in museum_success_block,
        "successful museum preload handle remains resident for R14.0-R14.5")
require("MuseumPreloadHandle.Reset();" in deployment_cpp,
        "museum preload handle is still released on failure/deinitialize paths")

for stage_name, stage_cpp in museum_stages.items():
    require("LoadObject<" not in stage_cpp,
            f"{stage_name} contains no blocking LoadObject")
    require("FSoftObjectPath" in stage_cpp and "ResolveObject()" in stage_cpp,
            f"{stage_name} consumes only resident soft-object payload")
    require("sync_load=0" in stage_cpp,
            f"{stage_name} reports the non-blocking load contract")
    require("prerequisite_resident=1" in stage_cpp,
            f"{stage_name} records successful prerequisite residency")

require('#include "OCGameRecoveryStadiumActivationSubsystem.h"' in coordinator_cpp,
        "landmark startup coordinator knows the stadium readiness owner")
require("World.GetSubsystem<UOCGameRecoveryStadiumActivationSubsystem>()" in coordinator_cpp,
        "landmark startup resolves the concrete stadium recovery subsystem")
require("!Stadium->IsStadiumPresentationReady()" in coordinator_cpp,
        "world readiness stalls until the authoritative stadium and shared landmark prerequisites exist")
require("stadium_ready=1" in coordinator_cpp,
        "world-ready logging records stadium readiness")

print("GAME RECOVERY LANDMARK PRELOAD VERIFY PASS")
print("- quarantined historical stadium owner stays abstract")
print("- concrete recovery activation preloads the exact stadium payload asynchronously")
print("- museum preload owns all 25 assets required through R14.5 and retains them for world lifetime")
print("- R138/R140/R142/R143/R144/R145 consume resident assets through ResolveObject only")
print("- staged landmark startup cannot reintroduce package loads in those museum stages")
print("- landmark/world readiness cannot complete before the authoritative stadium exists")
print("STATUS: SOURCE/PRELOAD CONTRACT ONLY; UE 5.8 rendered stadium/museum acceptance remains pending")
