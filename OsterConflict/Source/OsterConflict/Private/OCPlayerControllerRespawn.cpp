#include "OCPlayerController.h"

#include "OCCharacter.h"
#include "OCHealthComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerInput.h"

namespace
{
    constexpr float GameRecoveryRespawnDelaySeconds = 10.0f;
}

void AOCPlayerController::PawnPendingDestroy(APawn* InPawn)
{
    const AOCCharacter* DeadCharacter = Cast<AOCCharacter>(InPawn);
    const UOCHealthComponent* Health = DeadCharacter ? DeadCharacter->GetHealthComponent() : nullptr;
    const bool bConfirmedGameplayDeath = Health && Health->IsDead();

    Super::PawnPendingDestroy(InPawn);

    if (!HasAuthority() || !bConfirmedGameplayDeath)
    {
        return;
    }

    bServerAwaitingRespawnPossession = true;
    ClientBeginRespawnWait(InPawn, GameRecoveryRespawnDelaySeconds);

    UE_LOG(LogTemp, Display,
        TEXT("GAME_RECOVERY_RESPAWN_WAIT_BEGIN controller=%s delay_seconds=10.0 server_timer_owner=GameMode spectator_pawn=0 gameplay_debugger=0 corpse_view=1"),
        *GetNameSafe(this));
}

void AOCPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (!HasAuthority() || !bServerAwaitingRespawnPossession || !IsValid(InPawn))
    {
        return;
    }

    bServerAwaitingRespawnPossession = false;
    ClientFinishRespawnWait(InPawn);

    UE_LOG(LogTemp, Display,
        TEXT("GAME_RECOVERY_RESPAWN_POSSESSION_RESTORED controller=%s pawn=%s possession_restored=1 spectator_pawn=0 gameplay_debugger=0"),
        *GetNameSafe(this), *GetNameSafe(InPawn));
}

void AOCPlayerController::ClientBeginRespawnWait_Implementation(AActor* DeathViewTarget, float DelaySeconds)
{
    if (!IsLocalController())
    {
        return;
    }

    bRespawnWaiting = true;
    RespawnWaitDeadlineSeconds = GetWorld()
        ? GetWorld()->GetTimeSeconds() + FMath::Max(0.0f, DelaySeconds)
        : FMath::Max(0.0f, DelaySeconds);

    // Death must not inherit a stacked menu/input lock or leave a clickable debug-style screen behind.
    bScoreboardVisible = false;
    bFrontendMenuVisible = false;
    bChatInputActive = false;
    bSettingsVisible = false;
    bDeploymentPanelVisible = false;
    bAdminPanelVisible = false;

    ResetIgnoreMoveInput();
    ResetIgnoreLookInput();
    SetIgnoreMoveInput(true);
    SetIgnoreLookInput(true);
    bShowMouseCursor = false;
    if (PlayerInput)
    {
        PlayerInput->FlushPressedKeys();
    }
    SetInputMode(FInputModeGameOnly());

    if (IsValid(DeathViewTarget))
    {
        SetViewTargetWithBlend(DeathViewTarget, 0.15f);
    }

    UE_LOG(LogTemp, Display,
        TEXT("GAME_RECOVERY_RESPAWN_CLIENT_WAIT delay_seconds=%.1f input_locked=1 cursor=0 corpse_view=%d spectator_pawn=0 gameplay_debugger=0"),
        FMath::Max(0.0f, DelaySeconds), IsValid(DeathViewTarget) ? 1 : 0);
}

void AOCPlayerController::ClientFinishRespawnWait_Implementation(APawn* NewPawn)
{
    if (!IsLocalController() || !bRespawnWaiting)
    {
        return;
    }

    bRespawnWaiting = false;
    RespawnWaitDeadlineSeconds = 0.0;

    ResetIgnoreMoveInput();
    ResetIgnoreLookInput();
    bShowMouseCursor = false;
    if (PlayerInput)
    {
        PlayerInput->FlushPressedKeys();
    }
    SetInputMode(FInputModeGameOnly());

    if (IsValid(NewPawn))
    {
        SetViewTarget(NewPawn);
    }

    ConfigureControllerInput();

    UE_LOG(LogTemp, Display,
        TEXT("GAME_RECOVERY_RESPAWN_CLIENT_READY pawn=%s input_game_only=1 input_lock_cleared=1 cursor=0 possession_restored=%d spectator_pawn=0 gameplay_debugger=0"),
        *GetNameSafe(NewPawn), IsValid(NewPawn) ? 1 : 0);
}

float AOCPlayerController::GetRespawnSecondsRemaining() const
{
    if (!bRespawnWaiting)
    {
        return 0.0f;
    }

    const UWorld* World = GetWorld();
    if (!World)
    {
        return FMath::Max(0.0, RespawnWaitDeadlineSeconds);
    }

    return static_cast<float>(FMath::Max(0.0, RespawnWaitDeadlineSeconds - World->GetTimeSeconds()));
}
