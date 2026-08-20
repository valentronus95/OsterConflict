#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13MuseumRearTerrainSubsystem.generated.h"

/**
 * Replaces the single flat collision floor around the museum with a deterministic local valley:
 * museum plateau -> rear descent -> lower residential ground -> return to the city datum.
 * Exact elevation is gameplay-topology data, not survey data.
 */
UCLASS()
class OSTERCONFLICT_API UOCR13MuseumRearTerrainSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void BuildTerrainSurface(UWorld& World);
    void FinalizeLowerResidentialDistrict(UWorld& World);
};
