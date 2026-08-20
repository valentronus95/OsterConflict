#include "OCRecoveredBuildingDetailsSubsystem.h"

#include "OCGameMode.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

namespace
{
    UInstancedStaticMeshComponent* MakeISM(AActor* Owner, USceneComponent* Root,
        UStaticMesh* Mesh, const FName Name)
    {
        if (!Owner || !Root || !Mesh) return nullptr;
        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(Owner, Name);
        if (!Component) return nullptr;
        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Mesh);
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(false);
        Component->SetCullDistances(0, 140000);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    void AddFitted(UInstancedStaticMeshComponent* Component, UStaticMesh* Mesh,
        const FVector& Location, const FVector& DesiredSizeCm, float YawDegrees)
    {
        if (!Component || !Mesh) return;
        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector NativeSize = Bounds.BoxExtent * 2.0f;
        if (NativeSize.X <= 1.0f || NativeSize.Y <= 1.0f || NativeSize.Z <= 1.0f) return;

        const FVector Scale(
            DesiredSizeCm.X / NativeSize.X,
            DesiredSizeCm.Y / NativeSize.Y,
            DesiredSizeCm.Z / NativeSize.Z);
        const FQuat Rotation = FRotator(0.0f, YawDegrees, 0.0f).Quaternion();
        const FVector FittedLocation = Location - Rotation.RotateVector(Bounds.Origin * Scale);
        Component->AddInstance(FTransform(Rotation, FittedLocation, Scale), true);
    }
}

bool UOCRecoveredBuildingDetailsSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCRecoveredBuildingDetailsSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (InWorld.GetNetMode() == NM_DedicatedServer) return;
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle DelayTimer;
    InWorld.GetTimerManager().SetTimer(DelayTimer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) Populate(*World);
        }), 0.35f, false);
}

void UOCRecoveredBuildingDetailsSubsystem::Populate(UWorld& World)
{
    if (bPopulated) return;
    bPopulated = true;

    UStaticMesh* Floor = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Scene_UnfinishedBuilding/Assets/Custom/Ind_Unf__FloorPlane_01/SM_Ind_Unf_FloorPlane_01.SM_Ind_Unf_FloorPlane_01"));
    UStaticMesh* Stair = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Scene_UnfinishedBuilding/Assets/Custom/Ind_Unf__StairBase_01/SM_Ind_Unf_StairBase_01.SM_Ind_Unf_StairBase_01"));
    UStaticMesh* Wall02 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Scene_UnfinishedBuilding/Assets/Custom/Ind_Unf__Wall_02/SM_Ind_Unf_Wall_02.SM_Ind_Unf_Wall_02"));
    UStaticMesh* Window = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Scene_UnfinishedBuilding/Assets/Custom/Ind_Unf__Window_01/SM_Ind_Unf_Window_01.SM_Ind_Unf_Window_01"));

    if (!Floor && !Stair && !Wall02 && !Window) return;

    FActorSpawnParameters Params;
    Params.Name = TEXT("OC_RecoveredBuildingDetails");
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AActor* Actor = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, Params);
    if (!Actor) return;
    Actor->Tags.Add(TEXT("OC_RecoveredBuildingDetails"));

    USceneComponent* Root = NewObject<USceneComponent>(Actor, TEXT("RecoveredBuildingDetailsRoot"));
    Actor->AddInstanceComponent(Root);
    Root->RegisterComponent();
    Actor->SetRootComponent(Root);

    UInstancedStaticMeshComponent* Floors = MakeISM(Actor, Root, Floor, TEXT("ConstructionFloors"));
    UInstancedStaticMeshComponent* Stairs = MakeISM(Actor, Root, Stair, TEXT("ConstructionStairs"));
    UInstancedStaticMeshComponent* Walls = MakeISM(Actor, Root, Wall02, TEXT("ConstructionWallDetails"));
    UInstancedStaticMeshComponent* Windows = MakeISM(Actor, Root, Window, TEXT("ConstructionWindows"));

    const FVector Site(-69000.0f, 64500.0f, 0.0f);

    // Match the shell already created by OCRecoveredEnvironmentSubsystem.
    AddFitted(Floors, Floor, Site + FVector(0.0f, 0.0f, 10.0f), FVector(2200.0f, 1500.0f, 24.0f), 0.0f);
    AddFitted(Floors, Floor, Site + FVector(0.0f, 120.0f, 320.0f), FVector(2100.0f, 1350.0f, 22.0f), 0.0f);

    AddFitted(Stairs, Stair, Site + FVector(-480.0f, -430.0f, 145.0f), FVector(800.0f, 450.0f, 300.0f), 90.0f);

    // Use the alternate wall module to close the two side edges while retaining the open front.
    AddFitted(Walls, Wall02, Site + FVector(-1050.0f, 0.0f, 160.0f), FVector(1400.0f, 45.0f, 320.0f), 90.0f);
    AddFitted(Walls, Wall02, Site + FVector(1050.0f, 0.0f, 160.0f), FVector(1400.0f, 45.0f, 320.0f), 90.0f);

    // Authored window modules replace generic holes on the rear/side elevations.
    AddFitted(Windows, Window, Site + FVector(-520.0f, 1030.0f, 175.0f), FVector(360.0f, 50.0f, 220.0f), 90.0f);
    AddFitted(Windows, Window, Site + FVector(520.0f, 1030.0f, 175.0f), FVector(360.0f, 50.0f, 220.0f), 90.0f);
    AddFitted(Windows, Window, Site + FVector(-1030.0f, 350.0f, 175.0f), FVector(360.0f, 50.0f, 220.0f), 0.0f);

    UE_LOG(LogTemp, Display,
        TEXT("Recovered unfinished building expanded with floors, stair, side walls and windows."));
}
