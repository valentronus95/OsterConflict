#include "OCFoliageRuntimeGuardSubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"

namespace
{
    const FName DenseFoliageActorTag(TEXT("OC_DenseGroundFoliage"));
    const FName Block0PopulationCompleteTag(TEXT("OC_Block0FullMapGrassComplete"));

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

    const FName ProxyGroundCoverNames[]
    {
        TEXT("GrassMown"),
        TEXT("GrassRough"),
        TEXT("GrassWetland")
    };
    const FName DeveloperTextLabelNames[]
    {
        TEXT("MuseumLabel"),
        TEXT("StadiumLabel"),
        TEXT("ParkLabel"),
        TEXT("CollegeLabel"),
        TEXT("KrushelnytskaStreetLabel")
    };
    const FName RejectedPrimitiveTreeProxyNames[]
    {
        TEXT("TreeTrunks"),
        TEXT("TreeCrowns"),
        TEXT("SovietPoplarTrunks"),
        TEXT("SovietPoplarCrowns"),
        TEXT("BirchTrunks"),
        TEXT("BirchCrowns"),
        TEXT("PineTrunks"),
        TEXT("PineCrowns")
    };

    struct FRuntimeTreeFamilyExpectation
    {
        FName ComponentName;
        const TCHAR* MeshPath;
        const TCHAR* Label;
    };

    // PASS45 item 27: this is the final player-facing mapping after UOCTreeContentUpgradeSubsystem runs.
    // Merely being non-primitive is not enough. If the one-shot upgrade fails and the older authoring mesh stays
    // installed, the runtime guard must fail instead of emitting a misleading vegetation READY marker.
    const FRuntimeTreeFamilyExpectation RuntimeTreeFamilies[]
    {
        {
            TEXT("AuthoredDeciduousTrees"),
            TEXT("/Game/KiteDemo/Environments/Trees/HillTree_02/HillTree_02.HillTree_02"),
            TEXT("mixed_deciduous")
        },
        {
            TEXT("AuthoredPine01Trees"),
            TEXT("/Game/KiteDemo/Environments/Trees/ScotsPine_01/ScotsPine_01.ScotsPine_01"),
            TEXT("scots_pine")
        },
        {
            TEXT("AuthoredPine03Trees"),
            TEXT("/Game/KiteDemo/Environments/Trees/ScotsPineTall_01/ScotsPineTall_01.ScotsPineTall_01"),
            TEXT("scots_pine_tall")
        }
    };

    bool IsLowCPUProfile(const UWorld& World)
    {
        const TCHAR* Value = World.URL.GetOption(TEXT("PerfProfile="), TEXT(""));
        return Value && FString(Value).Equals(TEXT("LowCPU"), ESearchCase::IgnoreCase);
    }

    bool HasCompletedDenseFoliagePopulation(UWorld& World)
    {
        int32 DenseActorCount = 0;
        bool bPopulationComplete = false;
        for (TActorIterator<AActor> It(&World); It; ++It)
        {
            AActor* Actor = *It;
            if (!Actor || !Actor->ActorHasTag(DenseFoliageActorTag)) continue;
            ++DenseActorCount;
            bPopulationComplete = Actor->ActorHasTag(Block0PopulationCompleteTag);
        }
        return DenseActorCount == 1 && bPopulationComplete;
    }

