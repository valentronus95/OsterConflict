#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "OCMuseumVisibilityPass37Subsystem.generated.h"

/**
 * Pass 37 visual-presence guard for the Museum.
 *
 * Earlier acceptance counted R13.7/R13.8 owner actors. The latest playtest proved that an owner tag
 * is not evidence that an actual visible building exists. This guard validates registered, visible
 * MuseumStructural components near MuseumAnchor, rebuilds a stale/empty R13.8 owner, and retires
 * duplicate late R13.8 owners through the end of the normal delayed startup window.
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
    float ElapsedPollSeconds = 0.0f;
    bool bRebuildAttempted = false;

    void ValidateVisibleMuseum();
};
