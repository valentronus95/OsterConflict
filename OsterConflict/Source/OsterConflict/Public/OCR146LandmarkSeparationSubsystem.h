#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "OCR146LandmarkSeparationSubsystem.generated.h"

class AActor;

/**
 * Runtime exclusion/integrity guard for the canonical Museum, Silpo and Culture House parcels.
 *
 * This subsystem is deliberately NOT a landmark placement owner. The three photo-model systems keep
 * exclusive ownership of their own geometry. The guard only removes foreign generic/legacy geometry,
 * watches late legacy actor spawns and verifies the world after startup placement passes have finished.
 */
UCLASS()
class OSTERCONFLICT_API UOCR146LandmarkSeparationSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

private:
    FTimerHandle StartupGuardTimer;
    FDelegateHandle ActorSpawnedHandle;
    TWeakObjectPtr<UWorld> GuardWorld;
    int32 StartupGuardPass = 0;

    void RunStartupGuardPass();
    void EnforceSeparation(UWorld& World, bool bFinalValidation) const;
    void HandleActorSpawned(AActor* Actor);
    static bool IsForbiddenLegacyLandmarkActor(const AActor* Actor);
};
