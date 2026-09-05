#pragma once

#include "CoreMinimal.h"
#include "OCPickupGunTruck.h"
#include "OCHMMWVGunTruck.generated.h"

/**
 * Dedicated HMMWV + mounted M2 production vehicle.
 *
 * It intentionally reuses the proven armed-vehicle gameplay/network implementation from
 * AOCPickupGunTruck while selecting the HMMWV production shell explicitly. This keeps the
 * pickup and HMMWV as two honest vehicle identities without duplicating authoritative logic.
 */
UCLASS()
class OSTERCONFLICT_API AOCHMMWVGunTruck : public AOCPickupGunTruck
{
    GENERATED_BODY()

public:
    AOCHMMWVGunTruck()
    {
        // PASS45 item 28: keep the HMMWV comfortably above the >=80 km/h gameplay floor while
        // separating its high-speed handling from the lighter pickup profile. The runtime speed
        // subsystem uses the same 110 km/h target and only supplies the force needed to overcome
        // the legacy source-only drag model.
        MaxForwardSpeedKmh = 110.0f;
        SteeringTorque = 68000000.0f;
        LateralGrip = 9800.0f;
    }

protected:
    virtual bool ShouldUseHMMWVProductionVisual() const override { return true; }
};
