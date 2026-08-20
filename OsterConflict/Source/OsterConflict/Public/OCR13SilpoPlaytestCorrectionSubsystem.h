#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13SilpoPlaytestCorrectionSubsystem.generated.h"

/** Late visual cleanup for the photo-driven Silpo site after all R13 detail/decorator passes have run. */
UCLASS()
class OSTERCONFLICT_API UOCR13SilpoPlaytestCorrectionSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void ApplyCorrection(UWorld& World);
};