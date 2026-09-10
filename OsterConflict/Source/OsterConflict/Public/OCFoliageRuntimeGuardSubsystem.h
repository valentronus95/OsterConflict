#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCFoliageRuntimeGuardSubsystem.generated.h"

class AActor;
class AOCWorldSectorOster;

/**
 * Proves that normal runtime vegetation is owned by authored tree/foliage meshes and physically retires
 * obsolete source ground-cover/debug presentation components. PASS45 items 26/31 do not allow hidden
 * Cube/Cylinder/Sphere scenery to survive as player-facing runtime content.
 *
 * Block 0 also makes this guard the single strict runtime owner for factual grass distribution acceptance.
 * Population completion/count alone is not enough: accepted DenseGrass instances must occupy the compact
 * Oster footprint spatially before PASS10/PASS36 READY can be emitted.
 */
UCLASS()
class OSTERCONFLICT_API UOCFoliageRuntimeGuardSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override { return !bFinished; }
    virtual bool IsTickableWhenPaused() const override { return true; }

private:
    void InitializeRuntimeActorCache(UWorld& World);
    void SeedRuntimeActorCache(UWorld& World);
    void HandleActorSpawned(AActor* SpawnedActor);
    void TrackRuntimeActor(AActor* Actor);
    AActor* GetSingleDenseFoliageActor();
    bool HasCompletedDenseFoliagePopulation();

    bool DestroySourceGroundCoverProxies();
    bool DestroyDeveloperVisualMarkers();
    bool ValidateSourceAuthoredTrees();
    bool ValidateDenseFoliage(
        AActor* DenseActor,
        int32 MinGrassInstances,
        int32& OutGrassInstances,
        int32& OutDenseGrassComponents,
        int32& OutOccupiedBins,
        int32 OutQuadrantOccupied[4],
        bool& bOutEdgeReach) const;
    void FailValidation(const FString& Reason);

    TWeakObjectPtr<AOCWorldSectorOster> WorldSectorActor;
    TArray<TWeakObjectPtr<AActor>> DenseFoliageActors;
    FDelegateHandle ActorSpawnedHandle;

    float ElapsedSeconds = 0.0f;
    float ValidationAccumulator = 0.0f;
    float DenseValidationRetryAtSeconds = -1.0f;
    bool bFinished = false;
    bool bActorCacheInitialized = false;
    bool bCacheSeedRetried = false;
    bool bGroundProxyDestructionObserved = false;
    bool bDeveloperMarkerDestructionObserved = false;
    bool bAuthoredTreeValidationObserved = false;
    bool bDenseValidationSampled = false;
    bool bDenseValidationRetried = false;
    bool bDenseReadyCached = false;

    int32 CachedGrassInstances = 0;
    int32 CachedDenseGrassComponents = 0;
    int32 CachedOccupiedBins = 0;
    int32 CachedQuadrantOccupied[4] = {};
    bool bCachedEdgeReach = false;
};