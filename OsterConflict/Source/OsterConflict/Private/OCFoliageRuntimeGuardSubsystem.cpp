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
    const FName AuthoredTreeComponentNames[]
    {
        TEXT("AuthoredDeciduousTrees"),
        TEXT("AuthoredPine01Trees"),
        TEXT("AuthoredPine03Trees")
    };

    bool IsLowCPUProfile(const UWorld& World)
    {
        const TCHAR* Value = World.URL.GetOption(TEXT("PerfProfile="), TEXT(""));
        return Value && FString(Value).Equals(TEXT("LowCPU"), ESearchCase::IgnoreCase);
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

        for (const FName AuthoredName : AuthoredTreeComponentNames)
        {
            UInstancedStaticMeshComponent* Component = FindISM(Sector, AuthoredName);
            if (!Component || !Component->GetStaticMesh() || Component->GetInstanceCount() <= 0)
            {
                bAllValid = false;
                continue;
            }

            if (IsRejectedPrimitiveTreeMesh(Component->GetStaticMesh()))
            {
                ++PrimitiveTreeMeshes;
                bAllValid = false;
                continue;
            }

            ++AuthoredComponents;
            AuthoredInstances += Component->GetInstanceCount();
        }
    }

    const bool bReady = bFoundSector && bAllValid && RejectedProxyComponents == 0 && PrimitiveTreeMeshes == 0 &&
        AuthoredComponents >= static_cast<int32>(UE_ARRAY_COUNT(AuthoredTreeComponentNames));

    if (bReady && !bAuthoredTreeValidationObserved)
    {
        bAuthoredTreeValidationObserved = true;
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_AUTHORED_VEGETATION_READY authored_components=%d authored_instances=%d rejected_proxy_components=%d primitive_tree_meshes=%d pine_assets=SM_Pine_Tree_01,SM_Pine_Tree_03 oak_asset_verified=0"),
            AuthoredComponents,
            AuthoredInstances,
            RejectedProxyComponents,
            PrimitiveTreeMeshes);
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_AUTHORED_TREE_FAMILY_READY primitive_tree_components=0 authored_tree_components=3 basicshape_tree_meshes=0 authored_instances=%d pine_assets=SM_Pine_Tree_01,SM_Pine_Tree_03 oak_asset_verified=0"),
            AuthoredInstances);
    }

    return bReady;
}

bool UOCFoliageRuntimeGuardSubsystem::ValidateDenseFoliage(
    int32 MinGrassInstances,
    int32& OutGrassInstances,
    int32& OutDenseGrassComponents) const
{
    OutGrassInstances = 0;
    OutDenseGrassComponents = 0;

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

    if (DenseActorCount != 1 || !DenseActor) return false;

    TInlineComponentArray<UHierarchicalInstancedStaticMeshComponent*> Components;
    DenseActor->GetComponents(Components);
    for (UHierarchicalInstancedStaticMeshComponent* Component : Components)
    {
        if (!Component) continue;
        const FString Name = Component->GetName();
        if (!Name.StartsWith(TEXT("DenseGrass_"))) continue;

        ++OutDenseGrassComponents;
        OutGrassInstances += Component->GetInstanceCount();

        if (Component->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
        {
            return false;
        }
    }

    return OutDenseGrassComponents > 0 && OutGrassInstances >= MinGrassInstances;
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
    int32 GrassInstances = 0;
    int32 DenseGrassComponents = 0;
    const bool bDenseReady = ValidateDenseFoliage(MinGrassInstances, GrassInstances, DenseGrassComponents);

    if (bGroundProxiesDestroyed && bDeveloperMarkersDestroyed && bAuthoredTreesReady && bDenseReady)
    {
        bFinished = true;
        UE_LOG(LogTemp, Display,
            TEXT("PASS10_FOLIAGE_RUNTIME_READY groundProxyComponents=0 authoredTreeComponents=3 primitiveTreeProxyComponents=0 denseGrassComponents=%d grassInstances=%d minRequired=%d profile=%s developerMarkers=0"),
            DenseGrassComponents,
            GrassInstances,
            MinGrassInstances,
            bLowCPU ? TEXT("LowCPU") : TEXT("Full"));
        if (bLowCPU)
        {
            UE_LOG(LogTemp, Display,
                TEXT("PASS36_LOWCPU_FOLIAGE_RUNTIME_READY grassInstances=%d minRequired=%d full_sector_population=0"),
                GrassInstances,
                MinGrassInstances);
        }
        UE_LOG(LogTemp, Display,
            TEXT("PASS42_FOLIAGE_GUARD_THROTTLED_READY sample_hz=4 proxy_rescan_after_ready=0"));
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
        FailValidation(TEXT("source_authored_tree_family_not_ready"));
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

    FailValidation(TEXT("dense_foliage_collision_contract_failed"));
}
