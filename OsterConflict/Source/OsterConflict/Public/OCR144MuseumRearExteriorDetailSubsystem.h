#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR144MuseumRearExteriorDetailSubsystem.generated.h"

/**
 * R14.4 long-side/rear detail pass from REF-03/09/11/13/19/20.
 * Adds drainage, service-entry steps and conservative annex edge detail without inventing unseen openings.
 */
UCLASS()
class OSTERCONFLICT_API UOCR144MuseumRearExteriorDetailSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void BuildRearExteriorDetail(UWorld& World) const;
};