    int32 CoverageBin(const float Value, const float MinValue, const float MaxValue)
    {
        const float Alpha = FMath::Clamp((Value - MinValue) / (MaxValue - MinValue), 0.0f, 1.0f);
        return FMath::Clamp(FMath::FloorToInt(Alpha * CoverageBinsPerAxis), 0, CoverageBinsPerAxis - 1);
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

    UTextRenderComponent* FindText(AActor* Actor, const FName Name)
    {
        if (!Actor) return nullptr;
        TInlineComponentArray<UTextRenderComponent*> Components;
        Actor->GetComponents(Components);
        for (UTextRenderComponent* Component : Components)
        {
            if (Component && Component->GetFName() == Name) return Component;
        }
        return nullptr;
    }

    bool IsRejectedPrimitiveTreeMesh(const UStaticMesh* Mesh)
    {
        if (!Mesh) return true;
        const FString Path = Mesh->GetPathName();
        return Path.Contains(TEXT("/Engine/BasicShapes/Cylinder"), ESearchCase::IgnoreCase) ||
            Path.Contains(TEXT("/Engine/BasicShapes/Sphere"), ESearchCase::IgnoreCase);
    }
}

bool UOCFoliageRuntimeGuardSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && World->GetNetMode() != NM_DedicatedServer &&
        (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

TStatId UOCFoliageRuntimeGuardSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCFoliageRuntimeGuardSubsystem, STATGROUP_Tickables);
}

void UOCFoliageRuntimeGuardSubsystem::FailValidation(const FString& Reason)
{
    if (bFinished) return;
    bFinished = true;
    UE_LOG(LogTemp, Error, TEXT("PASS10_FOLIAGE_RUNTIME_FAIL reason=%s"), *Reason);
}

bool UOCFoliageRuntimeGuardSubsystem::DestroySourceGroundCoverProxies()
{
    UWorld* World = GetWorld();
    if (!World) return false;

    bool bFoundSector = false;
    int32 DestroyedComponents = 0;
    int32 RemainingComponents = 0;

    for (TActorIterator<AOCWorldSectorOster> It(World); It; ++It)
    {
        AOCWorldSectorOster* Sector = *It;
        if (!Sector) continue;
        bFoundSector = true;

        for (const FName ProxyName : ProxyGroundCoverNames)
        {
            if (UInstancedStaticMeshComponent* Proxy = FindISM(Sector, ProxyName))
            {
                Proxy->SetVisibility(false, true);
                Proxy->SetHiddenInGame(true, true);
                Proxy->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                Proxy->SetGenerateOverlapEvents(false);
                Proxy->SetCanEverAffectNavigation(false);
                Proxy->SetCastShadow(false);
                Proxy->DestroyComponent();
                ++DestroyedComponents;
            }
        }

        for (const FName ProxyName : ProxyGroundCoverNames)
        {
            if (FindISM(Sector, ProxyName)) ++RemainingComponents;
        }
    }

    const bool bReady = bFoundSector && RemainingComponents == 0;
    if (bReady && !bGroundProxyDestructionObserved)
    {
        bGroundProxyDestructionObserved = true;
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_GROUND_COVER_PRIMITIVES_DESTROYED destroyed=%d remaining=%d names=GrassMown,GrassRough,GrassWetland replacement=OC_DenseGroundFoliage"),
            DestroyedComponents,
            RemainingComponents);
    }
    return bReady;
}

bool UOCFoliageRuntimeGuardSubsystem::DestroyDeveloperVisualMarkers()
{
    UWorld* World = GetWorld();
    if (!World) return false;

    bool bFoundSector = false;
    int32 DestroyedMarkers = 0;
    int32 DestroyedLabels = 0;
    int32 Remaining = 0;

    for (TActorIterator<AOCWorldSectorOster> It(World); It; ++It)
    {
        AOCWorldSectorOster* Sector = *It;
        if (!Sector) continue;
        bFoundSector = true;

        if (UInstancedStaticMeshComponent* Marker = FindISM(Sector, TEXT("ReferenceMarkers")))
        {
            Marker->SetVisibility(false, true);
            Marker->SetHiddenInGame(true, true);
            Marker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            Marker->DestroyComponent();
            ++DestroyedMarkers;
        }

        for (const FName LabelName : DeveloperTextLabelNames)
        {
            if (UTextRenderComponent* Label = FindText(Sector, LabelName))
            {
                Label->SetVisibility(false, true);
                Label->SetHiddenInGame(true, true);
                Label->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                Label->DestroyComponent();
                ++DestroyedLabels;
            }
        }

        if (FindISM(Sector, TEXT("ReferenceMarkers"))) ++Remaining;
        for (const FName LabelName : DeveloperTextLabelNames)
        {
            if (FindText(Sector, LabelName)) ++Remaining;
        }
    }

    const bool bReady = bFoundSector && Remaining == 0;
    if (bReady && !bDeveloperMarkerDestructionObserved)
    {
        bDeveloperMarkerDestructionObserved = true;
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_DEVELOPER_WORLD_MARKERS_DESTROYED marker_components=%d text_labels=%d remaining=%d tactical_map_semantics_preserved=1"),
            DestroyedMarkers,
            DestroyedLabels,
            Remaining);
    }
    return bReady;
}

