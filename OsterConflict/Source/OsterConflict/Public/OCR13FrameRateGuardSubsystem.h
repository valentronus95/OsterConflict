#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13FrameRateGuardSubsystem.generated.h"

/** R13 playtest thermal guard: 45 FPS in pregame frontend, 60 FPS in gameplay, without mutating saved settings. */
UCLASS()
class OSTERCONFLICT_API UOCR13FrameRateGuardSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual void Deinitialize() override;

private:
    void ApplyCap(float NewCap);

    float ActiveCap = -1.0f;
    float PreviousEngineCap = 0.0f;
    bool bCapturedPreviousCap = false;
};
