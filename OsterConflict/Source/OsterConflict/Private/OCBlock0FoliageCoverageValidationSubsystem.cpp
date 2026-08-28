#include "OCBlock0FoliageCoverageValidationSubsystem.h"

#include "OCGameMode.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"

namespace
{
    constexpr float CompactMinX = -78000.0f;
    constexpr float CompactMaxX =  18000.0f;
    constexpr float CompactMinY = -12000.0f;
    constexpr float CompactMaxY =  82000.0f;
    constexpr float CompactWidthCm = CompactMaxX - CompactMinX;
    constexpr float CompactHeightCm = CompactMaxY - CompactMinY;
    constexpr int32 CoverageBinsPerAxis = 4;
    constexpr int32 CoverageBinCount = CoverageBinsPerAxis * CoverageBinsPerAxis;
    constexpr int32 MinOccupiedBins = 12;
    constexpr int32 MinOccupiedBinsPerQuadrant = 2;
    constexpr float EdgeToleranceFraction = 0.20f;

    const FName DenseFoliageActorTag(TEXT("OC_DenseGroundFoliage"));
    const FName Block0PopulationCompleteTag(TEXT("OC_Block0FullMapGrassComplete"));

    bool IsLowCPUProfile(const UWorld& World)
    {
        const TCHAR* Value = World.URL.GetOption(TEXT("PerfProfile="), TEXT(""));
        return Value && FString(Value).Equals(TEXT("LowCPU"), ESearchCase::IgnoreCase);
    }

    int32 CoverageBin(const float Value, const float MinValue, const float MaxValue)
    {
        const float Alpha = FMath::Clamp((Value - MinValue) / (MaxValue - MinValue), 0.0f, 1.0f);
        return FMath::Clamp(FMath::FloorToInt(Alpha * CoverageBinsPerAxis), 0, CoverageBinsPerAxis - 1);
    }
}

bool UOCBlock0FoliageCoverageValidationSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && World->GetNetMode() != NM_DedicatedServer &&
        (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

TStatId UOCBlock0FoliageCoverageValidationSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCBlock0FoliageCoverageValidationSubsystem, STATGROUP_Tickables);
}

void UOCBlock0FoliageCoverageValidationSubsystem::FailValidation(const FString& Reason)
{
    if (bFinished) return;
    bFinished = true;
    UE_LOG(LogTemp, Error,
        TEXT("PASS45_BLOCK0_SPATIAL_GRASS_COVERAGE_FAIL reason=%s mutation=0 runtime_acceptance=0"),
        *Reason);
}

