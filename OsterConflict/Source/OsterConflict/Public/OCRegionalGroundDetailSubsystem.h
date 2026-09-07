#pragma once

#include "CoreMinimal.h"
#include "Engine/StreamableManager.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCRegionalGroundDetailSubsystem.generated.h"

/**
 * One-shot Block0 ground-detail layer.
 * Adds sparse authored leaf litter only around the existing deciduous tree family.
 * Dense grass remains owned by UOCDenseGroundFoliageSubsystem; this subsystem owns no gameplay collision or tick.
 */
UCLASS()
class OSTERCONFLICT_API UOCRegionalGroundDetailSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

private:
    void RequestRegionalGroundDetailPreload();
    void HandleRegionalGroundDetailPreloadComplete();
    void PopulateRegionalGroundDetail();

    FTimerHandle PopulateTimerHandle;
    TSharedPtr<FStreamableHandle> PreloadHandle;
    bool bPreloadRequested = false;
    bool bPopulationFinished = false;
};
