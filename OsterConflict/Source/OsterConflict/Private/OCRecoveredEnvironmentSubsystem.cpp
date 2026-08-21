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
    if (!ForestPathMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("Recovered environment pass skipped: forest path mesh was not loadable."));
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

    if (ForestPaths)
    {
        AddFittedInstance(ForestPaths, ForestPathMesh, FVector(-92000.0f, -69000.0f, 4.0f),
            FVector(9800.0f, 4300.0f, 180.0f), 24.0f);
        AddFittedInstance(ForestPaths, ForestPathMesh, FVector(-80500.0f, -61000.0f, 4.0f),
            FVector(9600.0f, 4200.0f, 180.0f), 31.0f);
        AddFittedInstance(ForestPaths, ForestPathMesh, FVector(79000.0f, 61000.0f, 4.0f),
            FVector(10400.0f, 4500.0f, 180.0f), -38.0f);
    }

    // The former unfinished-building showcase at (-69000,64500) was intentionally removed.
    // It had no evidence-backed Oster site and two runtime subsystems were acting as its owners.
    UE_LOG(LogTemp, Display,
        TEXT("Recovered environment models placed: forest-road outskirts only; unverified unfinished-building site is disabled."));
}