#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "OCR146LandmarkSeparationSubsystem.generated.h"

/**
 * Pass 45 validation-only integrity check for Museum / Silpo / Culture House parcels.
 *
 * Historical versions removed instances and destroyed late actors after startup. That made final output depend on
 * timer order and hid primary-authoring defects. Current code never mutates landmark/world geometry. It observes
 * once after the coordinated startup window and fails visibly when forbidden legacy owners or generic overlap exist.
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
    FTimerHandle ValidationTimer;
    TWeakObjectPtr<UWorld> ValidationWorld;

    void ValidateSeparation();
};
