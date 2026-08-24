#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCPerformanceSampleSubsystem.generated.h"

/**
 * Client-side runtime performance evidence.
 * After possession it performs a short probe and a settled final sample. Low FPS is recorded as
 * evidence only; Pass 39 no longer destroys visual quality mid-session to disguise the real bottleneck.
 */
UCLASS()
class OSTERCONFLICT_API UOCPerformanceSampleSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override { return !bFinished; }

private:
    float WarmupSeconds = 0.0f;
    float ProbeSeconds = 0.0f;
    float ProbeFrameSeconds = 0.0f;
    int32 ProbeFrames = 0;
    float SampleSeconds = 0.0f;
    float AccumulatedFrameSeconds = 0.0f;
    float WorstFrameSeconds = 0.0f;
    int32 SampleFrames = 0;
    bool bProbeComplete = false;
    bool bFinished = false;

    void ReportLowFpsProbe(float ProbeFps) const;
};