#include "OCRecoveredEnvironmentSubsystem.h"

#include "OCGameMode.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

bool UOCRecoveredEnvironmentSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCRecoveredEnvironmentSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (InWorld.GetNetMode() == NM_DedicatedServer) return;
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle Timer;
    InWorld.GetTimerManager().SetTimer(Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) Populate(*World);
        }), 1.25f, false);
}

UInstancedStaticMeshComponent* UOCRecoveredEnvironmentSubsystem::CreateVisualISM(
    AActor* Owner, USceneComponent* Root, UStaticMesh* Mesh, const FName Name)
{
    if (!Owner || !Root || !Mesh) return nullptr;
    UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(Owner, Name);
    if (!Component) return nullptr;
    Component->SetupAttachment(Root);
    Component->SetStaticMesh(Mesh);
    Component->SetMobility(EComponentMobility::Static);
    Component->SetGenerateOverlapEvents(false);
    Component->SetCollisionProfileName(TEXT("NoCollision"));
    Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Component->SetCanEverAffectNavigation(false);
    Component->SetCullDistances(0, 140000);
    Owner->AddInstanceComponent(Component);
    Component->RegisterComponent();
    return Component;
}

void UOCRecoveredEnvironmentSubsystem::AddFittedInstance(UInstancedStaticMeshComponent* Component,
    UStaticMesh* Mesh, const FVector& Location, const FVector& DesiredSizeCm, const float YawDegrees)
{
    if (!Component || !Mesh) return;
    const FBoxSphereBounds Bounds = Mesh->GetBounds();
    const FVector NativeSize = Bounds.BoxExtent * 2.0f;
    if (NativeSize.X <= 1.0f || NativeSize.Y <= 1.0f || NativeSize.Z <= 1.0f) return;

    const FVector Scale(DesiredSizeCm.X / NativeSize.X, DesiredSizeCm.Y / NativeSize.Y, DesiredSizeCm.Z / NativeSize.Z);
    const FQuat Rotation = FRotator(0.0f, YawDegrees, 0.0f).Quaternion();
    const FVector FittedLocation = Location - Rotation.RotateVector(Bounds.Origin * Scale);
    Component->AddInstance(FTransform(Rotation, FittedLocation, Scale), true);
}

void UOCRecoveredEnvironmentSubsystem::Populate(UWorld& World)
{
    if (bPopulated) return;
    bPopulated = true;

    UStaticMesh* ForestPathMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/TileableForestRoad/Meshes/SM_Forest_Path.SM_Forest_Path"));
    UStaticMesh* WallMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Scene_UnfinishedBuilding/Assets/Custom/Ind_Unf__Wall_01/SM_Ind_Unf_Wall_01.SM_Ind_Unf_Wall_01"));
    UStaticMesh* PillarMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Scene_UnfinishedBuilding/Assets/Custom/Ind_Unf__Pillar_01/SM_Ind_Unf_Pillar_01.SM_Ind_Unf_Pillar_01"));
    if (!ForestPathMesh && !WallMesh && !PillarMesh) return;

    FActorSpawnParameters Params;
    Params.Name = TEXT("OC_RecoveredEnvironmentModels_R13");
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AActor* Actor = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, Params);
    if (!Actor) return;
    Actor->Tags.Add(TEXT("OC_RecoveredEnvironment"));

    USceneComponent* Root = NewObject<USceneComponent>(Actor, TEXT("RecoveredEnvironmentRoot"));
    Actor->AddInstanceComponent(Root);
    Root->RegisterComponent();
    Actor->SetRootComponent(Root);

    UInstancedStaticMeshComponent* Paths = CreateVisualISM(Actor, Root, ForestPathMesh, TEXT("RecoveredForestPaths"));
    UInstancedStaticMeshComponent* Walls = CreateVisualISM(Actor, Root, WallMesh, TEXT("RecoveredConstructionWalls"));
    UInstancedStaticMeshComponent* Pillars = CreateVisualISM(Actor, Root, PillarMesh, TEXT("RecoveredConstructionPillars"));

    // Keep every restored model inside the compact R13 supported ground instead of using the old full-map outskirts.
    if (Paths && ForestPathMesh)
    {
        AddFittedInstance(Paths, ForestPathMesh, FVector(-67000.0f, -18500.0f, 4.0f), FVector(9200.0f, 4100.0f, 180.0f), 24.0f);
        AddFittedInstance(Paths, ForestPathMesh, FVector(-56000.0f, -12500.0f, 4.0f), FVector(9000.0f, 4000.0f, 180.0f), 31.0f);
        AddFittedInstance(Paths, ForestPathMesh, FVector(15500.0f, 44500.0f, 4.0f), FVector(9400.0f, 4200.0f, 180.0f), -38.0f);
    }

    // Outer test site near Team One's compact edge, away from named central landmarks.
    const FVector Site(-58500.0f, 48500.0f, 0.0f);
    if (Walls && WallMesh)
    {
        AddFittedInstance(Walls, WallMesh, Site + FVector(-600.0f, 0.0f, 160.0f), FVector(1200.0f, 45.0f, 320.0f), 0.0f);
        AddFittedInstance(Walls, WallMesh, Site + FVector(600.0f, 0.0f, 160.0f), FVector(1200.0f, 45.0f, 320.0f), 0.0f);
        AddFittedInstance(Walls, WallMesh, Site + FVector(0.0f, 1050.0f, 160.0f), FVector(2100.0f, 45.0f, 320.0f), 90.0f);
    }
    if (Pillars && PillarMesh)
    {
        for (const FVector& Offset : { FVector(-1050.0f,-700.0f,160.0f), FVector(1050.0f,-700.0f,160.0f),
            FVector(-1050.0f,700.0f,160.0f), FVector(1050.0f,700.0f,160.0f) })
        {
            AddFittedInstance(Pillars, PillarMesh, Site + Offset, FVector(45.0f,45.0f,320.0f), 0.0f);
        }
    }

    UE_LOG(LogTemp, Display, TEXT("R13 recovered environment activated: forest paths and unfinished-building meshes."));
}
