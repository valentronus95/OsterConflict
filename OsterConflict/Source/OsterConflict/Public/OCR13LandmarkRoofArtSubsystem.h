#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13LandmarkRoofArtSubsystem.generated.h"

/** Applies bundled roof art only to the source-authored pitched museum roof panels. */
UCLASS()
class OSTERCONFLICT_API UOCR13LandmarkRoofArtSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void BuildMuseumRoofBridge(UWorld& World);
};
