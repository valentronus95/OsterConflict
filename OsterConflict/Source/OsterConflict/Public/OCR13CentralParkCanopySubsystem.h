#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13CentralParkCanopySubsystem.generated.h"

/**
 * Reference-only central-park canopy.
 * Disabled from R13 runtime until the park is re-anchored against corrected local topology. This also removes one of
 * the delayed visual layers that can visibly pop after the player has already entered the map.
 */
UCLASS(Abstract)
class OSTERCONFLICT_API UOCR13CentralParkCanopySubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    bool bApplied = false;
    void ApplyCentralParkCanopy(UWorld& World);
};
