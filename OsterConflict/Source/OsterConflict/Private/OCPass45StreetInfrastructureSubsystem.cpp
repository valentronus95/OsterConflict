#include "OCPass45StreetInfrastructureSubsystem.h"

#include "OCGameMode.h"
#include "OCPass45LocalAssetResolver.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"

namespace
{
    constexpr float UpgradeDelaySeconds = 0.95f;
    constexpr int32 MaxRoadSegmentsToDecorate = 18;
    const FName AuthoredPoleTag(TEXT("OC_PASS45_AUTHORED_STREET_POLE"));

    UInstancedStaticMeshComponent* FindISM(AActor* Owner, const FName Name)
    {
        if (!Owner) return nullptr;
        TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
        Owner->GetComponents(Components);
        for (UInstancedStaticMeshComponent* Component : Components)
        {
            if (Component && Component->GetFName() == Name) return Component;
        }
        return nullptr;
    }

    bool SpawnPole(UWorld& World, UStaticMesh* Mesh, const FVector& GroundLocation, const FRotator& Rotation,
        const float DesiredHeightCm)
    {
        if (!Mesh) return false;
        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector NativeSize = Bounds.BoxExtent * 2.0f;
        if (NativeSize.Z <= 1.0f) return false;

        const float Scale = DesiredHeightCm / NativeSize.Z;
        FVector Location = GroundLocation;
        Location.Z -= (Bounds.Origin.Z - Bounds.BoxExtent.Z) * Scale;

        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        AStaticMeshActor* Actor = World.SpawnActor<AStaticMeshActor>(Location, Rotation, Params);
        if (!Actor) return false;

        UStaticMeshComponent* Component = Actor->GetStaticMeshComponent();
        if (!Component)
        {
            Actor->Destroy();
            return false;
        }
        Component->SetMobility(EComponentMobility::Movable);
        Component->SetStaticMesh(Mesh);
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(false);
        Component->SetCastShadow(true);
        Actor->SetActorScale3D(FVector(Scale));
        Actor->Tags.AddUnique(AuthoredPoleTag);
        return true;
    }
}

bool UOCPass45StreetInfrastructureSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && World->GetNetMode() != NM_DedicatedServer &&
        (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

TStatId UOCPass45StreetInfrastructureSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCPass45StreetInfrastructureSubsystem, STATGROUP_Tickables);
}

void UOCPass45StreetInfrastructureSubsystem::Tick(float DeltaTime)
{
    if (bFinished) return;
    UWorld* World = GetWorld();
    if (!World || !World->IsGameWorld()) return;
    if (!World->GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    if (const AOCGameMode* GameMode = World->GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    ElapsedSeconds += FMath::Max(0.0f, DeltaTime);
    if (ElapsedSeconds < UpgradeDelaySeconds) return;
    bFinished = true;

    // TActorIterator expects the UWorld pointer. Passing *World forces an invalid
    // UWorld& conversion in UE 5.8/MSVC and breaks the editor build.
    for (TActorIterator<AStaticMeshActor> Existing(World); Existing; ++Existing)
    {
        if (Existing->ActorHasTag(AuthoredPoleTag)) return;
    }

    UStaticMesh* LightPoleMesh = OCPass45FindLocalStaticMesh(
        { FName(TEXT("/Game/pripyat-light-poles")) },
        { TEXT("light"), TEXT("pole") });
    UStaticMesh* UtilityPoleMesh = OCPass45FindLocalStaticMesh(
        { FName(TEXT("/Game/telephone-pole-scene")) },
        { TEXT("telephone"), TEXT("utility"), TEXT("pole") });

    if (!LightPoleMesh && !UtilityPoleMesh)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("PASS45_STREET_INFRA_LOCAL_CONTENT_GAP light_pole=0 utility_pole=0 runtime_acceptance=0"));
        return;
    }

    AOCWorldSectorOster* Sector = nullptr;
    int32 SectorCount = 0;
    for (TActorIterator<AOCWorldSectorOster> It(World); It; ++It)
    {
        Sector = *It;
        ++SectorCount;
    }
    UInstancedStaticMeshComponent* Roads = SectorCount == 1 ? FindISM(Sector, FName(TEXT("Roads"))) : nullptr;
    if (!Roads || Roads->GetInstanceCount() <= 0)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("PASS45_STREET_INFRA_FAIL roads_owner_missing=1 sector_count=%d runtime_acceptance=0"), SectorCount);
        return;
    }

    const int32 RoadCount = FMath::Min(Roads->GetInstanceCount(), MaxRoadSegmentsToDecorate);
    int32 Spawned = 0;
    for (int32 Index = 0; Index < RoadCount; ++Index)
    {
        FTransform RoadTransform;
        if (!Roads->GetInstanceTransform(Index, RoadTransform, true)) continue;

        const FVector AbsScale = RoadTransform.GetScale3D().GetAbs();
        const bool bLongX = AbsScale.X >= AbsScale.Y;
        const FVector LocalLong = bLongX ? FVector::ForwardVector : FVector::RightVector;
        const FVector LocalSide = bLongX ? FVector::RightVector : FVector::ForwardVector;
        const FVector Long = RoadTransform.GetRotation().RotateVector(LocalLong).GetSafeNormal2D();
        const FVector Side = RoadTransform.GetRotation().RotateVector(LocalSide).GetSafeNormal2D();
        const float HalfLength = 50.0f * (bLongX ? AbsScale.X : AbsScale.Y);
        const float HalfWidth = 50.0f * (bLongX ? AbsScale.Y : AbsScale.X);
        if (HalfLength < 500.0f) continue;

        const FVector Center = RoadTransform.GetLocation();
        const float Along = FMath::Min(HalfLength * 0.62f, 2600.0f);
        const float SideOffset = HalfWidth + 165.0f;
        const FRotator PoleRotation(0.0f, RoadTransform.Rotator().Yaw, 0.0f);

        UStaticMesh* MeshA = LightPoleMesh ? LightPoleMesh : UtilityPoleMesh;
        UStaticMesh* MeshB = UtilityPoleMesh ? UtilityPoleMesh : LightPoleMesh;
        if (SpawnPole(*World, MeshA, Center + Long * Along + Side * SideOffset, PoleRotation, 720.0f)) ++Spawned;
        if (SpawnPole(*World, MeshB, Center - Long * Along - Side * SideOffset, PoleRotation, 760.0f)) ++Spawned;
    }

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_STREET_INFRA_READY road_segments_considered=%d authored_poles=%d light_pole_loaded=%d utility_pole_loaded=%d road_transform_reuse=1 duplicate_coordinate_map=0 no_collision=1 runtime_acceptance=0"),
        RoadCount, Spawned, LightPoleMesh ? 1 : 0, UtilityPoleMesh ? 1 : 0);
}
