#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13EnterableHouseArtSubsystem.generated.h"

/** Improves enterable-house exterior art while preserving authored openings, collision and interiors. */
UCLASS()
class OSTERCONFLICT_API UOCR13EnterableHouseArtSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void ApplyEnterableHouseArt(UWorld& World);
};
