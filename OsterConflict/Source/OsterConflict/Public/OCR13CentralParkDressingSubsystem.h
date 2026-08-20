#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13CentralParkDressingSubsystem.generated.h"

/**
 * Reference-only planting layer for the central city park.
 * Disabled from R13 runtime while the park site is being corrected; otherwise a wrong anchor gets dressed until it
 * looks intentional, which is precisely the opposite of useful debugging.
 */
UCLASS(Abstract)
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
