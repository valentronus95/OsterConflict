#include "OCCombatVehicleSpawnPoints.h"
#include "OCPickupGunTruck.h"
#include "OCHMMWVGunTruck.h"
#include "OCBTR.h"

AOCPickupGunTruckSpawnPoint::AOCPickupGunTruckSpawnPoint()
{
    VehicleClass = AOCPickupGunTruck::StaticClass();
    RespawnDelaySeconds = 48.0f;
}

AOCHMMWVGunTruckSpawnPoint::AOCHMMWVGunTruckSpawnPoint()
{
    VehicleClass = AOCHMMWVGunTruck::StaticClass();
    RespawnDelaySeconds = 48.0f;
}

AOCBTRSpawnPoint::AOCBTRSpawnPoint()
{
    VehicleClass = AOCBTR::StaticClass();
    RespawnDelaySeconds = 72.0f;
}
