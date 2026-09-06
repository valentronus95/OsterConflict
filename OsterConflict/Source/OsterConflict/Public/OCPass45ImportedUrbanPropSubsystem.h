#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "OCPass45ImportedUrbanPropSubsystem.generated.h"

/** Adds a small authored street-prop layer from already-imported packs without duplicating world ownership. */
UCLASS()
class OSTERCONFLICT_API UOCPass45ImportedUrbanPropSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

private:
    void TryBuildWhenGameplayReady();

    FTimerHandle GameplayReadyTimer;
    bool bBuildFinished = false;
};