void UOCBlock0FoliageCoverageValidationSubsystem::Tick(float DeltaTime)
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
    ValidationAccumulator += FMath::Max(0.0f, DeltaTime);
    if (ValidationAccumulator < 0.50f) return;
    ValidationAccumulator = 0.0f;

    AActor* DenseActor = nullptr;
    int32 DenseActorCount = 0;
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor || !Actor->ActorHasTag(DenseFoliageActorTag)) continue;
        DenseActor = Actor;
        ++DenseActorCount;
    }

    const bool bLowCPU = IsLowCPUProfile(*World);
    const float DeadlineSeconds = bLowCPU ? 15.0f : 35.0f;
    if (DenseActorCount != 1 || !DenseActor || !DenseActor->ActorHasTag(Block0PopulationCompleteTag))
    {
        if (ElapsedSeconds >= DeadlineSeconds)
        {
            FailValidation(FString::Printf(
                TEXT("population_not_complete_by_deadline dense_actor_count=%d profile=%s"),
                DenseActorCount,
                bLowCPU ? TEXT("LowCPU") : TEXT("Full")));
        }
        return;
    }

    bool OccupiedBins[CoverageBinCount] = {};
    int32 GrassInstances = 0;
    int32 DenseGrassComponents = 0;
    float ObservedMinX = TNumericLimits<float>::Max();
    float ObservedMaxX = TNumericLimits<float>::Lowest();
    float ObservedMinY = TNumericLimits<float>::Max();
    float ObservedMaxY = TNumericLimits<float>::Lowest();

    TInlineComponentArray<UHierarchicalInstancedStaticMeshComponent*> Components;
    DenseActor->GetComponents(Components);
    for (UHierarchicalInstancedStaticMeshComponent* Component : Components)
    {
        if (!Component || !Component->GetName().StartsWith(TEXT("DenseGrass_"))) continue;
        ++DenseGrassComponents;

        for (int32 Index = 0; Index < Component->GetInstanceCount(); ++Index)
        {
            FTransform InstanceTransform;
            if (!Component->GetInstanceTransform(Index, InstanceTransform, true)) continue;
            const FVector Location = InstanceTransform.GetLocation();
            if (Location.X < CompactMinX || Location.X > CompactMaxX ||
                Location.Y < CompactMinY || Location.Y > CompactMaxY)
            {
                continue;
            }

            ++GrassInstances;
            ObservedMinX = FMath::Min(ObservedMinX, Location.X);
            ObservedMaxX = FMath::Max(ObservedMaxX, Location.X);
            ObservedMinY = FMath::Min(ObservedMinY, Location.Y);
            ObservedMaxY = FMath::Max(ObservedMaxY, Location.Y);

            const int32 BinX = CoverageBin(Location.X, CompactMinX, CompactMaxX);
            const int32 BinY = CoverageBin(Location.Y, CompactMinY, CompactMaxY);
            OccupiedBins[BinY * CoverageBinsPerAxis + BinX] = true;
        }
    }

    if (DenseGrassComponents <= 0 || GrassInstances <= 0)
    {
        FailValidation(TEXT("dense_grass_instances_missing_after_population_complete"));
        return;
    }

    int32 OccupiedBinCount = 0;
    int32 QuadrantOccupied[4] = {};
    for (int32 BinY = 0; BinY < CoverageBinsPerAxis; ++BinY)
    {
        for (int32 BinX = 0; BinX < CoverageBinsPerAxis; ++BinX)
        {
            if (!OccupiedBins[BinY * CoverageBinsPerAxis + BinX]) continue;
            ++OccupiedBinCount;
            const int32 QuadrantX = BinX >= CoverageBinsPerAxis / 2 ? 1 : 0;
            const int32 QuadrantY = BinY >= CoverageBinsPerAxis / 2 ? 1 : 0;
            ++QuadrantOccupied[QuadrantY * 2 + QuadrantX];
        }
    }

    const float EdgeToleranceX = CompactWidthCm * EdgeToleranceFraction;
    const float EdgeToleranceY = CompactHeightCm * EdgeToleranceFraction;
    const bool bEdgeReach =
        ObservedMinX <= CompactMinX + EdgeToleranceX &&
        ObservedMaxX >= CompactMaxX - EdgeToleranceX &&
        ObservedMinY <= CompactMinY + EdgeToleranceY &&
        ObservedMaxY >= CompactMaxY - EdgeToleranceY;
    const bool bQuadrantsReady =
        QuadrantOccupied[0] >= MinOccupiedBinsPerQuadrant &&
        QuadrantOccupied[1] >= MinOccupiedBinsPerQuadrant &&
        QuadrantOccupied[2] >= MinOccupiedBinsPerQuadrant &&
        QuadrantOccupied[3] >= MinOccupiedBinsPerQuadrant;

    if (OccupiedBinCount < MinOccupiedBins || !bQuadrantsReady || !bEdgeReach)
    {
        FailValidation(FString::Printf(
            TEXT("distribution_insufficient grass=%d occupied_bins=%d/%d quadrants=%d,%d,%d,%d edge_reach=%d span_x_m=%.1f span_y_m=%.1f profile=%s"),
            GrassInstances,
            OccupiedBinCount,
            CoverageBinCount,
            QuadrantOccupied[0],
            QuadrantOccupied[1],
            QuadrantOccupied[2],
            QuadrantOccupied[3],
            bEdgeReach ? 1 : 0,
            (ObservedMaxX - ObservedMinX) / 100.0f,
            (ObservedMaxY - ObservedMinY) / 100.0f,
            bLowCPU ? TEXT("LowCPU") : TEXT("Full")));
        return;
    }

    bFinished = true;
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_BLOCK0_SPATIAL_GRASS_COVERAGE_READY grass=%d dense_components=%d occupied_bins=%d/%d quadrants=%d,%d,%d,%d span_x_m=%.1f span_y_m=%.1f edge_reach=1 full_playable_distribution=1 mutation=0 runtime_acceptance=0"),
        GrassInstances,
        DenseGrassComponents,
        OccupiedBinCount,
        CoverageBinCount,
        QuadrantOccupied[0],
        QuadrantOccupied[1],
        QuadrantOccupied[2],
        QuadrantOccupied[3],
        (ObservedMaxX - ObservedMinX) / 100.0f,
        (ObservedMaxY - ObservedMinY) / 100.0f);
}
