#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCLocalInboxWorldAssetsSubsystem.generated.h"

struct FStreamableHandle;

/**
 * Replaces source-only BasicShape world presentation with meshes supplied through models_game_OC.
 * Existing authored transforms/collision remain authoritative; the local meshes become the visible layer.
 *
 * GAME RECOVERY: world-model packages are preloaded asynchronously and materialized while deployment owns
 * the screen. Normal gameplay must never trigger the historical delayed synchronous LoadObject burst.
 */
UCLASS()
class OSTERCONFLICT_API UOCLocalInboxWorldAssetsSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override { return bEligible && !bFinished; }
    virtual bool IsTickableWhenPaused() const override { return true; }

    bool IsWorldAssetsReady() const { return !bEligible || bFinished; }
    float GetWorldAssetsProgress() const;

private:
    void RequestPreload();
    void ApplyWorldAssets();

    TSharedPtr<FStreamableHandle> PreloadHandle;
    bool bEligible = false;
    bool bPreloadRequested = false;
    bool bPreloadComplete = false;
    bool bFinished = false;
    bool bSucceeded = false;
};