bool UOCFoliageRuntimeGuardSubsystem::ValidateSourceAuthoredTrees()
{
    UWorld* World = GetWorld();
    if (!World) return false;

    bool bFoundSector = false;
    bool bAllValid = true;
    int32 AuthoredComponents = 0;
    int32 AuthoredInstances = 0;
    int32 RejectedProxyComponents = 0;
    int32 PrimitiveTreeMeshes = 0;
    int32 RuntimeIdentityMismatches = 0;

    for (TActorIterator<AOCWorldSectorOster> It(World); It; ++It)
    {
        AOCWorldSectorOster* Sector = *It;
        if (!Sector) continue;
        bFoundSector = true;

        for (const FName RejectedName : RejectedPrimitiveTreeProxyNames)
        {
            if (FindISM(Sector, RejectedName))
            {
                ++RejectedProxyComponents;
                bAllValid = false;
            }
        }

        for (const FRuntimeTreeFamilyExpectation& Family : RuntimeTreeFamilies)
        {
            UInstancedStaticMeshComponent* Component = FindISM(Sector, Family.ComponentName);
            if (!Component || !Component->GetStaticMesh() || Component->GetInstanceCount() <= 0)
            {
                ++RuntimeIdentityMismatches;
                bAllValid = false;
                UE_LOG(LogTemp, Error,
                    TEXT("PASS45_RUNTIME_TREE_IDENTITY_FAIL family=%s reason=component_mesh_or_instances_missing expected=%s runtime_acceptance=0"),
                    Family.Label,
                    Family.MeshPath);
                continue;
            }

            UStaticMesh* Mesh = Component->GetStaticMesh();
            if (IsRejectedPrimitiveTreeMesh(Mesh))
            {
                ++PrimitiveTreeMeshes;
                ++RuntimeIdentityMismatches;
                bAllValid = false;
                UE_LOG(LogTemp, Error,
                    TEXT("PASS45_RUNTIME_TREE_IDENTITY_FAIL family=%s reason=primitive_tree_mesh expected=%s actual=%s runtime_acceptance=0"),
                    Family.Label,
                    Family.MeshPath,
                    *Mesh->GetPathName());
                continue;
            }

            const FString ActualPath = Mesh->GetPathName();
            if (!ActualPath.Equals(Family.MeshPath, ESearchCase::CaseSensitive))
            {
                ++RuntimeIdentityMismatches;
                bAllValid = false;
                UE_LOG(LogTemp, Error,
                    TEXT("PASS45_RUNTIME_TREE_IDENTITY_FAIL family=%s reason=unexpected_runtime_tree_mesh expected=%s actual=%s runtime_acceptance=0"),
                    Family.Label,
                    Family.MeshPath,
                    *ActualPath);
                continue;
            }

            ++AuthoredComponents;
            AuthoredInstances += Component->GetInstanceCount();
        }
    }

    const bool bReady = bFoundSector && bAllValid && RejectedProxyComponents == 0 && PrimitiveTreeMeshes == 0 &&
        RuntimeIdentityMismatches == 0 &&
        AuthoredComponents == static_cast<int32>(UE_ARRAY_COUNT(RuntimeTreeFamilies));

    if (bReady && !bAuthoredTreeValidationObserved)
    {
        bAuthoredTreeValidationObserved = true;
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_RUNTIME_TREE_IDENTITY_READY exact_runtime_tree_components=%d runtime_identity_mismatches=%d final_mapping=HillTree_02,ScotsPine_01,ScotsPineTall_01 primitive_tree_meshes=%d oak_asset_verified=0 runtime_acceptance=0"),
            AuthoredComponents,
            RuntimeIdentityMismatches,
            PrimitiveTreeMeshes);
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_AUTHORED_VEGETATION_READY authored_components=%d authored_instances=%d rejected_proxy_components=%d primitive_tree_meshes=%d pine_assets=SM_Pine_Tree_01,SM_Pine_Tree_03 final_runtime_pine_assets=ScotsPine_01,ScotsPineTall_01 exact_runtime_identity=1 oak_asset_verified=0"),
            AuthoredComponents,
            AuthoredInstances,
            RejectedProxyComponents,
            PrimitiveTreeMeshes);
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_AUTHORED_TREE_FAMILY_READY primitive_tree_components=0 authored_tree_components=3 basicshape_tree_meshes=0 authored_instances=%d pine_assets=SM_Pine_Tree_01,SM_Pine_Tree_03 final_runtime_pine_assets=ScotsPine_01,ScotsPineTall_01 exact_runtime_identity=1 oak_asset_verified=0"),
            AuthoredInstances);
    }

    return bReady;
}

