#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCPerformanceSampleSubsystem.generated.h"

/**
 * Client-side runtime performance evidence.
 * Pass 45 samples both the pawn-less frontend and the settled possessed gameplay window. Low FPS is
 * evidence only; the sampler must never destroy visual quality to disguise the actual bottleneck.
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
    float FrontendWarmupSeconds = 0.0f;
    float FrontendSampleSeconds = 0.0f;
    float FrontendAccumulatedFrameSeconds = 0.0f;
    float FrontendWorstFrameSeconds = 0.0f;
    int32 FrontendFrames = 0;
    bool bFrontendSampleLogged = false;

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