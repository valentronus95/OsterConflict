#pragma once

#include "CoreMinimal.h"
#include "OCGameMode.h"
#include "OCGameModeRuntimeSafe.generated.h"

/**
 * Runtime-facing correction layer for normal Oster play.
 *
 * Keeps the existing OCGameMode feature set while enforcing runtime-safe contracts:
 * - frontend/travel stays light before a human host exists, then the dedicated population policy restores filler bots;
 * - BASE deployment means the actual human pawn is placed at the Museum BASE;
 * - PASS45 startup recovery keeps exactly one lightweight AOCWorldSectorOster alive before world-subsystem
 *   BeginPlay, then retires the legacy base-GameMode duplicate before the first gameplay tick;
 * - GAME_RECOVERY world-preparation state remains factual acceptance evidence, but visual/content preparation
 *   failures never deny the human a pawn or close deployment without possession.
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
    // Compatibility queue retained for already-armed startup requests. New human deployment is fail-soft and does
    // not enter this queue merely because a visual/content acceptance stage is pending or failed.
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
