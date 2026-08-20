#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCProductionVehicleRuntimeValidationSubsystem.generated.h"

/**
 * Runtime sanity check for production HMMWV, M2 Browning and BTR-4 visuals.
 *
 * The production integration deliberately keeps gameplay fallbacks alive, so a missing imported
 * asset would otherwise be easy to miss in PIE. This subsystem reports both asset availability
 * and actual use on spawned vehicle actors without changing authoritative vehicle/gameplay logic.
 */
UCLASS()
class OSTERCONFLICT_API UOCProductionVehicleRuntimeValidationSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void ValidateProductionVehicles(UWorld& World);
};
