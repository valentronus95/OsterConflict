#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13ResidentialInfillSubsystem.generated.h"

/**
 * Adds a bounded number of collision-backed houses beside authored compact-Oster roads.
 * Candidates are rejected near existing buildings, landmarks and the dedicated Krushelnytska slice.
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
