#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13VerifiedOsterGeographySubsystem.generated.h"

/**
 * R13 migration cleanup. Suppresses the obsolete near-spawn Krushelnytska art slice and, until the legacy
 * world-sector stadium placement is migrated, relocates its presentation to the canonical FOCGeoReference anchor.
 * No coordinate is authored in this subsystem.
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
