#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "OCVehicleExitInputRecoverySubsystem.generated.h"

class APawn;

/**
 * Runtime guard for character possession input recovery.
 *
 * Vehicle input uses a high-priority Enhanced Input mapping context, while deployment/front-end
 * transitions can also leave an ignore-move/look stack behind during possession. This subsystem
 * observes the local pawn and restores the normal controller + character mapping stack once an
 * AOCCharacter is possessed and all intentional UI input locks have been released.
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
    FTimerHandle PossessionPollTimer;
    TWeakObjectPtr<APawn> LastLocalPawn;
    TWeakObjectPtr<APawn> LastRecoveredCharacterPawn;
    bool bLastPawnWasVehicle = false;
    bool bPollBudgetLogged = false;

    void ScheduleNextPoll(float DelaySeconds);
    void PollLocalPossession();
    void RestoreCharacterInput(class AOCPlayerController& PlayerController);
};
