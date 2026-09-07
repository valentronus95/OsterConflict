#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCRespawnRecoveryValidationSubsystem.generated.h"

class AOCPlayerController;

/**
 * GAME_RECOVERY death/respawn acceptance probe.
 *
 * The game rule itself remains owned by AOCGameMode. This subsystem only watches factual human-player
 * death-count transitions and proves whether a fresh AOCCharacter is possessed around the fixed 10 s deadline.
 * It never spawns, teleports or possesses a pawn, so it cannot create a second respawn source of truth.
 */
UCLASS()
class OSTERCONFLICT_API UOCRespawnRecoveryValidationSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override { return true; }

private:
    static constexpr float PollIntervalSeconds = 0.10f;
    static constexpr double ExpectedRespawnDelaySeconds = 10.0;
    static constexpr double EarlyReadyToleranceSeconds = 0.50;
    static constexpr double LateFailureToleranceSeconds = 1.00;

    float PollAccumulator = 0.0f;
    TMap<TWeakObjectPtr<AOCPlayerController>, int32> LastDeathsByController;
    TMap<TWeakObjectPtr<AOCPlayerController>, double> PendingRespawnStartByController;

    void ValidateRespawns();
};
