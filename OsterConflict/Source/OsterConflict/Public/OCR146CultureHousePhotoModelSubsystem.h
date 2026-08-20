#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR146CultureHousePhotoModelSubsystem.generated.h"

/**
 * Authoritative current-main presentation owner for the Oster City House of Culture.
 * The building is anchored only to FOCGeoReference::CultureHouse() (Hranovskoho 3) and never to Museum/Silpo.
 */
UCLASS()
class OSTERCONFLICT_API UOCR146CultureHousePhotoModelSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void BuildCultureHouse(UWorld& World) const;
};
