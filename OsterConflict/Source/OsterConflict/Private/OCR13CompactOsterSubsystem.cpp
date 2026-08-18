#include "OCR13CompactOsterSubsystem.h"

#include "OCCapturePoint.h"
#include "OCGameMode.h"
#include "OCTeamSpawnPoint.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"

namespace
{
    constexpr int32 MaxApplyAttempts = 20;
    constexpr float RetryDelaySeconds = 0.25f;

    // R13.1: focus production on the recognizable central Oster cluster instead of a sparse 2.4 x 2.4 km field.
    // Bounds intentionally retain the museum, stadium, college, central park and Krushelnytska reference slice.
    constexpr float CompactMinX = -70000.0f;
    constexpr float CompactMaxX =  25000.0f;
    constexpr float CompactMinY = -25000.0f;
    constexpr float CompactMaxY =  50000.0f;

    bool IsInsideCompactBounds(const FVector& Location, const float Padding = 0.0f)
    {
        return Location.X >= CompactMinX - Padding && Location.X <= CompactMaxX + Padding &&
               Location.Y >= CompactMinY - Padding && Location.Y <= CompactMaxY + Padding;
    }

    FVector ObjectiveLocation(const FName PointId)
    {
        if (PointId == TEXT("A"))
        {
            return AOCWorldSectorOster::ParkAnchor() + FVector(10000.0f, -7000.0f, 120.0f);
        }
        if (PointId == TEXT("B"))
        {
            return AOCWorldSectorOster::CollegeAnchor() + FVector(5000.0f, -2500.0f, 120.0f);
        }
        if (PointId == TEXT("C"))
        {
            return AOCWorldSectorOster::StadiumAnchor() + FVector(-5000.0f, 2500.0f, 120.0f);
        }
        return FVector::ZeroVector;
    }
}

bool UOCR13CompactOsterSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13CompactOsterSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    ScheduleApply(InWorld, 0.10f);
}

void UOCR13CompactOsterSubsystem::ScheduleApply(UWorld& World, const float DelaySeconds)
{
    if (bApplied || ApplyAttemptCount >= MaxApplyAttempts) return;

    TWeakObjectPtr<UWorld> WeakWorld(&World);
    FTimerHandle TimerHandle;
    World.GetTimerManager().SetTimer(
        TimerHandle,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* RetryWorld = WeakWorld.Get())
            {
                TryApplyCompactLayout(*RetryWorld);
            }
        }),
        FMath::Max(0.05f, DelaySeconds), false);
}

