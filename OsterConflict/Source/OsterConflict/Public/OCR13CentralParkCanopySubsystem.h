#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13CentralParkCanopySubsystem.generated.h"

/** Adds a mature visual tree canopy around the central park while keeping memorial, alleys and skate pad clear. */
UCLASS()
class OSTERCONFLICT_API UOCR13CentralParkCanopySubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    bool bApplied = false;
    void ApplyCentralParkCanopy(UWorld& World);
};
