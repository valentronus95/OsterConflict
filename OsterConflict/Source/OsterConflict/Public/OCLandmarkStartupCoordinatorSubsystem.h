#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "OCLandmarkStartupCoordinatorSubsystem.generated.h"

class UWorld;

/**
 * Owns landmark startup ordering without monopolising the game thread during frontend/deployment.
 *
 * Historical Museum/Silpo/Culture stages still own their geometry and gameplay. Their old delayed
 * timers are cancelled once, then the authoritative stages are released only after deployment and
 * advanced one stage per frame. The R13.7 museum exterior on a playable client is owned by the
 * deployment-stability async preload path, so the coordinator waits for it instead of synchronously
 * loading the same packages on the UI frame.
 */
UCLASS()
class OSTERCONFLICT_API UOCLandmarkStartupCoordinatorSubsystem final : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

private:
    static constexpr float DeferredStartupRetrySeconds = 0.25f;

    void RunAuthoritativeStartup(UWorld& World);
    void CancelHistoricalStageTimers(UWorld& World);
    void ScheduleStartupStep(UWorld& World, float DelaySeconds);
    bool IsBlockingPreGameUI(UWorld& World) const;
    bool IsMuseumExteriorReady(UWorld& World) const;
    bool RunNextStartupStage(UWorld& World);

    FTimerHandle StartupStepTimerHandle;
    int32 StartupStageIndex = 0;
    bool bHistoricalTimersCancelled = false;
    bool bStartupComplete = false;
    bool bDeferredLogWritten = false;
};