void UOCR13CompactOsterSubsystem::TryApplyCompactLayout(UWorld& World)
{
    if (bApplied) return;
    ++ApplyAttemptCount;

    AOCWorldSectorOster* WorldSector = nullptr;
    for (TActorIterator<AOCWorldSectorOster> It(&World); It; ++It)
    {
        WorldSector = *It;
        if (WorldSector) break;
    }

    if (!WorldSector)
    {
        if (ApplyAttemptCount < MaxApplyAttempts)
        {
            ScheduleApply(World, RetryDelaySeconds);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("R13.1 compact Oster: world sector unavailable after %d attempts."), ApplyAttemptCount);
        }
        return;
    }

    const FVector CompactCenter(
        (CompactMinX + CompactMaxX) * 0.5f,
        (CompactMinY + CompactMaxY) * 0.5f,
        -100.0f);
    const float CompactWidthCm = CompactMaxX - CompactMinX;
    const float CompactHeightCm = CompactMaxY - CompactMinY;

    int32 RemovedInstances = 0;
    if (!bWorldCropped)
    {
        TArray<UStaticMeshComponent*> StaticMeshes;
        WorldSector->GetComponents<UStaticMeshComponent>(StaticMeshes);
        for (UStaticMeshComponent* Mesh : StaticMeshes)
        {
            if (!Mesh || Mesh->GetFName() != TEXT("Ground")) continue;
            Mesh->SetRelativeLocation(CompactCenter);
            Mesh->SetRelativeScale3D(FVector(CompactWidthCm / 100.0f, CompactHeightCm / 100.0f, 2.0f));
            break;
        }

        TArray<UInstancedStaticMeshComponent*> InstancedComponents;
        WorldSector->GetComponents<UInstancedStaticMeshComponent>(InstancedComponents);
        for (UInstancedStaticMeshComponent* Component : InstancedComponents)
        {
            if (!Component) continue;
            const FString ComponentName = Component->GetName();
            const bool bLinearInfrastructure = ComponentName.Contains(TEXT("Road")) ||
                ComponentName.Contains(TEXT("Sidewalk")) || ComponentName.Contains(TEXT("Water")) ||
                ComponentName.Contains(TEXT("Bridge"));
            const float Padding = bLinearInfrastructure ? 18000.0f : 8000.0f;

            for (int32 Index = Component->GetInstanceCount() - 1; Index >= 0; --Index)
            {
                FTransform InstanceTransform;
                if (!Component->GetInstanceTransform(Index, InstanceTransform, true)) continue;
                if (!IsInsideCompactBounds(InstanceTransform.GetLocation(), Padding))
                {
                    if (Component->RemoveInstance(Index)) ++RemovedInstances;
                }
            }
        }
        bWorldCropped = true;
    }

    TMap<FName, FVector> ObjectiveLocations;
    ObjectiveLocations.Add(TEXT("A"), ObjectiveLocation(TEXT("A")));
    ObjectiveLocations.Add(TEXT("B"), ObjectiveLocation(TEXT("B")));
    ObjectiveLocations.Add(TEXT("C"), ObjectiveLocation(TEXT("C")));

    // Capture points intentionally do not replicate movement. Apply the same deterministic compact coordinates on
    // server and clients; clients retry until all replicated point actors exist instead of drawing stale old locations.
    TSet<FName> ObjectivesMoved;
    for (TActorIterator<AOCCapturePoint> It(&World); It; ++It)
    {
        AOCCapturePoint* Point = *It;
        if (!Point) continue;
        if (const FVector* Target = ObjectiveLocations.Find(Point->GetPointId()))
        {
            Point->SetActorLocation(*Target, false, nullptr, ETeleportType::TeleportPhysics);
            ObjectivesMoved.Add(Point->GetPointId());
        }
    }

    if (World.GetNetMode() != NM_Client)
    {
        const FVector TeamOneBase(-64000.0f, 44000.0f, 160.0f);
        const FVector TeamTwoBase( 20000.0f,-19000.0f, 160.0f);
        for (TActorIterator<AOCTeamSpawnPoint> It(&World); It; ++It)
        {
            AOCTeamSpawnPoint* Spawn = *It;
            if (!Spawn) continue;

            FVector Target = Spawn->GetActorLocation();
            if (Spawn->IsBaseSpawn())
            {
                if (Spawn->GetTeamId() == EOCTeam::TeamOne) Target = TeamOneBase;
                else if (Spawn->GetTeamId() == EOCTeam::TeamTwo) Target = TeamTwoBase;
            }
            else if (const FVector* Objective = ObjectiveLocations.Find(Spawn->GetLinkedCapturePointId()))
            {
                const float TeamOffset = Spawn->GetTeamId() == EOCTeam::TeamTwo ? -650.0f : 650.0f;
                Target = *Objective + FVector(0.0f, TeamOffset, 40.0f);
            }

            Spawn->SetActorLocation(Target, false, nullptr, ETeleportType::TeleportPhysics);
        }
    }

    if (ObjectivesMoved.Num() < ObjectiveLocations.Num())
    {
        if (ApplyAttemptCount < MaxApplyAttempts)
        {
            ScheduleApply(World, RetryDelaySeconds);
        }
        else
        {
            UE_LOG(LogTemp, Warning,
                TEXT("R13.1 compact Oster: only %d/%d objective actors synchronized after %d attempts."),
                ObjectivesMoved.Num(), ObjectiveLocations.Num(), ApplyAttemptCount);
        }
        return;
    }

    bApplied = true;
    UE_LOG(LogTemp, Display,
        TEXT("R13.1 compact Oster applied: %.0f x %.0f m, center=(%.0f, %.0f), objectives=%d, removed source instances=%d."),
        CompactWidthCm / 100.0f, CompactHeightCm / 100.0f,
        CompactCenter.X, CompactCenter.Y, ObjectivesMoved.Num(), RemovedInstances);
}
