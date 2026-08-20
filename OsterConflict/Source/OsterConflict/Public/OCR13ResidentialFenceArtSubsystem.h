#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13ResidentialFenceArtSubsystem.generated.h"

/** Replaces visible private-sector fence cubes with bundled AdvancedVillagePack fence meshes. */
UCLASS()
class OSTERCONFLICT_API UOCR13ResidentialFenceArtSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void ApplyFenceArt(UWorld& World);
};
