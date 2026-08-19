#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13SilpoEnvelopeDetailSubsystem.generated.h"

/**
 * Late visual refinement for the Oster Silpo exterior envelope.
 * Adds coping/fascia/base details only to geometry already established by the supplied exterior references.
 */
UCLASS()
class OSTERCONFLICT_API UOCR13SilpoEnvelopeDetailSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void ApplyEnvelopeDetails(UWorld& World);
};
