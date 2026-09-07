#include "OCRespawnRecoveryValidationSubsystem.h"

#include "OCCharacter.h"
#include "OCGameMode.h"
#include "OCHealthComponent.h"
#include "OCPlayerController.h"
#include "OCPlayerState.h"

#include "Engine/World.h"

bool UOCRespawnRecoveryValidationSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

TStatId UOCRespawnRecoveryValidationSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCRespawnRecoveryValidationSubsystem, STATGROUP_Tickables);
}

void UOCRespawnRecoveryValidationSubsystem::Tick(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World || World->GetNetMode() == NM_Client) return;

    if (const AOCGameMode* GameMode = World->GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    PollAccumulator += DeltaTime;
    if (PollAccumulator < PollIntervalSeconds) return;
    PollAccumulator = 0.0f;

    ValidateRespawns();
}

void UOCRespawnRecoveryValidationSubsystem::ValidateRespawns()
{
    UWorld* World = GetWorld();
    if (!World) return;

    for (auto It = LastDeathsByController.CreateIterator(); It; ++It)
    {
        if (!It.Key().IsValid()) It.RemoveCurrent();
    }
    for (auto It = PendingRespawnStartByController.CreateIterator(); It; ++It)
    {
        if (!It.Key().IsValid()) It.RemoveCurrent();
    }

    const double Now = FPlatformTime::Seconds();
    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        AOCPlayerController* Controller = Cast<AOCPlayerController>(It->Get());
        if (!Controller || Controller->IsActorBeingDestroyed()) continue;

        AOCPlayerState* PlayerState = Controller->GetPlayerState<AOCPlayerState>();
        if (!PlayerState || PlayerState->IsBotPlayer()) continue;

        const TWeakObjectPtr<AOCPlayerController> ControllerKey(Controller);
        const int32 CurrentDeaths = PlayerState->GetDeaths();
        int32* PreviousDeaths = LastDeathsByController.Find(ControllerKey);
        if (!PreviousDeaths)
        {
            LastDeathsByController.Add(ControllerKey, CurrentDeaths);
            continue;
        }

        if (CurrentDeaths < *PreviousDeaths)
        {
            // Round statistics were reset. A reset is not a new death/respawn cycle.
            *PreviousDeaths = CurrentDeaths;
            PendingRespawnStartByController.Remove(ControllerKey);
            continue;
        }

        if (CurrentDeaths > *PreviousDeaths)
        {
            *PreviousDeaths = CurrentDeaths;
            PendingRespawnStartByController.Add(ControllerKey, Now);
            UE_LOG(LogTemp, Display,
                TEXT("GAME_RECOVERY_RESPAWN_WATCH_ARMED deaths=%d expected_delay_s=10.00 second_respawn_owner=0 validation_only=1"),
                CurrentDeaths);
        }

        const double* RespawnStart = PendingRespawnStartByController.Find(ControllerKey);
        if (!RespawnStart) continue;

        const double Elapsed = FMath::Max(0.0, Now - *RespawnStart);
        AOCCharacter* Character = Cast<AOCCharacter>(Controller->GetPawn());
        const bool bAliveCharacter = Character && Character->GetHealthComponent() && Character->GetHealthComponent()->IsAlive();

        if (bAliveCharacter && Elapsed >= ExpectedRespawnDelaySeconds - EarlyReadyToleranceSeconds)
        {
            if (Elapsed <= ExpectedRespawnDelaySeconds + LateFailureToleranceSeconds)
            {
                UE_LOG(LogTemp, Display,
                    TEXT("GAME_RECOVERY_RESPAWN_READY deaths=%d elapsed_s=%.2f expected_s=10.00 tolerance_s=1.00 pawn=1 alive=1 spectator_stuck=0 input_recovery_check=runtime"),
                    CurrentDeaths,
                    Elapsed);
            }
            else
            {
                UE_LOG(LogTemp, Error,
                    TEXT("GAME_RECOVERY_RESPAWN_FAIL reason=late_respawn deaths=%d elapsed_s=%.2f expected_s=10.00 tolerance_s=1.00 pawn=1 alive=1"),
                    CurrentDeaths,
                    Elapsed);
            }
            PendingRespawnStartByController.Remove(ControllerKey);
            continue;
        }

        if (Elapsed > ExpectedRespawnDelaySeconds + LateFailureToleranceSeconds)
        {
            UE_LOG(LogTemp, Error,
                TEXT("GAME_RECOVERY_RESPAWN_FAIL reason=no_live_character deaths=%d elapsed_s=%.2f expected_s=10.00 tolerance_s=1.00 pawn=%d alive=%d spectator_stuck=1"),
                CurrentDeaths,
                Elapsed,
                Character ? 1 : 0,
                bAliveCharacter ? 1 : 0);
            PendingRespawnStartByController.Remove(ControllerKey);
        }
    }
}
