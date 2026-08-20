#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR140SilpoPhotoModelSubsystem.generated.h"

/**
 * Silpo-only runtime replacement for the Oster supermarket at Bohdana Khmelnytskoho 54.
 *
 * The site is hard-anchored through FOCGeoReference. Exterior massing and facade rhythm are driven by
 * the dedicated Silpo reference set; the first interior pass is intentionally sparse and gameplay-ready:
 * working entrance doors, empty gondola shelving, basic checkout desks and neutral store lighting.
 *
 * No museum/stadium geometry is authored or removed by this subsystem.
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
