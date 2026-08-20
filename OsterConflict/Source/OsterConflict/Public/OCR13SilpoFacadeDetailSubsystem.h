#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13SilpoFacadeDetailSubsystem.generated.h"

/**
 * Late R13 Silpo facade-detail pass.
 *
 * Runs after the photo-driven Silpo shell, replaces the temporary simple logo presentation and adds
 * editable Ukrainian poster/sign text and small facade details from the supplied Oster photo set.
 * No reference bitmap is embedded in runtime content.
 */
UCLASS()
class OSTERCONFLICT_API UOCR13SilpoFacadeDetailSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void ApplyFacadeDetails(UWorld& World);
};
