#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13CityCouncilPhotoModelSubsystem.generated.h"

/**
 * Photo/reference-driven current Oster City Council at Nezalezhnosti 21.
 * Facade massing follows public Wikimedia references; parcel yaw remains explicit/provisional until surveyed.
 */
UCLASS()
class OSTERCONFLICT_API UOCR13CityCouncilPhotoModelSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void BuildCityCouncil(UWorld& World);
};
