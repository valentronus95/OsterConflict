#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "OCVehicleExitInputRecoverySubsystem.generated.h"

class AOCPlayerController;
class APawn;

/**
 * Event-driven runtime guard for character possession input recovery.
 *
 * Vehicle input uses a high-priority Enhanced Input mapping context, while deployment/front-end
 * transitions can also leave an ignore-move/look stack behind during possession. The subsystem
 * binds to the local controller's pawn-change notifier and only retries while an intentional UI
 * transition is still holding the input lock. Stable gameplay performs no background polling.
 */
UCLASS()
class OSTERCONFLICT_API UOCVehicleExitInputRecoverySubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

private:
    FTimerHandle RecoveryRetryTimer;
    TWeakObjectPtr<AOCPlayerController> BoundLocalController;
    TWeakObjectPtr<APawn> LastRecoveredCharacterPawn;
    FDelegateHandle NewPawnDelegateHandle;
    bool bEventBindingLogged = false;

    void BindLocalControllerOrRetry();
    void HandleLocalPawnChanged(APawn* NewPawn);
    void TryRecoverCurrentPossession();
    void ScheduleRetry(float DelaySeconds);
    void RestoreCharacterInput(AOCPlayerController& PlayerController);
};
