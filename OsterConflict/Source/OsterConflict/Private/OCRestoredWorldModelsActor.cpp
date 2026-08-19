#include "OCRestoredWorldModelsActor.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
    UInstancedStaticMeshComponent* MakeVisualISM(AActor* Owner, USceneComponent* Root, const TCHAR* Name)
    {
        UInstancedStaticMeshComponent* Component = Owner->CreateDefaultSubobject<UInstancedStaticMeshComponent>(FName(Name));
        Component->SetupAttachment(Root);
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetCollisionProfileName(TEXT("NoCollision"));
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(false);
        return Component;
    }
}

AOCRestoredWorldModelsActor::AOCRestoredWorldModelsActor()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    ForestPath = MakeVisualISM(this, SceneRoot, TEXT("RecoveredForestPath"));
    RoadGround = MakeVisualISM(this, SceneRoot, TEXT("RecoveredRoadGround"));
    RoadAsphalt = MakeVisualISM(this, SceneRoot, TEXT("RecoveredRoadAsphalt"));
    UnfinishedFloor = MakeVisualISM(this, SceneRoot, TEXT("RecoveredUnfinishedFloor"));
    UnfinishedWall = MakeVisualISM(this, SceneRoot, TEXT("RecoveredUnfinishedWall"));
    UnfinishedPillar = MakeVisualISM(this, SceneRoot, TEXT("RecoveredUnfinishedPillar"));
    UnfinishedStair = MakeVisualISM(this, SceneRoot, TEXT("RecoveredUnfinishedStair"));
    CementBag = MakeVisualISM(this, SceneRoot, TEXT("RecoveredCementBag"));
    CableWheel = MakeVisualISM(this, SceneRoot, TEXT("RecoveredCableWheel"));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> ForestPathMesh(
        TEXT("/Game/TileableForestRoad/Meshes/SM_Forest_Path.SM_Forest_Path"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> RoadGroundMesh(
        TEXT("/Game/Scene_RoadsideConstruction/Assets/Custom/Urb_Roa_Ground_01/SM_Urb_Roa_Ground_01.SM_Urb_Roa_Ground_01"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> RoadAsphaltMesh(
        TEXT("/Game/Scene_RoadsideConstruction/Assets/Custom/Urb_Roa_Asphalt_01/SM_Urb_Roa_Asphalt_01.SM_Urb_Roa_Asphalt_01"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> FloorMesh(
        TEXT("/Game/Scene_UnfinishedBuilding/Assets/Custom/Ind_Unf__FloorPlane_01/SM_Ind_Unf_FloorPlane_01.SM_Ind_Unf_FloorPlane_01"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> WallMesh(
        TEXT("/Game/Scene_UnfinishedBuilding/Assets/Custom/Ind_Unf__Wall_01/SM_Ind_Unf_Wall_01.SM_Ind_Unf_Wall_01"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> PillarMesh(
        TEXT("/Game/Scene_UnfinishedBuilding/Assets/Custom/Ind_Unf__Pillar_01/SM_Ind_Unf_Pillar_01.SM_Ind_Unf_Pillar_01"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> StairMesh(
        TEXT("/Game/Scene_UnfinishedBuilding/Assets/Custom/Ind_Unf__StairBase_01/SM_Ind_Unf_StairBase_01.SM_Ind_Unf_StairBase_01"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CementBagMesh(
        TEXT("/Game/Scene_RoadsideConstruction/Assets/MS/3D/Ind_Con_Bag_Cement_Closed_01/SM_Ind_Con_Bag_Cement_Closed_01.SM_Ind_Con_Bag_Cement_Closed_01"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CableWheelMesh(
        TEXT("/Game/Scene_RoadsideConstruction/Assets/MS/3D/Ind_Con_CableWheel_Wood_L_01/SM_Ind_Con_CableWheel_Wood_L_01.SM_Ind_Con_CableWheel_Wood_L_01"));

    if (ForestPathMesh.Succeeded()) ForestPath->SetStaticMesh(ForestPathMesh.Object);
    if (RoadGroundMesh.Succeeded()) RoadGround->SetStaticMesh(RoadGroundMesh.Object);
    if (RoadAsphaltMesh.Succeeded()) RoadAsphalt->SetStaticMesh(RoadAsphaltMesh.Object);
    if (FloorMesh.Succeeded()) UnfinishedFloor->SetStaticMesh(FloorMesh.Object);
    if (WallMesh.Succeeded()) UnfinishedWall->SetStaticMesh(WallMesh.Object);
    if (PillarMesh.Succeeded()) UnfinishedPillar->SetStaticMesh(PillarMesh.Object);
    if (StairMesh.Succeeded()) UnfinishedStair->SetStaticMesh(StairMesh.Object);
    if (CementBagMesh.Succeeded()) CementBag->SetStaticMesh(CementBagMesh.Object);
    if (CableWheelMesh.Succeeded()) CableWheel->SetStaticMesh(CableWheelMesh.Object);
}

void AOCRestoredWorldModelsActor::Populate()
{
    if (bPopulated) return;
    bPopulated = true;

    BuildForestTracks();
    BuildUnfinishedSite();
    BuildRoadworksProps();
}

void AOCRestoredWorldModelsActor::AddInstance(UInstancedStaticMeshComponent* Component,
    const FVector& Location, float YawDegrees, const FVector& Scale)
{
    if (!Component || !Component->GetStaticMesh()) return;
    Component->AddInstance(FTransform(FRotator(0.0f, YawDegrees, 0.0f), Location, Scale));
}

void AOCRestoredWorldModelsActor::BuildForestTracks()
{
    // Generic outskirts only. These are not claimed as survey-exact roads.
    const FVector WestTrack(-86500.0f, 54500.0f, 3.0f);
    for (int32 Index = 0; Index < 7; ++Index)
    {
        AddInstance(ForestPath, WestTrack + FVector(Index * 900.0f, Index * 1450.0f, 0.0f), 58.0f);
    }

    const FVector SouthTrack(51500.0f, -70500.0f, 3.0f);
    for (int32 Index = 0; Index < 6; ++Index)
    {
        AddInstance(ForestPath, SouthTrack + FVector(Index * 1350.0f, Index * 650.0f, 0.0f), 26.0f);
    }
}

void AOCRestoredWorldModelsActor::BuildUnfinishedSite()
{
    // A deliberately generic construction site on the outer urban edge, away from named landmarks.
    const FVector Origin(66500.0f, 51500.0f, 5.0f);
    constexpr float Bay = 420.0f;
    constexpr float Storey = 320.0f;

    for (int32 X = 0; X < 4; ++X)
    {
        for (int32 Y = 0; Y < 3; ++Y)
        {
            AddInstance(UnfinishedFloor, Origin + FVector(X * Bay, Y * Bay, 0.0f));
            AddInstance(UnfinishedFloor, Origin + FVector(X * Bay, Y * Bay, Storey));
        }
    }

    for (int32 StoreyIndex = 0; StoreyIndex < 2; ++StoreyIndex)
    {
        const float Z = static_cast<float>(StoreyIndex) * Storey;
        for (int32 X = 0; X <= 4; ++X)
        {
            for (int32 Y = 0; Y <= 3; ++Y)
            {
                AddInstance(UnfinishedPillar, Origin + FVector(X * Bay, Y * Bay, Z));
            }
        }

        for (int32 X = 0; X < 4; ++X)
        {
            if (X == 1 || X == 2) continue;
            AddInstance(UnfinishedWall, Origin + FVector(X * Bay + Bay * 0.5f, 0.0f, Z), 0.0f);
            AddInstance(UnfinishedWall, Origin + FVector(X * Bay + Bay * 0.5f, 3.0f * Bay, Z), 0.0f);
        }
    }

    AddInstance(UnfinishedStair, Origin + FVector(Bay * 1.45f, Bay * 1.0f, 0.0f), 90.0f);
    AddInstance(UnfinishedStair, Origin + FVector(Bay * 2.45f, Bay * 2.0f, Storey), -90.0f);
}

void AOCRestoredWorldModelsActor::BuildRoadworksProps()
{
    // Small roadworks scene near the outer east-west approach, not on a landmark footprint.
    const FVector Origin(-64500.0f, -48500.0f, 4.0f);
    for (int32 Index = 0; Index < 5; ++Index)
    {
        AddInstance(RoadGround, Origin + FVector(Index * 850.0f, 0.0f, 0.0f), 0.0f);
    }
    for (int32 Index = 0; Index < 3; ++Index)
    {
        AddInstance(RoadAsphalt, Origin + FVector((Index + 1) * 850.0f, 900.0f, 0.0f), 0.0f);
    }

    AddInstance(CementBag, Origin + FVector(500.0f, 1350.0f, 0.0f), 23.0f);
    AddInstance(CementBag, Origin + FVector(650.0f, 1430.0f, 0.0f), -11.0f);
    AddInstance(CementBag, Origin + FVector(780.0f, 1330.0f, 0.0f), 41.0f);
    AddInstance(CableWheel, Origin + FVector(1850.0f, 1450.0f, 0.0f), 74.0f);
    AddInstance(CableWheel, Origin + FVector(2350.0f, 1280.0f, 0.0f), -19.0f, FVector(0.85f));
}
