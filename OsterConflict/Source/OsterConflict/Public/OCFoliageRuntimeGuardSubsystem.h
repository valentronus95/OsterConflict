#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCFoliageRuntimeGuardSubsystem.generated.h"

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
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override { return !bFinished; }

private:
    bool DestroySourceGroundCoverProxies();
    bool DestroyDeveloperVisualMarkers();
    bool ValidateSourceAuthoredTrees();
    bool ValidateDenseFoliage(
        int32 MinGrassInstances,
        int32& OutGrassInstances,
        int32& OutDenseGrassComponents,
        int32& OutOccupiedBins,
        int32 OutQuadrantOccupied[4],
        bool& bOutEdgeReach) const;
    void FailValidation(const FString& Reason);

    float ElapsedSeconds = 0.0f;
    float ValidationAccumulator = 0.0f;
    bool bFinished = false;
    bool bGroundProxyDestructionObserved = false;
    bool bDeveloperMarkerDestructionObserved = false;
    bool bAuthoredTreeValidationObserved = false;
};
