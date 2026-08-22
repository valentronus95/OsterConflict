#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCPerformanceSampleSubsystem.generated.h"

/**
 * Lightweight client-side runtime evidence for the performance recovery pass.
 * Waits for an actually possessed gameplay pawn, ignores the frontend/loading period,
 * warms up for five seconds and then records a ten-second frame-time sample.
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
    float SampleSeconds = 0.0f;
    float AccumulatedFrameSeconds = 0.0f;
    float WorstFrameSeconds = 0.0f;
    int32 SampleFrames = 0;
    bool bFinished = false;
};
