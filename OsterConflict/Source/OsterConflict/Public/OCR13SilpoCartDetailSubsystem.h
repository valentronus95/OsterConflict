#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13SilpoCartDetailSubsystem.generated.h"

/**
 * Visual-only shopping-cart detail for the Oster Silpo photo reconstruction.
 * Built procedurally because the bundled prop packs do not contain a supermarket trolley.
 */
UCLASS()
class OSTERCONFLICT_API UOCR13SilpoCartDetailSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void ApplyCartDetail(UWorld& World);
};
