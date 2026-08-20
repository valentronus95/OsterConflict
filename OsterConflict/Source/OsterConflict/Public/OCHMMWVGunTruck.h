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

protected:
    virtual bool ShouldUseHMMWVProductionVisual() const override { return true; }
};
