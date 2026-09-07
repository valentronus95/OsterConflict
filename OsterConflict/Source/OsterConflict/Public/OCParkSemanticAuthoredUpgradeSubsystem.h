#pragma once

#include "CoreMinimal.h"
#include "Engine/StreamableManager.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCParkSemanticAuthoredUpgradeSubsystem.generated.h"

/**
 * PASS45 Gate K semantic Central Park authored-content upgrade.
 *
 * GAME_RECOVERY owns ParkBenches as pre-spawn world preparation. The tracked authored bench is
 * asynchronously preloaded during world startup, then applied from resident memory only. The subsystem
 * exposes factual READY/FAIL/progress state so deployment cannot release the human pawn while the park
 * is still mutating behind the player.
 *
 * The world source keeps deterministic semantic proxy transforms for park details. This subsystem only
 * upgrades homogeneous semantic families when an exact tracked authored family is available. It must not
 * blanket-remap ParkDetails, memorial, skate/fitness or other unrelated proxy groups.
 */
UCLASS()
class OSTERCONFLICT_API UOCParkSemanticAuthoredUpgradeSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

    bool IsParkSemanticReady() const { return bFinished && bSucceeded; }
    bool HasParkSemanticFailed() const { return bFinished && !bSucceeded; }
    float GetParkSemanticProgress() const
    {
        if (bFinished) return bSucceeded ? 1.0f : 0.0f;
        return bPreloadRequested ? 0.50f : 0.0f;
    }

private:
    void BeginBenchPreload();
    void HandleBenchPreloadComplete();

    TSharedPtr<FStreamableHandle> BenchPreloadHandle;
    bool bPreloadRequested = false;
    bool bFinished = false;
    bool bSucceeded = false;
};
