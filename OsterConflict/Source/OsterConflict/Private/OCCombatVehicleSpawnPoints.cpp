#include "OCCombatVehicleSpawnPoints.h"
#include "OCPickupGunTruck.h"
#include "OCBTR.h"

AOCPickupGunTruckSpawnPoint::AOCPickupGunTruckSpawnPoint()
{
    VehicleClass = AOCPickupGunTruck::StaticClass();
    RespawnDelaySeconds = 48.0f;
}

AOCBTRSpawnPoint::AOCBTRSpawnPoint()
{
    VehicleClass = AOCBTR::StaticClass();
    RespawnDelaySeconds = 72.0f;
}
