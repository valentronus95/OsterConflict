#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR140MuseumFacadeDetailSubsystem.generated.h"

/**
 * R14.0 photo-detail pass for the Oster museum exterior.
 * Corrects the photographed service-door gable, adds its upper breakable window,
 * replaces the prototype service door and layers facade-specific trim/utilities.
 */
UCLASS()
class OSTERCONFLICT_API UOCR140MuseumFacadeDetailSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void ApplyFacadeDetail(UWorld& World) const;
};