bool UOCFoliageRuntimeGuardSubsystem::ValidateDenseFoliage(
    int32 MinGrassInstances,
    int32& OutGrassInstances,
    int32& OutDenseGrassComponents,
    int32& OutOccupiedBins,
    int32 OutQuadrantOccupied[4],
    bool& bOutEdgeReach) const
{
    OutGrassInstances = 0;
    OutDenseGrassComponents = 0;
    OutOccupiedBins = 0;
    OutQuadrantOccupied[0] = 0;
    OutQuadrantOccupied[1] = 0;
    OutQuadrantOccupied[2] = 0;
    OutQuadrantOccupied[3] = 0;
    bOutEdgeReach = false;

    UWorld* World = GetWorld();
    if (!World) return false;

    AActor* DenseActor = nullptr;
    int32 DenseActorCount = 0;
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        if (Actor && Actor->ActorHasTag(DenseFoliageActorTag))
        {
            DenseActor = Actor;
            ++DenseActorCount;
        }
    }

    if (DenseActorCount != 1 || !DenseActor || !DenseActor->ActorHasTag(Block0PopulationCompleteTag)) return false;

    bool OccupiedBins[CoverageBinCount] = {};
    float ObservedMinX = BIG_NUMBER;
    float ObservedMaxX = -BIG_NUMBER;
    float ObservedMinY = BIG_NUMBER;
    float ObservedMaxY = -BIG_NUMBER;

    TInlineComponentArray<UHierarchicalInstancedStaticMeshComponent*> Components;
    DenseActor->GetComponents(Components);
    for (UHierarchicalInstancedStaticMeshComponent* Component : Components)
    {
        if (!Component) continue;
        const FString Name = Component->GetName();
        if (!Name.StartsWith(TEXT("DenseGrass_"))) continue;

        ++OutDenseGrassComponents;
        if (Component->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
        {
            return false;
        }

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

            ++OutGrassInstances;
            ObservedMinX = FMath::Min(ObservedMinX, Location.X);
            ObservedMaxX = FMath::Max(ObservedMaxX, Location.X);
            ObservedMinY = FMath::Min(ObservedMinY, Location.Y);
            ObservedMaxY = FMath::Max(ObservedMaxY, Location.Y);

            const int32 BinX = CoverageBin(Location.X, CompactMinX, CompactMaxX);
            const int32 BinY = CoverageBin(Location.Y, CompactMinY, CompactMaxY);
            OccupiedBins[BinY * CoverageBinsPerAxis + BinX] = true;
        }
    }

    for (int32 BinY = 0; BinY < CoverageBinsPerAxis; ++BinY)
    {
        for (int32 BinX = 0; BinX < CoverageBinsPerAxis; ++BinX)
        {
            if (!OccupiedBins[BinY * CoverageBinsPerAxis + BinX]) continue;
            ++OutOccupiedBins;
            const int32 QuadrantX = BinX >= CoverageBinsPerAxis / 2 ? 1 : 0;
            const int32 QuadrantY = BinY >= CoverageBinsPerAxis / 2 ? 1 : 0;
            ++OutQuadrantOccupied[QuadrantY * 2 + QuadrantX];
        }
    }

    if (OutGrassInstances > 0)
    {
        const float EdgeToleranceX = CompactWidthCm * EdgeToleranceFraction;
        const float EdgeToleranceY = CompactHeightCm * EdgeToleranceFraction;
        bOutEdgeReach =
            ObservedMinX <= CompactMinX + EdgeToleranceX &&
            ObservedMaxX >= CompactMaxX - EdgeToleranceX &&
            ObservedMinY <= CompactMinY + EdgeToleranceY &&
            ObservedMaxY >= CompactMaxY - EdgeToleranceY;
    }

    const bool bQuadrantsReady =
        OutQuadrantOccupied[0] >= MinOccupiedBinsPerQuadrant &&
        OutQuadrantOccupied[1] >= MinOccupiedBinsPerQuadrant &&
        OutQuadrantOccupied[2] >= MinOccupiedBinsPerQuadrant &&
        OutQuadrantOccupied[3] >= MinOccupiedBinsPerQuadrant;

    return OutDenseGrassComponents > 0 &&
        OutGrassInstances >= MinGrassInstances &&
        OutOccupiedBins >= MinOccupiedBins &&
        bQuadrantsReady &&
        bOutEdgeReach;
}

