#pragma once

#include "CoreMinimal.h"
#include "OCVehicleSpawnPoint.h"
#include "OCCombatVehicleSpawnPoints.generated.h"

UCLASS()
class OSTERCONFLICT_API AOCPickupGunTruckSpawnPoint : public AOCVehicleSpawnPoint
{
    GENERATED_BODY()
public:
    AOCPickupGunTruckSpawnPoint();
};

UCLASS()
class OSTERCONFLICT_API AOCBTRSpawnPoint : public AOCVehicleSpawnPoint
{
    GENERATED_BODY()
public:
    AOCBTRSpawnPoint();
};
