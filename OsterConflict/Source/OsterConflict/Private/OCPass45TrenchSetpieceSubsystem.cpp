#include "OCPass45TrenchSetpieceSubsystem.h"

#include "OCGameMode.h"
#include "OCPass45LocalAssetResolver.h"
#include "OCTeamSpawnPoint.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"

namespace
{
    constexpr float BuildDelaySeconds = 0.85f;
    const FName AuthoredTrenchTag(TEXT("OC_PASS45_AUTHORED_TRENCH_SETPIECE"));

    AStaticMeshActor* SpawnSetpiece(
        UWorld& World,
        UStaticMesh* Mesh,
        const FVector& GroundLocation,
        const FRotator& Rotation,
        const float DesiredLongestAxisCm)
    {
        if (!Mesh) return nullptr;

        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector NativeSize = Bounds.BoxExtent * 2.0f;
        const float NativeLongestAxis = FMath::Max3(NativeSize.X, NativeSize.Y, NativeSize.Z);
        if (NativeLongestAxis <= 1.0f) return nullptr;

        const float Scale = DesiredLongestAxisCm / NativeLongestAxis;
        FVector Location = GroundLocation;
        Location.Z -= (Bounds.Origin.Z - Bounds.BoxExtent.Z) * Scale;

        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        AStaticMeshActor* Actor = World.SpawnActor<AStaticMeshActor>(Location, Rotation, Params);
        if (!Actor) return nullptr;

        UStaticMeshComponent* Component = Actor->GetStaticMeshComponent();
        if (!Component)
        {
            Actor->Destroy();
            return nullptr;
        }

        Component->SetMobility(EComponentMobility::Movable);
        Component->SetStaticMesh(Mesh);
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(false);
        Component->SetCastShadow(true);
        Actor->SetActorScale3D(FVector(Scale));
        Actor->Tags.AddUnique(AuthoredTrenchTag);
        return Actor;
    }
}

bool UOCPass45TrenchSetpieceSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && World->GetNetMode() != NM_DedicatedServer &&
        (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

TStatId UOCPass45TrenchSetpieceSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCPass45TrenchSetpieceSubsystem, STATGROUP_Tickables);
}

void UOCPass45TrenchSetpieceSubsystem::Tick(float DeltaTime)
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
    if (ElapsedSeconds < BuildDelaySeconds) return;
    bFinished = true;

    for (TActorIterator<AStaticMeshActor> Existing(*World); Existing; ++Existing)
    {
        if (Existing->ActorHasTag(AuthoredTrenchTag))
        {
            UE_LOG(LogTemp, Display,
                TEXT("PASS45_TRENCH_SETPIECE_SKIPPED reason=already_authored duplicate_spawn=0 runtime_acceptance=0"));
            return;
        }
    }

    UStaticMesh* BarrierMesh = OCPass45FindLocalStaticMesh(
        { FName(TEXT("/Game/Fab/Megascans/3D/Military_Trenches_Barrier_Sandbag_Canvas_Square_01_yd0kbfl")) },
        { TEXT("sandbag"), TEXT("barrier") });
    UStaticMesh* PileMesh = OCPass45FindLocalStaticMesh(
        { FName(TEXT("/Game/Fab/Megascans/3D/Military_Trenches_Pile_Sandbag_Canvas_01_yd0tae2")) },
        { TEXT("sandbag"), TEXT("pile") });

    if (!BarrierMesh || !PileMesh)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("PASS45_TRENCH_LOCAL_CONTENT_GAP barrier_loaded=%d pile_loaded=%d local_import_preserved=1 runtime_acceptance=0"),
            BarrierMesh ? 1 : 0, PileMesh ? 1 : 0);
        return;
    }

    // Optional user-added rubble pack. Strict identity matching means a random prop is never substituted.
    // /Game fallback is allowed because Fab packs may import to their own top-level package rather than /Game/Fab.
    UStaticMesh* RubbleMesh = OCPass45FindLocalStaticMeshStrict(
        { FName(TEXT("/Game/Fab")), FName(TEXT("/Game")) },
        { TEXT("rubble") });

    int32 BaseCount = 0;
    int32 SpawnedCount = 0;
    int32 RubbleSpawnedCount = 0;
    for (TActorIterator<AOCTeamSpawnPoint> It(*World); It; ++It)
    {
        AOCTeamSpawnPoint* Spawn = *It;
        if (!Spawn || !Spawn->IsBaseSpawn()) continue;
        ++BaseCount;

        const FVector Base = Spawn->GetActorLocation();
        const FRotator Facing(0.0f, Spawn->GetActorRotation().Yaw, 0.0f);
        const FVector Forward = Facing.Vector();
        const FVector Right = FRotationMatrix(Facing).GetUnitAxis(EAxis::Y);

        const struct FPlacement
        {
            FVector Offset;
            float YawOffset;
            bool bPile;
            float SizeCm;
        } Placements[] = {
            { Forward * 620.0f + Right * 330.0f,   0.0f, false, 520.0f },
            { Forward * 620.0f - Right * 330.0f,   0.0f, false, 520.0f },
            { Right * 760.0f,                      90.0f, false, 500.0f },
            { Right * -760.0f,                    -90.0f, false, 500.0f },
            { Forward * -360.0f + Right * 510.0f, 25.0f, true,  280.0f },
            { Forward * -420.0f - Right * 460.0f,-20.0f, true,  280.0f }
        };

        for (const FPlacement& Placement : Placements)
        {
            UStaticMesh* Mesh = Placement.bPile ? PileMesh : BarrierMesh;
            if (SpawnSetpiece(*World, Mesh, Base + Placement.Offset,
                FRotator(0.0f, Facing.Yaw + Placement.YawOffset, 0.0f), Placement.SizeCm))
            {
                ++SpawnedCount;
            }
        }

        if (RubbleMesh)
        {
            const struct FRubblePlacement
            {
                FVector Offset;
                float YawOffset;
                float SizeCm;
            } RubblePlacements[] = {
                { Forward * 930.0f + Right * 690.0f,  18.0f, 260.0f },
                { Forward * 870.0f - Right * 720.0f, -24.0f, 230.0f },
                { Forward * -650.0f + Right * 840.0f, 41.0f, 210.0f },
                { Forward * -720.0f - Right * 790.0f,-37.0f, 240.0f }
            };

            for (const FRubblePlacement& Placement : RubblePlacements)
            {
                if (SpawnSetpiece(*World, RubbleMesh, Base + Placement.Offset,
                    FRotator(0.0f, Facing.Yaw + Placement.YawOffset, 0.0f), Placement.SizeCm))
                {
                    ++RubbleSpawnedCount;
                    ++SpawnedCount;
                }
            }
        }
    }

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_TRENCH_SETPIECE_READY base_spawns=%d authored_instances=%d barrier=%s pile=%s rubble_loaded=%d rubble_instances=%d rubble_asset=%s no_collision=1 duplicate_spawn=0 runtime_acceptance=0"),
        BaseCount,
        SpawnedCount,
        *BarrierMesh->GetName(),
        *PileMesh->GetName(),
        RubbleMesh ? 1 : 0,
        RubbleSpawnedCount,
        RubbleMesh ? *RubbleMesh->GetPathName() : TEXT("NONE"));
}
