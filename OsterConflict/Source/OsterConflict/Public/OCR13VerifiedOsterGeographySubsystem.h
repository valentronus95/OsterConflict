#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13VerifiedOsterGeographySubsystem.generated.h"

/**
 * R13.6 map-first cleanup: suppresses the old near-spawn fake Krushelnytska art slice and relocates the stadium
 * presentation from its legacy gameplay approximation to a public-map coordinate relative to the museum origin.
 */
UCLASS()
class OSTERCONFLICT_API UOCR13VerifiedOsterGeographySubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    static FVector VerifiedStadiumAnchor();

private:
    void ApplyVerifiedGeography(UWorld& World);
    void SuppressLegacyNearSpawnSlice(UWorld& World);
    void RemoveLegacySliceResidentialPresentation(UWorld& World);
    void RelocateStadiumPresentation(UWorld& World);
};
