#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "OCDenseGroundFoliageSubsystem.generated.h"

/** Dense, collision-aware grass coverage for the playable Oster runtime map. */
UCLASS()
class OSTERCONFLICT_API UOCDenseGroundFoliageSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void TryPopulateWhenGameplayReady();
    void Populate(UWorld& World);

    FTimerHandle GameplayReadyTimer;
    bool bPopulated = false;
};
