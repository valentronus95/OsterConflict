#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCPerformanceSampleSubsystem.generated.h"

/**
 * Client-side runtime performance evidence and emergency recovery.
 * After possession it performs a short probe. If the real runtime is below 20 FPS,
 * it applies a temporary low-cost playtest profile before collecting the final sample.
 */
UCLASS()
class OSTERCONFLICT_API UOCPerformanceSampleSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;

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
    bool bEmergencyProfileApplied = false;
    bool bFinished = false;

    void ApplyEmergencyPlaytestProfile(float ProbeFps);
};
