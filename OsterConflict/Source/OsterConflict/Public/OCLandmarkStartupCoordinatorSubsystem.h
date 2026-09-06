#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCLandmarkStartupCoordinatorSubsystem.generated.h"

class UWorld;

/**
 * Owns landmark startup ordering without monopolising the game thread.
 *
 * Historical Museum/Silpo/Culture timers are cancelled once. The coordinator then advances the
 * authoritative stages in small wall-clock-spaced steps while the deployment UI is still visible.
 * It is deliberately tickable while the world is paused so pre-game preparation finishes before
 * possession instead of materialising the city after the player has already spawned.
 */
UCLASS()
class OSTERCONFLICT_API UOCLandmarkStartupCoordinatorSubsystem final : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual void Deinitialize() override;
    virtual bool IsTickable() const override { return true; }
    virtual bool IsTickableWhenPaused() const override { return true; }

    bool IsWorldStartupReady() const { return bStartupComplete; }
    float GetStartupProgress() const;

private:
    static constexpr double StageIntervalSeconds = 0.05;
    static constexpr int32 TotalStartupStages = 13;

    void RunAuthoritativeStartup(UWorld& World);
    void CancelHistoricalStageTimers(UWorld& World);
    bool IsMuseumExteriorReady(UWorld& World) const;
    bool RunNextStartupStage(UWorld& World);

    double NextStageWallTimeSeconds = 0.0;
    int32 StartupStageIndex = 0;
    bool bInitialized = false;
    bool bHistoricalTimersCancelled = false;
    bool bStartupComplete = false;
};
