#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13ResidentialInfillFenceSubsystem.generated.h"

/** Adds bounded private-yard fence art/collision only around R13 road-derived infill houses. */
UCLASS()
class OSTERCONFLICT_API UOCR13ResidentialInfillFenceSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void BuildInfillFences(UWorld& World);
    bool bApplied = false;
};
