#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13MuseumStadiumPhotoFidelitySubsystem.generated.h"

/**
 * Retired R13.6 compatibility shim.
 *
 * This subsystem intentionally owns no museum or stadium presentation. Stadion Oster is owned exclusively by
 * UOCR13StadiumSurfaceSubsystem so no delayed legacy pass can rebuild or overlap the site after BeginPlay.
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
};