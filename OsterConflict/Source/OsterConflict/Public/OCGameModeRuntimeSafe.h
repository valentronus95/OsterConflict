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
 *   BeginPlay, then retires the legacy base-GameMode duplicate before the first gameplay tick.
 */
UCLASS()
class OSTERCONFLICT_API AOCGameModeRuntimeSafe : public AOCGameMode
{
    GENERATED_BODY()

public:
    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
    virtual void BeginPlay() override;
    virtual void RestartPlayer(AController* NewPlayer) override;
};
