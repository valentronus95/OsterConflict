#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR143MuseumSiteVegetationSubsystem.generated.h"

class UWorld;

/** Adds photo-oriented low vegetation around the museum while preserving the clear central slab approach. */
UCLASS()
class OSTERCONFLICT_API UOCR143MuseumSiteVegetationSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    void RunAuthoritativeDetailNow(UWorld& World) { BuildSiteVegetation(World); }

private:
    void BuildSiteVegetation(UWorld& World) const;
};