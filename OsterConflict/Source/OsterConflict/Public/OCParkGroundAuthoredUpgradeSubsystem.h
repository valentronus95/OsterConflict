#pragma once

#include "CoreMinimal.h"
#include "Engine/StreamableManager.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCParkGroundAuthoredUpgradeSubsystem.generated.h"

/**
 * PASS45 Gate K authored ground presentation for exact semantic owners created directly by AOCWorldSectorOster.
 *
 * GAME_RECOVERY prepares ParkCentralGround, ParkNorthCivicGround and CollegeRecreationGround before human spawn.
 * The tracked plane + grass material pair is async-preloaded and applied from resident memory only. XY footprint,
 * yaw and source surface-top elevation are preserved, and the whole three-owner mutation rolls back if a write fails.
 *
 * Semantic ownership is primary_authoring=1 / normalization_bridge=0. Source readiness does not constitute
 * direct UE 5.8 visual acceptance.
 */
UCLASS()
class OSTERCONFLICT_API UOCParkGroundAuthoredUpgradeSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

    bool IsParkGroundReady() const { return bFinished && bSucceeded; }
    bool HasParkGroundFailed() const { return bFinished && !bSucceeded; }
    float GetParkGroundProgress() const
    {
        if (bFinished) return bSucceeded ? 1.0f : 0.0f;
        return bPreloadRequested ? 0.50f : 0.0f;
    }

private:
    void BeginGroundPreload();
    void HandleGroundPreloadComplete();

    TSharedPtr<FStreamableHandle> GroundPreloadHandle;
    bool bPreloadRequested = false;
    bool bFinished = false;
    bool bSucceeded = false;
};
