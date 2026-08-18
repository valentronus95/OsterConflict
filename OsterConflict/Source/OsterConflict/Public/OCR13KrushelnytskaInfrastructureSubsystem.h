#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13KrushelnytskaInfrastructureSubsystem.generated.h"

/** Real utility-pole pass for the dedicated Krushelnytska visual slice excluded from generic roadside dressing. */
UCLASS()
class OSTERCONFLICT_API UOCR13KrushelnytskaInfrastructureSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    bool bApplied = false;
    void ApplyKrushelnytskaInfrastructure(UWorld& World);
};
