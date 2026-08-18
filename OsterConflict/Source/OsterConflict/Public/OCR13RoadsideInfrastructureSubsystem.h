#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13RoadsideInfrastructureSubsystem.generated.h"

/**
 * R13.5 residential/civic roadside infrastructure pass.
 * Adds visual-only utility poles and restrained pole attachments along authored road strips.
 * Exact pole/cable alignment can be refined from location photos without changing road or navigation geometry.
 */
UCLASS()
class OSTERCONFLICT_API UOCR13RoadsideInfrastructureSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void ApplyRoadsideInfrastructure(UWorld& World);

    bool bApplied = false;
};
