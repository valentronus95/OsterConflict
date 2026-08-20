#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13OsterResidentialArchitectureSubsystem.generated.h"

/**
 * Replaces the visible generic village-prefab houses with a restrained Oster residential language:
 * low brick homes, dark plinths, grey pitched roofs, simple windows/doors and small porches.
 * Hidden legacy house ISMs remain authoritative for collision and placement metadata.
 */
UCLASS()
class OSTERCONFLICT_API UOCR13OsterResidentialArchitectureSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void ApplyOsterResidentialArchitecture(UWorld& World);
};
