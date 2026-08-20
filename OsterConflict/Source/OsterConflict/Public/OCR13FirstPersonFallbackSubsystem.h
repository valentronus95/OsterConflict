#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13FirstPersonFallbackSubsystem.generated.h"

/** Keeps the existing source-only first-person arm/hand bridge visible until authored FP arms are available. */
UCLASS()
class OSTERCONFLICT_API UOCR13FirstPersonFallbackSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;

private:
    float ScanAccumulator = 0.0f;
};