#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR137MuseumRuntimeValidationSubsystem.generated.h"

/**
 * Runtime sanity check for the final R13.7 museum replacement.
 * Runs after the final photo-model pass and reports duplicate/missing final models,
 * visible legacy museum layers, leftover source proxy geometry, competing decorator vegetation
 * and basic collision/instance health.
 */
UCLASS()
class OSTERCONFLICT_API UOCR137MuseumRuntimeValidationSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void ValidateMuseumReplacement(UWorld& World);
};
