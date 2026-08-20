#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13MuseumFacadeEvidenceSubsystem.generated.h"

/**
 * Evidence-driven micro-detail pass for the Solonyna/Oster museum facade.
 * Replaces coarse temporary window bars with photo-matched mullions and diamond security grilles.
 */
UCLASS()
class OSTERCONFLICT_API UOCR13MuseumFacadeEvidenceSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void BuildFacadeEvidence(UWorld& World);
};
