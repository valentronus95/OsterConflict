#pragma once

#include "CoreMinimal.h"
#include "OCGameMode.h"
#include "OCGameModeRuntimeSafe.generated.h"

/**
 * Runtime-facing correction layer for normal Oster play.
 *
 * Keeps the existing OCGameMode feature set while enforcing runtime-safe contracts:
 * - a normal local session does not silently create a 16-player filler-bot load;
 * - BASE deployment means the actual human pawn is placed at the Museum BASE;
 * - PASS45 startup recovery keeps exactly one lightweight AOCWorldSectorOster alive before world-subsystem
 *   BeginPlay, then retires the legacy base-GameMode duplicate before the first gameplay tick;
 * - GAME_RECOVERY never uses the player pawn as a loading screen: initial human spawn waits for factual
 *   Block0 ground, authored world-surface and landmark readiness on non-dedicated Oster runtime worlds.
 */
UCLASS()
class OSTERCONFLICT_API AOCGameModeRuntimeSafe : public AOCGameMode
{
    GENERATED_BODY()

public:
    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
    virtual void BeginPlay() override;
    virtual void RestartPlayer(AController* NewPlayer) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    struct FPendingWorldReadyRestart
    {
        FTimerHandle TimerHandle;
        double StartWallTimeSeconds = 0.0;
    };

    static constexpr float WorldReadyRestartPollSeconds = 0.10f;
    static constexpr double WorldReadyRestartTimeoutSeconds = 60.0;

    bool IsRecoveryWorldReady(FString& OutPendingStages, bool& bOutHardFailure) const;
    void QueueRestartWhenWorldReady(AController* NewPlayer, const FString& PendingStages);
    void PollRestartWhenWorldReady(TWeakObjectPtr<AController> WeakController);
    void ClearPendingWorldReadyRestart(const TWeakObjectPtr<AController>& WeakController);

    TMap<TWeakObjectPtr<AController>, FPendingWorldReadyRestart> PendingWorldReadyRestarts;
};
