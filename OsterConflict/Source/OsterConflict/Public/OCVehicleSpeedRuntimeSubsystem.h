#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCVehicleSpeedRuntimeSubsystem.generated.h"

class AOCVehicleBase;
class UPrimitiveComponent;

/**
 * Enforces the current runtime vehicle-speed acceptance contract on the source-only rigid-body vehicles.
 * This remains temporary until the production vehicles move to authored Chaos vehicle setups.
 */
UCLASS()
class OSTERCONFLICT_API UOCVehicleSpeedRuntimeSubsystem final : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;

private:
    void ApplySpeedContract(AOCVehicleBase& Vehicle, float TargetKmh, float AssistAccelerationCmPerSecSq);
    bool HasGroundContact(const AOCVehicleBase& Vehicle, const UPrimitiveComponent& Body) const;
    static float DamagePowerScale(const AOCVehicleBase& Vehicle);
};
