#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "OCPass45ImportedWeaponBridgeSubsystem.generated.h"

/**
 * Bridges already-imported local weapon packs into existing gameplay weapon actors without creating
 * a second weapon system. Exact semantic matches are preferred; unrelated gun models are never silently
 * relabelled as another weapon. Missing local content leaves the existing tracked production/fallback path intact.
 */
UCLASS()
class OSTERCONFLICT_API UOCPass45ImportedWeaponBridgeSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

private:
    FTimerHandle RefreshTimer;
    int32 RefreshPass = 0;

    void RefreshWeapons();
};
