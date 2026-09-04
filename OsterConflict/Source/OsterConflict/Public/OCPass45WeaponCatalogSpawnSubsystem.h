#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "OCPass45WeaponCatalogSpawnSubsystem.generated.h"

/**
 * Sandbox-only owner for the complete weapon rack.
 * It spawns one pickup for every distinct gameplay weapon identity and skips identities already present.
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
    void EnsureCompleteWeaponRack();
    FTimerHandle SpawnTimer;
};
