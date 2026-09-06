#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCAuthoredWorldSurfaceUpgradeSubsystem.generated.h"

struct FStreamableHandle;

/**
 * PASS45 item 31 runtime upgrade for canonical Oster Cube-authored topology that already has verified tracked assets.
 *
 * GAME RECOVERY treats these player-facing surface packages as pre-spawn world preparation. The subsystem async
 * preloads them while deployment owns the screen, applies the authored upgrade before possession, and exposes factual
 * readiness so the player is never used as a loading screen.
 */
UCLASS()
class OSTERCONFLICT_API UOCAuthoredWorldSurfaceUpgradeSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override { return !bFinished; }
    virtual bool IsTickableWhenPaused() const override { return true; }

    bool IsWorldSurfaceReady() const { return bInitialized && (!bEligible || (bFinished && bSucceeded)); }
    float GetWorldSurfaceProgress() const;

private:
    void BeginPreload();

    TSharedPtr<FStreamableHandle> PreloadHandle;
    double PreparationStartWallTimeSeconds = 0.0;
    bool bInitialized = false;
    bool bEligible = false;
    bool bPreloadRequested = false;
    bool bSucceeded = false;
    bool bFinished = false;
};
