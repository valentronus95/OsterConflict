#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCRecoveredBuildingDetailsSubsystem.generated.h"

/** Completes the restored unfinished-building showcase with its matching modular pieces. */
UCLASS()
class OSTERCONFLICT_API UOCRecoveredBuildingDetailsSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void Populate(UWorld& World);
    bool bPopulated = false;
};
