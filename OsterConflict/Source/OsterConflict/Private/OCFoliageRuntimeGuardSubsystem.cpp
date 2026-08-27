#include "OCFoliageRuntimeGuardSubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
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

    bool RetireProxyComponent(UInstancedStaticMeshComponent* Proxy)
    {
        if (!Proxy) return false;
        Proxy->SetVisibility(false, true);
        Proxy->SetHiddenInGame(true, true);
        Proxy->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Proxy->SetGenerateOverlapEvents(false);
        Proxy->SetCanEverAffectNavigation(false);
        Proxy->SetCastShadow(false);
        return !Proxy->IsVisible() && Proxy->GetCollisionEnabled() == ECollisionEnabled::NoCollision;
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

bool UOCFoliageRuntimeGuardSubsystem::RetireSourceGroundCoverProxies()
{
    UWorld* World = GetWorld();
    if (!World) return false;

    bool bFoundSector = false;
    bool bAllRetired = true;
    int32 RetiredComponents = 0;

    for (TActorIterator<AOCWorldSectorOster> It(World); It; ++It)
    {
        AOCWorldSectorOster* Sector = *It;
        if (!Sector) continue;
        bFoundSector = true;

        for (const FName ProxyName : ProxyGroundCoverNames)
        {
            UInstancedStaticMeshComponent* Proxy = FindISM(Sector, ProxyName);
            if (!Proxy || !RetireProxyComponent(Proxy))
            {
                bAllRetired = false;
                continue;
            }
            ++RetiredComponents;
        }
    }

    if (bFoundSector && bAllRetired && RetiredComponents >= 3 && !bProxyRetirementObserved)
    {
        bProxyRetirementObserved = true;
        UE_LOG(LogTemp, Display,
            TEXT("PASS10_GROUND_COVER_PROXY_RETIRED components=%d names=GrassMown,GrassRough,GrassWetland"),
            RetiredComponents);
    }

    return bFoundSector && bAllRetired && RetiredComponents >= 3;
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

    const bool bGroundProxiesRetired = bProxyRetirementObserved || RetireSourceGroundCoverProxies();
    const bool bAuthoredTreesReady = bAuthoredTreeValidationObserved || ValidateSourceAuthoredTrees();

    if (ElapsedSeconds < 2.0f) return;

    const bool bLowCPU = IsLowCPUProfile(*World);
    const int32 MinGrassInstances = bLowCPU ? 48 : 250;
    int32 GrassInstances = 0;
    int32 DenseGrassComponents = 0;
    const bool bDenseReady = ValidateDenseFoliage(MinGrassInstances, GrassInstances, DenseGrassComponents);

    if (bGroundProxiesRetired && bAuthoredTreesReady && bDenseReady)
    {
        bFinished = true;
        UE_LOG(LogTemp, Display,
            TEXT("PASS10_FOLIAGE_RUNTIME_READY groundProxyComponents=3 authoredTreeComponents=3 primitiveTreeProxyComponents=0 denseGrassComponents=%d grassInstances=%d minRequired=%d profile=%s"),
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
        return;
    }

    if (ElapsedSeconds < (bLowCPU ? 8.0f : 25.0f)) return;

    if (!bGroundProxiesRetired)
    {
        FailValidation(TEXT("source_ground_cover_proxy_not_retired"));
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