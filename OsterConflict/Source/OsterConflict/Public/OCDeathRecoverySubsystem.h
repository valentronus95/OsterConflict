#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCDeathRecoverySubsystem.generated.h"

class AOCPlayerController;

/**
 * GAME RECOVERY safety net for the local death -> respawn transition.
 * The normal GameMode timer remains authoritative at 10 seconds. This guard only intervenes when the
 * local player previously had a gameplay character and is still stuck in spectator/no-character state
 * after that deadline, then restores clean gameplay input after possession returns.
 */
UCLASS()
class OSTERCONFLICT_API UOCDeathRecoverySubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickableWhenPaused() const override { return true; }

private:
    void BeginDeathWindow(AOCPlayerController& Controller);
    void RecoverRespawn(AOCPlayerController& Controller);
    void RestoreGameplayInput(AOCPlayerController& Controller);

    TWeakObjectPtr<AOCPlayerController> TrackedController;
    double DeathStartWallTimeSeconds = 0.0;
    double LastFallbackAttemptWallTimeSeconds = 0.0;
    int32 FallbackAttempts = 0;
    bool bHadGameplayCharacter = false;
    bool bDeathWindowActive = false;
};
