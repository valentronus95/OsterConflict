#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13CentralParkDressingSubsystem.generated.h"

/** Visual-only planting layer for the verified central city park anchor. */
UCLASS()
class OSTERCONFLICT_API UOCR13CentralParkDressingSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void ApplyCentralParkDressing(UWorld& World);
    bool bApplied = false;
};
