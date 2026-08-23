#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "OCMuseumLayerPerformanceGuardSubsystem.generated.h"

/**
 * Late museum-site repair/budget guard.
 *
 * The museum has several historical construction layers (source world, R13.7 prototype,
 * R13.8 enterable architecture and later photo-detail passes). This subsystem runs after
 * authoritative landmark startup and guarantees that old source/prototype geometry cannot
 * remain visible/collidable underneath the current museum while also applying a conservative
 * render budget to decorative museum ISMs.
 */
UCLASS()
class OSTERCONFLICT_API UOCMuseumLayerPerformanceGuardSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

private:
    FTimerHandle RepairTimer;
    FTimerHandle ValidationTimer;

    int32 TotalSourceInstancesRemoved = 0;
    int32 TotalObsoletePrototypeComponentsHidden = 0;
    int32 TotalDecorativeComponentsTuned = 0;

    void RunRepairPass();
    void RunFinalValidation();
};