void UOCFoliageRuntimeGuardSubsystem::Tick(float DeltaTime)
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

    // Acceptance guard, not gameplay logic. Sample at 4 Hz and stop touching source components once validation is proven.
    if (ValidationAccumulator < 0.25f) return;
    ValidationAccumulator = 0.0f;

    const bool bGroundProxiesDestroyed = bGroundProxyDestructionObserved || DestroySourceGroundCoverProxies();
    const bool bDeveloperMarkersDestroyed = bDeveloperMarkerDestructionObserved || DestroyDeveloperVisualMarkers();
    const bool bAuthoredTreesReady = bAuthoredTreeValidationObserved || ValidateSourceAuthoredTrees();

    if (ElapsedSeconds < 2.0f) return;

    const bool bLowCPU = IsLowCPUProfile(*World);
    const int32 MinGrassInstances = bLowCPU ? 48 : 250;
    const bool bPopulationComplete = HasCompletedDenseFoliagePopulation(*World);
    int32 GrassInstances = 0;
    int32 DenseGrassComponents = 0;
    int32 OccupiedBins = 0;
    int32 QuadrantOccupied[4] = {};
    bool bEdgeReach = false;
    const bool bDenseReady = bPopulationComplete && ValidateDenseFoliage(
        MinGrassInstances,
        GrassInstances,
        DenseGrassComponents,
        OccupiedBins,
        QuadrantOccupied,
        bEdgeReach);

    if (bGroundProxiesDestroyed && bDeveloperMarkersDestroyed && bAuthoredTreesReady && bDenseReady)
    {
        bFinished = true;
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_BLOCK0_SPATIAL_GRASS_COVERAGE_READY grass=%d occupied_bins=%d/%d quadrants=%d,%d,%d,%d edge_reach=1 full_playable_distribution=1 strict_runtime_owner=OCFoliageRuntimeGuard mutation=0 runtime_acceptance=0"),
            GrassInstances,
            OccupiedBins,
            CoverageBinCount,
            QuadrantOccupied[0],
            QuadrantOccupied[1],
            QuadrantOccupied[2],
            QuadrantOccupied[3]);
        UE_LOG(LogTemp, Display,
            TEXT("PASS10_FOLIAGE_RUNTIME_READY groundProxyComponents=0 authoredTreeComponents=3 primitiveTreeProxyComponents=0 denseGrassComponents=%d grassInstances=%d minRequired=%d profile=%s developerMarkers=0 population_complete=1 full_playable_bounds=1 spatial_coverage=1 occupied_bins=%d/%d edge_reach=1 exact_runtime_tree_identity=1"),
            DenseGrassComponents,
            GrassInstances,
            MinGrassInstances,
            bLowCPU ? TEXT("LowCPU") : TEXT("Full"),
            OccupiedBins,
            CoverageBinCount);
        if (bLowCPU)
        {
            UE_LOG(LogTemp, Display,
                TEXT("PASS36_LOWCPU_FOLIAGE_RUNTIME_READY grassInstances=%d minRequired=%d full_sector_population=1 population_complete=1 density_policy_only=1 spatial_coverage=1 occupied_bins=%d/%d edge_reach=1"),
                GrassInstances,
                MinGrassInstances,
                OccupiedBins,
                CoverageBinCount);
        }
        UE_LOG(LogTemp, Display,
            TEXT("PASS42_FOLIAGE_GUARD_THROTTLED_READY sample_hz=4 proxy_rescan_after_ready=0 spatial_scan_terminal=1"));
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_VISUAL_CLEANUP_PARTIAL_READY ground_cover_cube_proxies=0 developer_reference_markers=0 developer_text_labels=0 authored_dense_foliage=1 native_render_scale_required=1 gate_k_complete=0"));
        return;
    }

    if (ElapsedSeconds < (bLowCPU ? 8.0f : 25.0f)) return;

    if (!bGroundProxiesDestroyed)
    {
        FailValidation(TEXT("source_ground_cover_proxy_not_destroyed"));
        return;
    }
    if (!bDeveloperMarkersDestroyed)
    {
        FailValidation(TEXT("developer_world_markers_not_destroyed"));
        return;
    }
    if (!bAuthoredTreesReady)
    {
        FailValidation(TEXT("final_runtime_tree_identity_not_ready"));
        return;
    }
    if (!bPopulationComplete)
    {
        FailValidation(TEXT("full_map_foliage_population_incomplete"));
        return;
    }
    if (DenseGrassComponents <= 0)
    {
        FailValidation(TEXT("dense_grass_components_missing"));
        return;
    }
    if (GrassInstances < MinGrassInstances)
    {
        FailValidation(FString::Printf(TEXT("dense_grass_instances_%d_lt_%d"), GrassInstances, MinGrassInstances));
        return;
    }
    if (OccupiedBins < MinOccupiedBins ||
        QuadrantOccupied[0] < MinOccupiedBinsPerQuadrant ||
        QuadrantOccupied[1] < MinOccupiedBinsPerQuadrant ||
        QuadrantOccupied[2] < MinOccupiedBinsPerQuadrant ||
        QuadrantOccupied[3] < MinOccupiedBinsPerQuadrant ||
        !bEdgeReach)
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_BLOCK0_SPATIAL_GRASS_COVERAGE_FAIL grass=%d occupied_bins=%d/%d quadrants=%d,%d,%d,%d edge_reach=%d full_playable_distribution=0 strict_runtime_owner=OCFoliageRuntimeGuard mutation=0 runtime_acceptance=0"),
            GrassInstances,
            OccupiedBins,
            CoverageBinCount,
            QuadrantOccupied[0],
            QuadrantOccupied[1],
            QuadrantOccupied[2],
            QuadrantOccupied[3],
            bEdgeReach ? 1 : 0);
        FailValidation(TEXT("block0_spatial_grass_distribution_insufficient"));
        return;
    }

    FailValidation(TEXT("dense_foliage_collision_contract_failed"));
}
