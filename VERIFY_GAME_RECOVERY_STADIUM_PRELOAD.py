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
    "Museum R140 facade": SRC / "Private" / "OCR140MuseumFacadeDetailSubsystem.cpp",
    "Museum R142 entrance": SRC / "Private" / "OCR142MuseumEntranceDetailSubsystem.cpp",
    "Museum R143 vegetation": SRC / "Private" / "OCR143MuseumSiteVegetationSubsystem.cpp",
    "Museum R144 rear exterior": SRC / "Private" / "OCR144MuseumRearExteriorDetailSubsystem.cpp",
    "Museum R145 trees": SRC / "Private" / "OCR145MuseumTreeLayoutSubsystem.cpp",
}
SILPO_STAGE_PATHS = {
    "Silpo R140 shell": SRC / "Private" / "OCR140SilpoPhotoModelSubsystem.cpp",
    "Silpo R141 detail": SRC / "Private" / "OCR141SilpoDetailSubsystem.cpp",
    "Silpo R142 interior": SRC / "Private" / "OCR142SilpoInteriorDetailSubsystem.cpp",
    "Silpo R143 identity": SRC / "Private" / "OCR143SilpoFacadeIdentitySubsystem.cpp",
}
CULTURE_CPP = SRC / "Private" / "OCR146CultureHousePhotoModelSubsystem.cpp"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"GAME RECOVERY LANDMARK PRELOAD VERIFY FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit("GAME RECOVERY LANDMARK PRELOAD VERIFY FAIL: " + message)
    print("PASS:", message)


base_h = read(BASE_H)
activation_h = read(ACTIVATION_H)
activation_cpp = read(ACTIVATION_CPP)
coordinator_cpp = read(COORDINATOR_CPP)
deployment_cpp = read(DEPLOYMENT_CPP)
r138_cpp = read(R138_CPP)
museum_stages = {name: read(path) for name, path in MUSEUM_STAGE_PATHS.items()}
silpo_stages = {name: read(path) for name, path in SILPO_STAGE_PATHS.items()}
culture_cpp = read(CULTURE_CPP)

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
    require(path in activation_cpp, f"stadium preload contains {path}")

require("RequestAsyncLoad" in activation_cpp and "FSoftObjectPath" in activation_cpp,
        "stadium payload uses async soft-object preload")
require("LoadObject<" not in activation_cpp,
        "recovery stadium activation contains no blocking LoadObject")
require("UWorldSubsystem::OnWorldBeginPlay(InWorld);" in activation_cpp and
        "Super::OnWorldBeginPlay(InWorld);" not in activation_cpp,
        "recovery activation bypasses the quarantined eager base startup")
require("HasLoadCompleted()" in activation_cpp and "AreStadiumAssetsResolved" in activation_cpp,
        "stadium build waits for preload completion and resolved-object validation")
require("ApplyStadiumSurface(*World);" in activation_cpp,
        "preloaded recovery path reuses canonical stadium authoring")
require("R13_StadionOsterAuthoritative" in activation_cpp,
        "stadium readiness requires the canonical authored stadium actor")
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
        "R138 collision prerequisite remains explicit")
require("FSoftObjectPath" in r138_cpp and "ResolveObject()" in r138_cpp,
        "R138 consumes the resident collision cube")
require("LoadObject<" not in r138_cpp,
        "R138 contains no blocking LoadObject")
require("sync_load=0" in r138_cpp and "prerequisite_resident=1" in r138_cpp,
        "R138 records non-blocking prerequisite residency")

stadium_success_tail = activation_cpp.split("if (bPresentationReady)", 1)[1]
stadium_success_block = stadium_success_tail.split("else", 1)[0]
require("PreloadHandle.Reset();" not in stadium_success_block,
        "successful stadium preload remains resident through landmark startup")
require("PreloadHandle.Reset();" in stadium_success_tail.split("else", 1)[1],
        "failed stadium presentation releases its preload handle")

