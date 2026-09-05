#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "OCBlock0GroundFoundationSubsystem.generated.h"

/**
 * PASS45 Block 0 ground foundation.
 *
 * The source-sized Ground proxy remains lightweight while frontend/deployment UI is active.
 * Tracked authored mesh/material packages are applied only after the local player has actually
 * deployed, so synchronous package loads cannot stall Slate or the native Windows message pump.
 */
UCLASS()
class OSTERCONFLICT_API UOCBlock0GroundFoundationSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

private:
    void TryApplyWhenGameplayReady();

    FTimerHandle GameplayReadyTimer;
    bool bGroundAttemptFinished = false;
};