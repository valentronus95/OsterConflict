#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCRecoveredWeaponVariantSubsystem.generated.h"

/**
 * Development/playtest weapon rack. In non-shipping builds it exposes every currently implemented firearm beside
 * the player's actual deployed spawn so visual, audio, pickup, ADS and reload behavior can be tested in one place.
 */
UCLASS()
class OSTERCONFLICT_API UOCRecoveredWeaponVariantSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void TrySpawnTestRack();
    void ScheduleSpawnAttempt(float DelaySeconds);

    int32 SpawnAttemptCount = 0;
    bool bSpawnComplete = false;
};
