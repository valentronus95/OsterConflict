#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13CultureHousePhotoModelSubsystem.generated.h"

/**
 * Photo/reference-driven Oster City House of Culture, Hranovskoho 3 (former Great Synagogue).
 * Current facade massing follows public modern photography; exact parcel yaw remains explicit/provisional.
 */
UCLASS()
class OSTERCONFLICT_API UOCR13CultureHousePhotoModelSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void BuildCultureHouse(UWorld& World);
};
