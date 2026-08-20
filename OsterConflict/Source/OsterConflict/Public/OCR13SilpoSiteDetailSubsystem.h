#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13SilpoSiteDetailSubsystem.generated.h"

/**
 * Late photo-driven site-detail pass for the Oster Silpo reconstruction.
 *
 * Runs after the base shell and facade/signage pass. It adds small photographed details that make the
 * location read as the real store without replacing roads or inventing an interior: entrance hardware,
 * poster mounting rails/copy, facade seams, the blue entrance bin, and the narrow planted strip.
 */
UCLASS()
class OSTERCONFLICT_API UOCR13SilpoSiteDetailSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void ApplySiteDetails(UWorld& World);
};
