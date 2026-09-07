#include "OCPlayerController.h"

#include "OCCharacter.h"
#include "OCHealthComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "GameFramework/PlayerInput.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
    constexpr float GameRecoveryRespawnDelaySeconds = 10.0f;
}

void AOCPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    HideRespawnOverlay();
    Super::EndPlay(EndPlayReason);
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

    ShowRespawnOverlay();

    UE_LOG(LogTemp, Display,
        TEXT("GAME_RECOVERY_RESPAWN_CLIENT_WAIT delay_seconds=%.1f input_locked=1 cursor=0 corpse_view=%d spectator_pawn=0 gameplay_debugger=0 respawn_hud=1"),
        FMath::Max(0.0f, DelaySeconds), IsValid(DeathViewTarget) ? 1 : 0);
}

void AOCPlayerController::ClientFinishRespawnWait_Implementation(APawn* NewPawn)
{
    if (!IsLocalController())
    {
        return;
    }

    HideRespawnOverlay();

    if (!bRespawnWaiting)
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
        TEXT("GAME_RECOVERY_RESPAWN_CLIENT_READY pawn=%s input_game_only=1 input_lock_cleared=1 cursor=0 possession_restored=%d spectator_pawn=0 gameplay_debugger=0 respawn_hud_removed=1"),
        *GetNameSafe(NewPawn), IsValid(NewPawn) ? 1 : 0);
}

void AOCPlayerController::ShowRespawnOverlay()
{
    if (!IsLocalController() || !GEngine || !GEngine->GameViewport)
    {
        return;
    }

    HideRespawnOverlay();

    const TWeakObjectPtr<AOCPlayerController> WeakController(this);
    RespawnOverlayWidget =
        SNew(SBox)
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Center)
        [
            SNew(SBorder)
            .BorderImage(FCoreStyle::Get().GetBrush(TEXT("GenericWhiteBox")))
            .BorderBackgroundColor(FLinearColor(0.018f, 0.024f, 0.032f, 0.90f))
            .Padding(FMargin(30.0f, 16.0f))
            [
                SNew(STextBlock)
                .Text_Lambda([WeakController]() -> FText
                {
                    const AOCPlayerController* Controller = WeakController.Get();
                    if (!Controller || !Controller->IsRespawnWaiting())
                    {
                        return FText::GetEmpty();
                    }
                    const int32 Seconds = FMath::Max(0, FMath::CeilToInt(Controller->GetRespawnSecondsRemaining()));
                    return FText::Format(
                        NSLOCTEXT("OsterConflict", "RespawnCountdown", "RESPAWN  {0}"),
                        FText::AsNumber(Seconds));
                })
                .Font(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 26))
                .ColorAndOpacity(FSlateColor(FLinearColor(0.92f, 0.95f, 0.98f, 1.0f)))
            ]
        ];

    GEngine->GameViewport->AddViewportWidgetContent(RespawnOverlayWidget.ToSharedRef(), 1900);

    UE_LOG(LogTemp, Display,
        TEXT("GAME_RECOVERY_RESPAWN_HUD_READY countdown=1 debug_text=0 spectator_overlay=0"));
}

void AOCPlayerController::HideRespawnOverlay()
{
    if (!RespawnOverlayWidget.IsValid())
    {
        return;
    }

    if (GEngine && GEngine->GameViewport)
    {
        GEngine->GameViewport->RemoveViewportWidgetContent(RespawnOverlayWidget.ToSharedRef());
    }
    RespawnOverlayWidget.Reset();
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
        return static_cast<float>(FMath::Max(0.0, RespawnWaitDeadlineSeconds));
    }

    return static_cast<float>(FMath::Max(0.0, RespawnWaitDeadlineSeconds - World->GetTimeSeconds()));
}
