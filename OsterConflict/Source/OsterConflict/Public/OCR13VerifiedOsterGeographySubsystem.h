#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13VerifiedOsterGeographySubsystem.generated.h"

/**
 * Temporary R13 migration cleanup. Its only remaining job is deleting obsolete near-spawn Krushelnytska
 * presentation components before residential styling runs. Permanent geography now belongs to FOCGeoReference
 * and AOCWorldSectorOster and must not be relocated after BeginPlay.
 */
UCLASS()
class OSTERCONFLICT_API UOCR13VerifiedOsterGeographySubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void SuppressLegacyNearSpawnSlice(UWorld& World);
};
