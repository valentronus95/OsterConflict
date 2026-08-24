#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "OCCentralPlayableAreaSubsystem.generated.h"

/**
 * Pass 44 one-shot runtime trim for the old 2.4 km source-only Oster sector.
 *
 * The user-approved 2026-08-24 map reference defines a compact central battlefield.
 * This subsystem removes legacy procedural instances outside that area and resizes the
 * source ground so those old peripherals no longer inflate render/navigation/map bounds.
 */
UCLASS()
class OSTERCONFLICT_API UOCCentralPlayableAreaSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

private:
    FTimerHandle ApplyTimer;
    int32 ApplyAttempts = 0;

    void ApplyCompactPlayableArea();
};
