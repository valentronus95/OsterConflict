#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR146LandmarkSeparationSubsystem.generated.h"

/**
 * Early startup guard that removes generic source-building instances from the canonical Museum, Silpo and
 * Culture House parcels before the authoritative landmark owners are revealed. It never relocates those owners.
 */
UCLASS()
class OSTERCONFLICT_API UOCR146LandmarkSeparationSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void EnforceSeparation(UWorld& World) const;
};
