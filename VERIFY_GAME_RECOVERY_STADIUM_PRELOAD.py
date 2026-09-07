#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
BASE_H = SRC / "Public" / "OCR13StadiumSurfaceSubsystem.h"
ACTIVATION_H = SRC / "Public" / "OCGameRecoveryStadiumActivationSubsystem.h"
ACTIVATION_CPP = SRC / "Private" / "OCGameRecoveryStadiumActivationSubsystem.cpp"
COORDINATOR_CPP = SRC / "Private" / "OCLandmarkStartupCoordinatorSubsystem.cpp"
R138_CPP = SRC / "Private" / "OCR138MuseumInteractiveArchitectureSubsystem.cpp"


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
r138_cpp = read(R138_CPP)

require("UCLASS(Abstract)" in base_h,
        "historical stadium owner remains abstract and cannot revive its eager startup path")
require("protected:" in base_h and "void ApplyStadiumSurface(UWorld& World);" in base_h,
        "canonical authored stadium build is exposed only to the recovery activation subclass")
require("UCLASS()" in activation_h and
        "UOCGameRecoveryStadiumActivationSubsystem final : public UOCR13StadiumSurfaceSubsystem" in activation_h,
        "concrete GAME_RECOVERY stadium activation subsystem exists")
require("IsStadiumPresentationReady() const" in activation_h,
        "stadium activation exposes a readiness gate")

required_paths = (
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
for path in required_paths:
    require(path in activation_cpp, f"landmark preload contains {path}")

require("RequestAsyncLoad" in activation_cpp and "FSoftObjectPath" in activation_cpp,
        "stadium payload and shared landmark prerequisite use async soft-object preload")
require("LoadObject<" not in activation_cpp,
        "recovery activation path contains no blocking LoadObject")
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

success_tail = activation_cpp.split("if (bPresentationReady)", 1)[1]
success_block = success_tail.split("else", 1)[0]
require("PreloadHandle.Reset();" not in success_block,
        "successful landmark preload retains its handle through R138 startup")
require("PreloadHandle.Reset();" in success_tail.split("else", 1)[1],
        "failed stadium presentation releases the preload handle")

require('#include "OCGameRecoveryStadiumActivationSubsystem.h"' in coordinator_cpp,
        "landmark startup coordinator knows the stadium readiness owner")
require("World.GetSubsystem<UOCGameRecoveryStadiumActivationSubsystem>()" in coordinator_cpp,
        "landmark startup resolves the concrete stadium recovery subsystem")
require("!Stadium->IsStadiumPresentationReady()" in coordinator_cpp,
        "world readiness stalls until the authoritative stadium and shared landmark prerequisites exist")
require("stadium_ready=1" in coordinator_cpp,
        "world-ready logging records stadium readiness")

print("GAME RECOVERY STADIUM PRELOAD VERIFY PASS")
print("- quarantined historical stadium owner stays abstract")
print("- concrete recovery activation preloads the exact stadium payload asynchronously")
print("- R138 museum collision cube is preloaded, retained and consumed only through ResolveObject")
print("- staged museum collision startup can no longer issue a blocking LoadObject request")
print("- canonical stadium authoring is reused only after preload resolution")
print("- landmark/world readiness cannot complete before the stadium actor exists")
print("STATUS: SOURCE/PRELOAD CONTRACT ONLY; UE 5.8 rendered stadium/museum acceptance remains pending")
