#include "OCWorldGeometryStabilitySubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Materials/MaterialInterface.h"

namespace
{
    constexpr float BaselineCaptureSeconds = 12.0f;
    constexpr float ComparisonIntervalSeconds = 4.0f;
    constexpr int32 RequiredStableComparisons = 2;

    const FName TrackedFamilies[]
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
        TEXT("Sidewalks"),
        TEXT("ParkPaths"),
        TEXT("Fences")
    };

    UStaticMeshComponent* FindStaticMeshComponent(AActor* Actor, const FName Name)
    {
        if (!Actor) return nullptr;
        TInlineComponentArray<UStaticMeshComponent*> Components;
        Actor->GetComponents(Components);
        for (UStaticMeshComponent* Component : Components)
        {
            if (Component && Component->GetFName() == Name) return Component;
        }
        return nullptr;
    }

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

    bool HasAuthoredGroundSurface(
        UStaticMeshComponent* Component,
        const TCHAR* ExpectedMeshToken,
        const TCHAR* ExpectedMaterialToken,
        FString& OutFailure)
    {
        if (!Component || !Component->GetStaticMesh())
        {
            OutFailure = TEXT("authored_ground_mesh_missing");
            return false;
        }

        const FString MeshPath = Component->GetStaticMesh()->GetPathName();
        if (!MeshPath.Contains(ExpectedMeshToken, ESearchCase::IgnoreCase) ||
            MeshPath.Contains(TEXT("/Engine/BasicShapes/"), ESearchCase::IgnoreCase))
        {
            OutFailure = FString::Printf(TEXT("authored_ground_mesh_invalid_%s"), *MeshPath);
            return false;
        }

        UMaterialInterface* Material = Component->GetMaterial(0);
        if (!Material)
        {
            OutFailure = TEXT("authored_ground_material_missing");
            return false;
        }
        const FString MaterialPath = Material->GetPathName();
        if (!MaterialPath.Contains(ExpectedMaterialToken, ESearchCase::IgnoreCase) ||
            MaterialPath.Contains(TEXT("/Engine/BasicShapes/"), ESearchCase::IgnoreCase))
        {
            OutFailure = FString::Printf(TEXT("authored_ground_material_invalid_%s"), *MaterialPath);
            return false;
        }
        return true;
    }

    bool HasAuthoredSurface(
        UInstancedStaticMeshComponent* Component,
        const TCHAR* ExpectedMeshToken,
        FString& OutFailure,
        const TCHAR* Family)
    {
        if (!Component || !Component->GetStaticMesh())
        {
            OutFailure = FString::Printf(TEXT("authored_surface_mesh_missing_%s"), Family);
            return false;
        }

        const FString MeshPath = Component->GetStaticMesh()->GetPathName();
        if (!MeshPath.Contains(ExpectedMeshToken, ESearchCase::IgnoreCase) ||
            MeshPath.Contains(TEXT("/Engine/BasicShapes/"), ESearchCase::IgnoreCase))
        {
            OutFailure = FString::Printf(TEXT("authored_surface_mesh_invalid_%s_%s"), Family, *MeshPath);
            return false;
        }

        UMaterialInterface* Material = Component->GetMaterial(0);
        if (!Material)
        {
            OutFailure = FString::Printf(TEXT("authored_surface_material_missing_%s"), Family);
            return false;
        }
        if (Material->GetPathName().Contains(TEXT("/Engine/BasicShapes/"), ESearchCase::IgnoreCase))
        {
            OutFailure = FString::Printf(TEXT("authored_surface_basicshape_material_%s"), Family);
            return false;
        }
        return true;
    }

    bool ValidateSemanticMaterials(AOCWorldSectorOster* Sector, FString& OutFailure)
    {
        if (!Sector)
        {
            OutFailure = TEXT("semantic_material_sector_missing");
            return false;
        }

        // Item 31 upgrades all verified surface families before the 12-second stability baseline. None of these
        // families may fall back to Engine BasicShape material/mesh acceptance after startup.
        if (!HasAuthoredGroundSurface(
            FindStaticMeshComponent(Sector, TEXT("Ground")),
            TEXT("SM_Plane_1x1"),
            TEXT("M_Inst_Landscape"),
            OutFailure)) return false;
        if (!HasAuthoredSurface(
            FindISM(Sector, TEXT("Roads")),
            TEXT("SM_Urb_Roa_Asphalt_01"),
            OutFailure,
            TEXT("Roads"))) return false;
        if (!HasAuthoredSurface(
            FindISM(Sector, TEXT("Sidewalks")),
            TEXT("SM_Urb_Roa_Sidewalk_01"),
            OutFailure,
            TEXT("Sidewalks"))) return false;
        if (!HasAuthoredSurface(
            FindISM(Sector, TEXT("ParkPaths")),
            TEXT("SM_Stonepath_Var01"),
            OutFailure,
            TEXT("ParkPaths"))) return false;
        if (!HasAuthoredSurface(
            FindISM(Sector, TEXT("Fences")),
            TEXT("SM_Fence_Var01"),
            OutFailure,
            TEXT("Fences"))) return false;

        return true;
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

    if (!ValidateSemanticMaterials(Sector, OutFailure))
    {
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
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_WORLD_MATERIAL_BASELINE_READY ground_authored=1 authored_surface_families=5 basicshape_surface_materials=0 owner=OCWorldSectorOster"));
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
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_WORLD_MATERIAL_STABLE ground_authored=1 authored_surface_families=5 samples=12s,16s,20s result=semantic_material_contract_preserved"));
    }
}
