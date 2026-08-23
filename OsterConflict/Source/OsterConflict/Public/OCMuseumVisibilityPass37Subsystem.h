#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "OCMuseumVisibilityPass37Subsystem.generated.h"

/**
 * Pass 37/38 visual-presence guard for the Museum.
 *
 * Pass 37 proved actor tags are not enough, but its recovery loop could repeatedly destroy and rebuild
 * the full R13.8 museum while waiting for visible-component evidence. The 2026-08-24 runtime showed the
 * exact failure signature: initially playable FPS followed by a rapid collapse to single digits while the
 * museum still never became visible. Pass 38 therefore makes recovery fail-closed and strictly bounded:
 * at most one architecture rebuild is allowed. After that this subsystem observes only and reports failure
 * instead of churning actors/components/materials every few hundred milliseconds.
 */
UCLASS()
class OSTERCONFLICT_API UOCMuseumVisibilityPass37Subsystem final : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

private:
    FTimerHandle VisibilityPollTimer;
    int32 PollCount = 0;
    int32 RebuildAttemptCount = 0;
    float ElapsedPollSeconds = 0.0f;

    void ValidateVisibleMuseum();
};