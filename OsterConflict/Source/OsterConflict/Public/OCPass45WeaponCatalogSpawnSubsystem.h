#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "OCPass45WeaponCatalogSpawnSubsystem.generated.h"

/**
 * Sandbox-only completion owner for the existing "Spawn all weapons" admin rack.
 * It does not create a second automatic rack on map load. Instead it detects the compact core rack
 * spawned by the admin action and adds only the missing distinct gameplay weapon identities.
 */
UCLASS()
class OSTERCONFLICT_API UOCPass45WeaponCatalogSpawnSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

private:
    void CompleteRequestedWeaponRack();
    FTimerHandle SpawnTimer;
    int32 ScanPass = 0;
    bool bRackCompleted = false;
};
