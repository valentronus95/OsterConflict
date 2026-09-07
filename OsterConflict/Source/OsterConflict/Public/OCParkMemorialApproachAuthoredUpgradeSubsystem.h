#pragma once

#include "CoreMinimal.h"
#include "Engine/StreamableManager.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCParkMemorialApproachAuthoredUpgradeSubsystem.generated.h"

/**
 * PASS45 Gate K: replaces only the homogeneous ParkMemorialApproach source family with a tracked authored mesh.
 * GAME_RECOVERY async-preloads the curb mesh and applies the four-step family from resident memory before human spawn.
 * ParkMemorialPlaza, ParkSkateFitness, ParkBenches and legacy ParkDetails remain separate ownership domains.
 */
UCLASS()
class OSTERCONFLICT_API UOCParkMemorialApproachAuthoredUpgradeSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

    bool IsParkMemorialApproachReady() const { return bFinished && bSucceeded; }
    bool HasParkMemorialApproachFailed() const { return bFinished && !bSucceeded; }
    float GetParkMemorialApproachProgress() const
    {
        if (bFinished) return bSucceeded ? 1.0f : 0.0f;
        return bPreloadRequested ? 0.50f : 0.0f;
    }

private:
    void BeginMemorialApproachPreload();
    void HandleMemorialApproachPreloadComplete();

    TSharedPtr<FStreamableHandle> MemorialApproachPreloadHandle;
    bool bPreloadRequested = false;
    bool bFinished = false;
    bool bSucceeded = false;
};
