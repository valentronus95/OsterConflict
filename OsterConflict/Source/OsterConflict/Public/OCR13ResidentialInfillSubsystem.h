#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13ResidentialInfillSubsystem.generated.h"

/**
 * Migration stub retained while R13 moves from procedural roadside housing to explicit,
 * reference-driven Oster street/block placement. Procedural infill is intentionally disabled.
 */
UCLASS()
class OSTERCONFLICT_API UOCR13ResidentialInfillSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    bool bApplied = false;
    void BuildResidentialInfill(UWorld& World);
};
