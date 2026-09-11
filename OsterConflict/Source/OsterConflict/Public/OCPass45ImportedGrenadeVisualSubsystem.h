#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "OCPass45ImportedGrenadeVisualSubsystem.generated.h"

struct FStreamableHandle;

/**
 * Owns grenade presentation preload for GAME_RECOVERY and replaces the old shared grenade body with
 * exact imported Fab frag/smoke/flash visuals when present. Package loads are completed before deployment
 * releases the player, so throw/detonation paths never need a blocking LoadObject/GetAsset call.
 */
UCLASS()
class OSTERCONFLICT_API UOCPass45ImportedGrenadeVisualSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

    bool IsGrenadePresentationReady() const;
    float GetGrenadePresentationProgress() const;

private:
    void BeginPresentationPreload();
    void HandlePresentationPreloadReady();
    void HandleActorSpawned(AActor* Actor);
    void ScheduleRefresh(float DelaySeconds);
    void RefreshGrenadeVisuals();

    TSharedPtr<FStreamableHandle> PreloadHandle;
    FDelegateHandle ActorSpawnedHandle;
    FTimerHandle RefreshTimer;
    bool bPreloadRequested = false;
    bool bPreloadFailed = false;
    bool bPreloadReadyLogged = false;
};