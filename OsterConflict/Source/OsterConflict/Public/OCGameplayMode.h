#pragma once

#include "CoreMinimal.h"
#include "OCGameplayMode.generated.h"

UENUM(BlueprintType)
enum class EOCGameplayMode : uint8
{
    Conquest UMETA(DisplayName="Conquest"),
    Sandbox UMETA(DisplayName="Sandbox / Test Range")
};

UENUM(BlueprintType)
enum class EOCSandboxAdminAction : uint8
{
    SpawnWeaponRack,
    RefillAmmo,
    RestorePlayer,
    SpawnCivilianVehicle,
    SpawnGunTruck,
    SpawnBTR,
    ToggleGodMode,
    ResetInteractables,
    TeleportMuseum,
    TeleportStadium,
    TeleportPark,
    TeleportCollege,
    SpawnFourBots,
    ClearBots
};
