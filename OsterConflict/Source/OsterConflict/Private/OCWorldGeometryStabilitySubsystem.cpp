#include "OCWorldGeometryStabilitySubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"

namespace
{
    constexpr float BaselineCaptureSeconds = 12.0f;
    constexpr float ComparisonIntervalSeconds = 4.0f;
    constexpr int32 RequiredStableComparisons = 2;

    const FName TrackedFamilies[] =
    {
        TEXT("Buildings"),
        TEXT("ResidentialRoofs"),
        TEXT("ResidentialDetails"),
        TEXT("LandmarkBlocks"),
        TEXT("LandmarkRoofs"),
        TEXT("LandmarkWindows"),
        TEXT("LandmarkDetails"),
        TEXT("ParkGeometry"),
        TEXT("Roads"),
        TEXT("Sidewalks")
    };

    UInstancedStaticMeshComponent* FindISM(AActor* Actor, const FName Name)
    {
        if (!Actor) return nullptr;
        TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
        Actor->GetComponents(Components);
        for (UInstancedStaticMeshComponent* Component : Components)
        {
            if (Component && Component->GetFName() == Name) return Component;
        }
        return nullptr;
    }
}

bool UOCWorldGeometryStabilitySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && World->GetNetMode() != NM_DedicatedServer &&
        (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

TStatId UOCWorldGeometryStabilitySubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCWorldGeometryStabilitySubsystem, STATGROUP_Tickables);
}

void UOCWorldGeometryStabilitySubsystem::FailValidation(const FString& Reason)
{
    if (bFinished) return;
    bFinished = true;
    UE_LOG(LogTemp, Error, TEXT("PASS12_WORLD_GEOMETRY_STABILITY_FAIL reason=%s"), *Reason);
}

bool UOCWorldGeometryStabilitySubsystem::ReadTrackedCounts(
    TMap<FName, int32>& OutCounts, FString& OutFailure) const
{
    OutCounts.Reset();
    OutFailure.Reset();

    UWorld* World = GetWorld();
    if (!World)
    {
        OutFailure = TEXT("world_missing");
        return false;
    }

    AOCWorldSectorOster* Sector = nullptr;
    int32 SectorCount = 0;
    for (TActorIterator<AOCWorldSectorOster> It(World); It; ++It)
    {
        Sector = *It;
        ++SectorCount;
    }
    if (SectorCount != 1 || !Sector)
    {
        OutFailure = FString::Printf(TEXT("oster_sector_count_%d"), SectorCount);
        return false;
    }

    for (const FName Family : TrackedFamilies)
    {
        UInstancedStaticMeshComponent* Component = FindISM(Sector, Family);
        if (!Component)
        {
            OutFailure = FString::Printf(TEXT("tracked_family_missing_%s"), *Family.ToString());
            return false;
        }
        OutCounts.Add(Family, Component->GetInstanceCount());
    }

    return true;
}

bool UOCWorldGeometryStabilitySubsystem::CompareWithBaseline(
    const TMap<FName, int32>& CurrentCounts, FString& OutFailure) const
{
    OutFailure.Reset();
    for (const TPair<FName, int32>& Pair : BaselineCounts)
    {
        const int32* Current = CurrentCounts.Find(Pair.Key);
        if (!Current)
        {
            OutFailure = FString::Printf(TEXT("tracked_family_disappeared_%s"), *Pair.Key.ToString());
            return false;
        }
        if (*Current != Pair.Value)
        {
            OutFailure = FString::Printf(TEXT("late_geometry_mutation_%s_%d_to_%d"),
                *Pair.Key.ToString(), Pair.Value, *Current);
            return false;
        }
    }
    return true;
}

void UOCWorldGeometryStabilitySubsystem::Tick(float DeltaTime)
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
    if (ElapsedSeconds < BaselineCaptureSeconds) return;

    if (!bBaselineCaptured)
    {
        FString Failure;
        if (!ReadTrackedCounts(BaselineCounts, Failure))
        {
            FailValidation(Failure);
            return;
        }
        bBaselineCaptured = true;
        StableComparisonCount = 0;
        UE_LOG(LogTemp, Display,
            TEXT("PASS12_WORLD_GEOMETRY_BASELINE_CAPTURED families=%d at=%.1fs startupWindow=8.0s"),
            BaselineCounts.Num(), ElapsedSeconds);
        return;
    }

    const float NextComparisonSeconds = BaselineCaptureSeconds +
        ComparisonIntervalSeconds * static_cast<float>(StableComparisonCount + 1);
    if (ElapsedSeconds < NextComparisonSeconds) return;

    TMap<FName, int32> CurrentCounts;
    FString Failure;
    if (!ReadTrackedCounts(CurrentCounts, Failure))
    {
        FailValidation(Failure);
        return;
    }
    if (!CompareWithBaseline(CurrentCounts, Failure))
    {
        FailValidation(Failure);
        return;
    }

    ++StableComparisonCount;
    UE_LOG(LogTemp, Display,
        TEXT("PASS12_WORLD_GEOMETRY_STABLE_SAMPLE sample=%d/%d at=%.1fs families=%d"),
        StableComparisonCount, RequiredStableComparisons, ElapsedSeconds, CurrentCounts.Num());

    if (StableComparisonCount >= RequiredStableComparisons)
    {
        bFinished = true;
        UE_LOG(LogTemp, Display,
            TEXT("PASS12_WORLD_GEOMETRY_STABLE families=%d baseline=12.0s final=20.0s result=no_late_source_geometry_mutation"),
            BaselineCounts.Num());
    }
}
