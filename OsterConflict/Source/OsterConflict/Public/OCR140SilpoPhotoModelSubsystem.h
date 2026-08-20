#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR140SilpoPhotoModelSubsystem.generated.h"

/**
 * Silpo-only runtime replacement for the Oster supermarket at Bohdana Khmelnytskoho 54.
 *
 * The site is hard-anchored through FOCGeoReference. Exterior massing and facade rhythm are driven by
 * the dedicated Silpo photo reference set. Phase one is gameplay-ready: a working left-side entrance,
 * empty gondola shelving, basic checkout desks, an empty produce island and neutral store lighting.
 */
UCLASS()
class OSTERCONFLICT_API UOCR140SilpoPhotoModelSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void ReplaceSilpo(UWorld& World);
    void SuppressSourceSite(UWorld& World);
    void BuildSilpo(UWorld& World);
};
