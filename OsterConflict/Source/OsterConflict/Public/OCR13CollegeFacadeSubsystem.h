#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13CollegeFacadeSubsystem.generated.h"

/**
 * Reference-only facade dressing for the Oster college.
 * Disabled from player-facing R13 runtime until the college parcel is re-anchored against the corrected museum / park
 * topology. Keeping decorative detail on a known-wrong site only makes the geography error harder to see and debug.
 */
UCLASS(Abstract)
class OSTERCONFLICT_API UOCR13CollegeFacadeSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    bool bApplied = false;
    void ApplyCollegeFacade(UWorld& World);
};
