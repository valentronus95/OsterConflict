#pragma once

#include "CoreMinimal.h"
#include "OCVehicleSpawnPoint.h"
#include "OCCombatVehicleSpawnPoints.generated.h"

/** Production armed pickup spawn point. */
UCLASS()
class OSTERCONFLICT_API AOCPickupGunTruckSpawnPoint : public AOCVehicleSpawnPoint
{
    GENERATED_BODY()
public:
    AOCPickupGunTruckSpawnPoint();
};

/** Production HMMWV + M2 spawn point. */
UCLASS()
class OSTERCONFLICT_API AOCHMMWVGunTruckSpawnPoint : public AOCVehicleSpawnPoint
{
    GENERATED_BODY()
public:
    AOCHMMWVGunTruckSpawnPoint();
};

UCLASS()
class OSTERCONFLICT_API AOCBTRSpawnPoint : public AOCVehicleSpawnPoint
{
    GENERATED_BODY()
public:
    AOCBTRSpawnPoint();
};
