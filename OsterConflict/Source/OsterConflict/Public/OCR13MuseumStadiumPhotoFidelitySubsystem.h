#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13MuseumStadiumPhotoFidelitySubsystem.generated.h"

/**
 * Final R13.6 presentation owner for the Oster museum + adjacent stadium zone.
 * It removes legacy/source presentation only inside the photographed site and rebuilds the area from the supplied
 * museum/stadium photo set while preserving the wider Oster geography and gameplay systems.
 */
UCLASS()
class OSTERCONFLICT_API UOCR13MuseumStadiumPhotoFidelitySubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void ApplyPhotoFidelity(UWorld& World);
    void SuppressLegacyMuseumPresentation(UWorld& World);
    void SuppressLegacyStadiumPresentation(UWorld& World);
    void BuildMuseum(UWorld& World);
    void BuildStadium(UWorld& World);
};
