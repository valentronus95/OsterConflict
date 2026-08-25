#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "OCMuseumLayerPerformanceGuardSubsystem.generated.h"

/**
 * Pass45 Museum ownership validation.
 *
 * Historical versions repaired/hid Museum layers after startup. That behavior is forbidden now:
 * R13.7 is the single visible exterior owner, R13.8 owns hidden interaction collision, and the
 * primary authoring stages must remove source overlap themselves. This subsystem observes once and
 * reports failure without changing visibility, collision, transforms, instances, materials or cull state.
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
    FTimerHandle ValidationTimer;
    void RunValidation();
};
