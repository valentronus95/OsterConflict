#pragma once

#include "CoreMinimal.h"
#include "Engine/StreamableManager.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCParkHardscapeAuthoredUpgradeSubsystem.generated.h"

/**
 * PASS45 Gate K authored hardscape presentation for exact flat semantic owners created directly by AOCWorldSectorOster.
 *
 * GAME_RECOVERY prepares ParkMemorialSurface and ParkSkateSurface before human spawn. The tracked authored plane
 * and concrete material are async-preloaded and applied from resident memory only. ParkMemorialMonument and
 * ParkSkateRamps remain separate content gaps and are deliberately not mutated here. XY footprint, yaw and source
 * top-surface elevation are preserved, with transaction rollback if either write fails.
 *
 * Semantic ownership is primary_authoring=1 / normalization_bridge=0. Source readiness does not constitute
 * direct UE 5.8 visual acceptance.
 */
UCLASS()
class OSTERCONFLICT_API UOCParkHardscapeAuthoredUpgradeSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

    bool IsParkHardscapeReady() const { return bFinished && bSucceeded; }
    bool HasParkHardscapeFailed() const { return bFinished && !bSucceeded; }
    float GetParkHardscapeProgress() const
    {
        if (bFinished) return bSucceeded ? 1.0f : 0.0f;
        return bPreloadRequested ? 0.50f : 0.0f;
    }

private:
    void BeginHardscapePreload();
    void HandleHardscapePreloadComplete();

    TSharedPtr<FStreamableHandle> HardscapePreloadHandle;
    bool bPreloadRequested = false;
    bool bFinished = false;
    bool bSucceeded = false;
};
