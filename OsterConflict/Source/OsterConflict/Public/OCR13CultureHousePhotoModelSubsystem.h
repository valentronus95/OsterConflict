#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13CultureHousePhotoModelSubsystem.generated.h"

/**
 * Photo/reference-driven Oster City House of Culture, Hranovskoho 3 (former Great Synagogue).
 * Runtime spawning is temporarily disabled because the current playtest proved that the provisional site/yaw and
 * surrounding topology are not trustworthy enough for the player-facing map. Keep the implementation as reference
 * geometry until the culture-house parcel is re-anchored against the corrected Oster topology.
 */
UCLASS(Abstract)
class OSTERCONFLICT_API UOCR13CultureHousePhotoModelSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void BuildCultureHouse(UWorld& World);
};
