#include "OCCombatVehicleSpawnPoints.h"
#include "OCPickupGunTruck.h"
#include "OCHMMWVGunTruck.h"
#include "OCBTR.h"

AOCPickupGunTruckSpawnPoint::AOCPickupGunTruckSpawnPoint()
{
    // Legacy class name kept for existing GameMode/map references. Preserve current fleet behavior
    // by spawning the explicit HMMWV class instead of silently turning those slots into pickups.
    VehicleClass = AOCHMMWVGunTruck::StaticClass();
    RespawnDelaySeconds = 48.0f;
}

AOCHMMWVGunTruckSpawnPoint::AOCHMMWVGunTruckSpawnPoint()
{
    VehicleClass = AOCHMMWVGunTruck::StaticClass();
    RespawnDelaySeconds = 48.0f;
}

AOCProductionPickupGunTruckSpawnPoint::AOCProductionPickupGunTruckSpawnPoint()
{
    VehicleClass = AOCPickupGunTruck::StaticClass();
    RespawnDelaySeconds = 48.0f;
}

AOCBTRSpawnPoint::AOCBTRSpawnPoint()
{
    VehicleClass = AOCBTR::StaticClass();
    RespawnDelaySeconds = 72.0f;
}
