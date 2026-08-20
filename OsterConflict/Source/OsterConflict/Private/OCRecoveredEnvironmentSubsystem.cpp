#include "OCRecoveredEnvironmentSubsystem.h"

#include "OCGameMode.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

namespace
{
    constexpr int32 MaxAttachAttempts = 40;
    constexpr float RetryDelaySeconds = 0.25f;
    const FName RecoveredEnvironmentTag(TEXT("OC_RecoveredEnvironment"));
}

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
    InWorld.GetTimerManager().SetTimerForNextTick(
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) TryPopulate(*World);
        }));
}

void UOCRecoveredEnvironmentSubsystem::TryPopulate(UWorld& World)
{
    if (bPopulated) return;

    // Give the authoritative runtime sector/GameMode a chance to finish BeginPlay first.
    if (!World.HasBegunPlay() && AttachAttempts < MaxAttachAttempts)
    {
        ++AttachAttempts;
        TWeakObjectPtr<UWorld> WeakWorld(&World);
        FTimerHandle RetryTimer;
        World.GetTimerManager().SetTimer(RetryTimer,
            FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
            {
                if (UWorld* RetryWorld = WeakWorld.Get()) TryPopulate(*RetryWorld);
            }), RetryDelaySeconds, false);
        return;
    }

    Populate(World);
}

UInstancedStaticMeshComponent* UOCRecoveredEnvironmentSubsystem::CreateVisualISM(
    AActor* Owner, USceneComponent* Root, UStaticMesh* Mesh, const FName Name, const bool bCollision)
{
    if (!Owner || !Root || !Mesh) return nullptr;

    UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(Owner, Name);
    if (!Component) return nullptr;

    Component->SetupAttachment(Root);
    Component->SetStaticMesh(Mesh);
    Component->SetMobility(EComponentMobility::Static);
    Component->SetGenerateOverlapEvents(false);
    Component->SetCollisionProfileName(FName(bCollision ? TEXT("BlockAll") : TEXT("NoCollision")));
    Component->SetCollisionEnabled(bCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
    Component->SetCanEverAffectNavigation(bCollision);
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

    const FVector Scale(
        DesiredSizeCm.X / NativeSize.X,
        DesiredSizeCm.Y / NativeSize.Y,
        DesiredSizeCm.Z / NativeSize.Z);
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
    UStaticMesh* UnfinishedWallMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Scene_UnfinishedBuilding/Assets/Custom/Ind_Unf__Wall_01/SM_Ind_Unf_Wall_01.SM_Ind_Unf_Wall_01"));
    UStaticMesh* UnfinishedPillarMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Scene_UnfinishedBuilding/Assets/Custom/Ind_Unf__Pillar_01/SM_Ind_Unf_Pillar_01.SM_Ind_Unf_Pillar_01"));

    if (!ForestPathMesh && !UnfinishedWallMesh && !UnfinishedPillarMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("Recovered environment pass skipped: restored meshes were not loadable."));
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Name = TEXT("OC_RecoveredEnvironmentModels");
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AActor* Decorator = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, SpawnParams);
    if (!Decorator) return;
    Decorator->Tags.Add(RecoveredEnvironmentTag);

    USceneComponent* Root = NewObject<USceneComponent>(Decorator, TEXT("RecoveredEnvironmentRoot"));
    Decorator->AddInstanceComponent(Root);
    Root->RegisterComponent();
    Decorator->SetRootComponent(Root);

    UInstancedStaticMeshComponent* ForestPaths =
        CreateVisualISM(Decorator, Root, ForestPathMesh, TEXT("RecoveredForestPaths"), false);
    UInstancedStaticMeshComponent* ConstructionWalls =
        CreateVisualISM(Decorator, Root, UnfinishedWallMesh, TEXT("RecoveredConstructionWalls"), false);
    UInstancedStaticMeshComponent* ConstructionPillars =
        CreateVisualISM(Decorator, Root, UnfinishedPillarMesh, TEXT("RecoveredConstructionPillars"), false);

    // Dirt/forest-road pieces are kept on the outskirts so they enrich the rural edge without
    // replacing the authored central road network or landmark footprints.
    if (ForestPaths && ForestPathMesh)
    {
        AddFittedInstance(ForestPaths, ForestPathMesh, FVector(-92000.0f, -69000.0f, 4.0f),
            FVector(9800.0f, 4300.0f, 180.0f), 24.0f);
        AddFittedInstance(ForestPaths, ForestPathMesh, FVector(-80500.0f, -61000.0f, 4.0f),
            FVector(9600.0f, 4200.0f, 180.0f), 31.0f);
        AddFittedInstance(ForestPaths, ForestPathMesh, FVector(79000.0f, 61000.0f, 4.0f),
            FVector(10400.0f, 4500.0f, 180.0f), -38.0f);
    }

    // A small unfinished shell on the outer residential edge demonstrates the restored modular
    // construction kit without overwriting any named Oster landmark. Collision stays authoritative
    // in the existing world sector until this placement is visually validated in UE.
    const FVector Site(-69000.0f, 64500.0f, 0.0f);
    if (ConstructionWalls && UnfinishedWallMesh)
    {
        AddFittedInstance(ConstructionWalls, UnfinishedWallMesh, Site + FVector(-600.0f, 0.0f, 160.0f),
            FVector(1200.0f, 45.0f, 320.0f), 0.0f);
        AddFittedInstance(ConstructionWalls, UnfinishedWallMesh, Site + FVector(600.0f, 0.0f, 160.0f),
            FVector(1200.0f, 45.0f, 320.0f), 0.0f);
        AddFittedInstance(ConstructionWalls, UnfinishedWallMesh, Site + FVector(0.0f, 1050.0f, 160.0f),
            FVector(2100.0f, 45.0f, 320.0f), 90.0f);
    }

    if (ConstructionPillars && UnfinishedPillarMesh)
    {
        const FVector PillarOffsets[] =
        {
            FVector(-1050.0f, -700.0f, 160.0f), FVector(1050.0f, -700.0f, 160.0f),
            FVector(-1050.0f, 700.0f, 160.0f),  FVector(1050.0f, 700.0f, 160.0f)
        };
        for (const FVector& Offset : PillarOffsets)
        {
            AddFittedInstance(ConstructionPillars, UnfinishedPillarMesh, Site + Offset,
                FVector(45.0f, 45.0f, 320.0f), 0.0f);
        }
    }

    UE_LOG(LogTemp, Display,
        TEXT("Recovered environment models placed: forest-road outskirts + modular unfinished-building shell."));
}
