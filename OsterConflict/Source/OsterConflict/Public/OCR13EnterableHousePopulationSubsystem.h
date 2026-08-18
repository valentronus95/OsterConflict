#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13EnterableHousePopulationSubsystem.generated.h"

/** Adds a few additional gameplay-enterable houses into clear residential roadside gaps. */
UCLASS()
class OSTERCONFLICT_API UOCR13EnterableHousePopulationSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void PopulateEnterableHouses(UWorld& World);
    bool bApplied = false;
};
