#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "OCVehicleExitInputRecoverySubsystem.generated.h"

class APawn;

/**
 * Runtime guard for the vehicle -> character possession transition.
 *
 * Vehicle input uses a high-priority Enhanced Input mapping context. If that context survives
 * possession, WASD/mouse can remain captured by actions that no longer have a vehicle pawn to
 * handle them. This subsystem observes the local possession transition and rebuilds the normal
 * controller + character mapping stack once the player returns to an AOCCharacter.
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
    bool bLastPawnWasVehicle = false;

    void PollLocalPossession();
    void RestoreCharacterInput(class AOCPlayerController& PlayerController);
};
