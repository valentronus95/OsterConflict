#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13SilpoPhotoModelSubsystem.generated.h"

/**
 * Photo-proportioned exterior replacement for the Oster Silpo site.
 *
 * The public map anchor is the Silpo point at 50.94907, 30.87621 (Bohdana Khmelnytskoho 54).
 * Architectural dimensions are inferred from the supplied multi-angle exterior photographs and are
 * intentionally treated as visual proportions rather than survey measurements.
 *
 * This subsystem removes source placeholder building instances only inside the store footprint,
 * then builds the replacement deterministically on every gameplay world (server and clients).
 */
UCLASS()
class OSTERCONFLICT_API UOCR13SilpoPhotoModelSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void ReplaceSilpo(UWorld& World);
    void SuppressSourceBuilding(UWorld& World);
    void BuildSilpo(UWorld& World);
};
