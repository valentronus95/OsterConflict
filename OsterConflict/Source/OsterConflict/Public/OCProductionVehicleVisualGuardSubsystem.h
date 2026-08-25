#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCProductionVehicleVisualGuardSubsystem.generated.h"

/**
 * Pass45 read-only production vehicle visual validator.
 *
 * VehicleBase now skips legacy BasicShape tinting for every /Game/Production mesh at the primary
 * source. This subsystem must never repair or clear materials. It performs one delayed validation
 * and reports any override/material/content gap as a source failure.
 */
UCLASS()
class OSTERCONFLICT_API UOCProductionVehicleVisualGuardSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

private:
    FTimerHandle ValidationTimer;

    void ValidateProductionVisuals();
};
