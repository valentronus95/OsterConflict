#pragma once

#include "CoreMinimal.h"
#include "OCGameMode.h"
#include "OCGameModeRuntimeSafe.generated.h"

class SWidget;

/**
 * Runtime-facing correction layer for normal Oster play.
 *
 * Keeps the existing OCGameMode feature set while enforcing two contracts that
 * source-only tests previously failed to prove:
 * - a normal local session does not silently create a 16-player filler-bot load;
 * - BASE deployment means the actual human pawn is placed at the Museum BASE.
 *
 * PASS45 also uses this runtime-safe owner as the final loading milestone. The
 * map MoviePlayer surface is stopped only after OCGameMode::BeginPlay returns.
 * The lightweight standalone frontend additionally owns a viewport-level
 * bootstrap overlay so a failed/late R13 menu can never degrade to a silent
 * full-screen black frame.
 */
UCLASS()
class OSTERCONFLICT_API AOCGameModeRuntimeSafe : public AOCGameMode
{
    GENERATED_BODY()

public:
    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void RestartPlayer(AController* NewPlayer) override;

private:
    void ShowFrontendBootstrapOverlay();
    void PollFrontendBootstrapReady();
    void RemoveFrontendBootstrapOverlay(const TCHAR* Reason);

    TSharedPtr<SWidget> FrontendBootstrapOverlay;
    FTimerHandle FrontendBootstrapPollHandle;
    double FrontendBootstrapStartedAtSeconds = 0.0;
    bool bFrontendBootstrapDelayLogged = false;
};
