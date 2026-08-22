#include "OCRoadProfileRuntimeCorrectionSubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"

namespace
{
    constexpr float RoadThicknessCm = 4.0f;
    constexpr float RoadCenterZCm = RoadThicknessCm * 0.5f;
    constexpr float SidewalkThicknessCm = 8.0f;
    constexpr float SidewalkCenterZCm = RoadThicknessCm + SidewalkThicknessCm * 0.5f;

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

    bool NormalizeComponentProfile(UInstancedStaticMeshComponent* Component, float DesiredCenterZCm,
        float DesiredThicknessCm, int32& OutInstances)
    {
        OutInstances = 0;
        if (!Component) return false;

        const int32 Count = Component->GetInstanceCount();
        if (Count <= 0) return false;

        bool bChanged = false;
        for (int32 Index = 0; Index < Count; ++Index)
        {
            FTransform Transform;
            if (!Component->GetInstanceTransform(Index, Transform, false)) continue;

            FVector Location = Transform.GetLocation();
            FVector Scale = Transform.GetScale3D();

            // Roads/Sidewalks are 100 cm engine cubes, so local Z scale maps directly to thickness / 100.
            Location.Z = DesiredCenterZCm;
            Scale.Z = DesiredThicknessCm / 100.0f;
            Transform.SetLocation(Location);
            Transform.SetScale3D(Scale);

            if (Component->UpdateInstanceTransform(Index, Transform, false, false, true))
            {
                ++OutInstances;
                bChanged = true;
            }
        }

        if (bChanged)
        {
            Component->MarkRenderStateDirty();
        }
        return bChanged && OutInstances == Count;
    }

    bool ValidateComponentProfile(const UInstancedStaticMeshComponent* Component, float ExpectedCenterZCm,
        float MaxThicknessCm, int32& OutInstances, FString& OutFailure, const TCHAR* Label)
    {
        OutInstances = 0;
        if (!Component)
        {
            OutFailure = FString::Printf(TEXT("%s_component_missing"), Label);
            return false;
        }

        const int32 Count = Component->GetInstanceCount();
        if (Count <= 0)
        {
            OutFailure = FString::Printf(TEXT("%s_instances_missing"), Label);
            return false;
        }

        for (int32 Index = 0; Index < Count; ++Index)
        {
            FTransform Transform;
            if (!Component->GetInstanceTransform(Index, Transform, false))
            {
                OutFailure = FString::Printf(TEXT("%s_transform_unavailable_%d"), Label, Index);
                return false;
            }

            const float ThicknessCm = FMath::Abs(Transform.GetScale3D().Z) * 100.0f;
            const float CenterZCm = Transform.GetLocation().Z;
            if (ThicknessCm > MaxThicknessCm + 0.25f)
            {
                OutFailure = FString::Printf(TEXT("%s_instance_%d_thickness_cm_%.2f"), Label, Index, ThicknessCm);
                return false;
            }
            if (FMath::Abs(CenterZCm - ExpectedCenterZCm) > 0.5f)
            {
                OutFailure = FString::Printf(TEXT("%s_instance_%d_center_z_cm_%.2f"), Label, Index, CenterZCm);
                return false;
            }
            ++OutInstances;
        }
        return true;
    }
}

bool UOCRoadProfileRuntimeCorrectionSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && World->GetNetMode() != NM_DedicatedServer &&
        (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

TStatId UOCRoadProfileRuntimeCorrectionSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCRoadProfileRuntimeCorrectionSubsystem, STATGROUP_Tickables);
}

void UOCRoadProfileRuntimeCorrectionSubsystem::FailValidation(const FString& Reason)
{
    if (bFinished) return;
    bFinished = true;
    UE_LOG(LogTemp, Error, TEXT("PASS11_ROAD_PROFILE_RUNTIME_FAIL reason=%s"), *Reason);
}

bool UOCRoadProfileRuntimeCorrectionSubsystem::NormalizeRoadProfile(
    int32& OutRoadInstances, int32& OutSidewalkInstances)
{
    OutRoadInstances = 0;
    OutSidewalkInstances = 0;

    UWorld* World = GetWorld();
    if (!World) return false;

    AOCWorldSectorOster* Sector = nullptr;
    for (TActorIterator<AOCWorldSectorOster> It(World); It; ++It)
    {
        Sector = *It;
        break;
    }
    if (!Sector) return false;

    UInstancedStaticMeshComponent* Roads = FindISM(Sector, TEXT("Roads"));
    UInstancedStaticMeshComponent* Sidewalks = FindISM(Sector, TEXT("Sidewalks"));
    const bool bRoads = NormalizeComponentProfile(Roads, RoadCenterZCm, RoadThicknessCm, OutRoadInstances);
    const bool bSidewalks = NormalizeComponentProfile(
        Sidewalks, SidewalkCenterZCm, SidewalkThicknessCm, OutSidewalkInstances);
    return bRoads && bSidewalks;
}

bool UOCRoadProfileRuntimeCorrectionSubsystem::ValidateRoadProfile(
    int32& OutRoadInstances, int32& OutSidewalkInstances, FString& OutFailure) const
{
    OutRoadInstances = 0;
    OutSidewalkInstances = 0;
    OutFailure.Reset();

    UWorld* World = GetWorld();
    if (!World)
    {
        OutFailure = TEXT("world_missing");
        return false;
    }

    AOCWorldSectorOster* Sector = nullptr;
    for (TActorIterator<AOCWorldSectorOster> It(World); It; ++It)
    {
        Sector = *It;
        break;
    }
    if (!Sector)
    {
        OutFailure = TEXT("oster_sector_missing");
        return false;
    }

    if (!ValidateComponentProfile(FindISM(Sector, TEXT("Roads")), RoadCenterZCm, RoadThicknessCm,
        OutRoadInstances, OutFailure, TEXT("roads")))
    {
        return false;
    }
    if (!ValidateComponentProfile(FindISM(Sector, TEXT("Sidewalks")), SidewalkCenterZCm, SidewalkThicknessCm,
        OutSidewalkInstances, OutFailure, TEXT("sidewalks")))
    {
        return false;
    }
    return true;
}

void UOCRoadProfileRuntimeCorrectionSubsystem::Tick(float DeltaTime)
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

    int32 CorrectedRoads = 0;
    int32 CorrectedSidewalks = 0;
    NormalizeRoadProfile(CorrectedRoads, CorrectedSidewalks);

    int32 ValidRoads = 0;
    int32 ValidSidewalks = 0;
    FString Failure;
    if (ValidateRoadProfile(ValidRoads, ValidSidewalks, Failure))
    {
        bFinished = true;
        UE_LOG(LogTemp, Display,
            TEXT("PASS11_ROAD_PROFILE_READY roads=%d sidewalks=%d roadThicknessCm=%.1f sidewalkThicknessCm=%.1f curbHeightCm=%.1f"),
            ValidRoads, ValidSidewalks, RoadThicknessCm, SidewalkThicknessCm, SidewalkThicknessCm);
        return;
    }

    if (ElapsedSeconds >= 8.0f)
    {
        FailValidation(Failure.IsEmpty() ? TEXT("road_profile_not_ready") : Failure);
    }
}