landmark_required_paths = (
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
    "/Game/Modular_Rural_Cabin/Meshes/Props/Power_Pole_1.Power_Pole_1",
    "/Game/Modular_Rural_Cabin/Meshes/Modular/Wall_Pillar.Wall_Pillar",
    "/Game/Modular_Rural_Cabin/Meshes/Modular/Door_01.Door_01",
    "/Game/Modular_Rural_Cabin/Meshes/Modular/Porch_Roof_8x4m.Porch_Roof_8x4m",
)
require("OutAssets.Reserve(29);" in deployment_cpp,
        "shared landmark preload documents the complete 29-asset payload")
for path in landmark_required_paths:
    require(path in deployment_cpp, f"shared landmark preload contains {path}")
require("RequestAsyncLoad" in deployment_cpp and "AreMuseumPreloadAssetsResolved" in deployment_cpp,
        "shared landmark payload is asynchronously loaded and validated")
for marker in (
    "full_landmark_chain=1",
    "resolved_assets=29",
    "prerequisite_residency=world_lifetime",
    "later_landmark_stages_resident=1",
    "synchronous_disk_load=0",
):
    require(marker in deployment_cpp, f"shared landmark preload marker present: {marker}")

landmark_success_block = deployment_cpp.split(
    "if (UOCR137MuseumPhotoModelSubsystem* MuseumSubsystem =", 1)[1].split(
        "void UOCPass45DeploymentStabilitySubsystem::EnsureDeploymentBackdrop", 1)[0]
require("MuseumPreloadHandle.Reset();" not in landmark_success_block,
        "successful shared landmark preload handle remains resident through R14.6")
require("MuseumPreloadHandle.Reset();" in deployment_cpp,
        "shared landmark preload handle is released on failure/deinitialize paths")

for stage_name, stage_cpp in museum_stages.items():
    require("LoadObject<" not in stage_cpp, f"{stage_name} contains no blocking LoadObject")
    require("FSoftObjectPath" in stage_cpp and "ResolveObject()" in stage_cpp,
            f"{stage_name} consumes resident soft-object payload")
    require("sync_load=0" in stage_cpp, f"{stage_name} reports non-blocking loading")
    require("prerequisite_resident=1" in stage_cpp, f"{stage_name} records prerequisite residency")

for stage_name, stage_cpp in silpo_stages.items():
    require("LoadObject<" not in stage_cpp, f"{stage_name} contains no blocking LoadObject")
    require("FSoftObjectPath" in stage_cpp and "ResolveObject()" in stage_cpp,
            f"{stage_name} consumes resident soft-object payload")
    require("sync_load=0" in stage_cpp, f"{stage_name} reports non-blocking loading")
    require("prerequisite_resident=1" in stage_cpp, f"{stage_name} records prerequisite residency")

require("LoadObject<" not in culture_cpp,
        "Culture House R14.6 contains no blocking LoadObject")
require("FSoftObjectPath" in culture_cpp and "ResolveObject()" in culture_cpp,
        "Culture House R14.6 consumes resident modular assets")
require("sync_load=0" in culture_cpp and "prerequisite_resident=1" in culture_cpp,
        "Culture House R14.6 records non-blocking prerequisite residency")

require('#include "OCGameRecoveryStadiumActivationSubsystem.h"' in coordinator_cpp,
        "landmark startup coordinator knows the stadium readiness owner")
require("World.GetSubsystem<UOCGameRecoveryStadiumActivationSubsystem>()" in coordinator_cpp,
        "landmark startup resolves the concrete stadium recovery subsystem")
require("!Stadium->IsStadiumPresentationReady()" in coordinator_cpp,
        "world readiness stalls until authoritative stadium readiness")
require("stadium_ready=1" in coordinator_cpp,
        "world-ready logging records stadium readiness")

print("GAME RECOVERY LANDMARK PRELOAD VERIFY PASS")
print("- stadium uses async preload and retains prerequisites")
print("- shared landmark preload owns 29 assets and keeps them resident through R14.6")
print("- Museum R138/R140/R142/R143/R144/R145 use ResolveObject only")
print("- Silpo R140/R141/R142/R143 use ResolveObject only")
print("- Culture House R146 uses ResolveObject only")
print("- staged landmark startup cannot reintroduce these blocking package loads")
print("STATUS: SOURCE/PRELOAD CONTRACT ONLY; UE 5.8 rendered acceptance remains pending")
