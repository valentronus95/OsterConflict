#pragma once

#include "CoreMinimal.h"
#include "OCVehicleSpawnPoint.h"
#include "OCCombatVehicleSpawnPoints.generated.h"

/**
 * Legacy combat-fleet spawn point name retained so existing GameMode/map code keeps the same
 * vehicle count and HMMWV behavior. Its VehicleClass is now the explicit AOCHMMWVGunTruck.
 */
UCLASS()
class OSTERCONFLICT_API AOCPickupGunTruckSpawnPoint : public AOCVehicleSpawnPoint
{
    GENERATED_BODY()
public:
    AOCPickupGunTruckSpawnPoint();
};

/** Explicit production HMMWV + M2 spawn point for new code. */
UCLASS()
class OSTERCONFLICT_API AOCHMMWVGunTruckSpawnPoint : public AOCVehicleSpawnPoint
{
    GENERATED_BODY()
public:
    AOCHMMWVGunTruckSpawnPoint();
};

/** Explicit production armed pickup + M2 spawn point. Not added to normal fleet balance yet. */
UCLASS()
class OSTERCONFLICT_API AOCProductionPickupGunTruckSpawnPoint : public AOCVehicleSpawnPoint
{
    GENERATED_BODY()
public:
    AOCProductionPickupGunTruckSpawnPoint();
};

UCLASS()
class OSTERCONFLICT_API AOCBTRSpawnPoint : public AOCVehicleSpawnPoint
{
    GENERATED_BODY()
public:
    AOCBTRSpawnPoint();
};
