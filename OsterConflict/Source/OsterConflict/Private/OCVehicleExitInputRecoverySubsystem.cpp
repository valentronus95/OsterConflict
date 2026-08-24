#include "OCVehicleExitInputRecoverySubsystem.h"

#include "OCCharacter.h"
#include "OCPlayerController.h"
#include "OCVehicleBase.h"

#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

namespace
{
    constexpr float TransitionPollSeconds = 0.05f;
    constexpr float StablePollSeconds = 0.10f;
}

bool UOCVehicleExitInputRecoverySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCVehicleExitInputRecoverySubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    if (InWorld.GetNetMode() == NM_DedicatedServer) return;

    ScheduleNextPoll(TransitionPollSeconds);

    if (!bPollBudgetLogged)
    {
        bPollBudgetLogged = true;
        UE_LOG(LogTemp, Display,
            TEXT("PASS41_INPUT_RECOVERY_POLL_BUDGET_READY transition_hz=20 stable_hz=10 repeating_timer=0"));
    }
}

void UOCVehicleExitInputRecoverySubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PossessionPollTimer);
    }
    LastLocalPawn.Reset();
    LastRecoveredCharacterPawn.Reset();
    bLastPawnWasVehicle = false;
    Super::Deinitialize();
}

void UOCVehicleExitInputRecoverySubsystem::ScheduleNextPoll(const float DelaySeconds)
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            PossessionPollTimer,
            this,
            &UOCVehicleExitInputRecoverySubsystem::PollLocalPossession,
            FMath::Max(0.01f, DelaySeconds),
            false);
    }
}

void UOCVehicleExitInputRecoverySubsystem::PollLocalPossession()
{
    UWorld* World = GetWorld();
    if (!World) return;

    AOCPlayerController* PC = Cast<AOCPlayerController>(World->GetFirstPlayerController());
    if (!PC || !PC->IsLocalController())
    {
        ScheduleNextPoll(StablePollSeconds);
        return;
    }

    APawn* CurrentPawn = PC->GetPawn();
    if (CurrentPawn != LastLocalPawn.Get())
    {
        const bool bPreviousWasVehicle = bLastPawnWasVehicle;
        LastLocalPawn = CurrentPawn;
        bLastPawnWasVehicle = CurrentPawn && CurrentPawn->IsA<AOCVehicleBase>();

        // A vehicle can temporarily replace the character as the possessed pawn. Mark the
        // character recovery as pending so the same character pawn is rebuilt again on exit.
        if (bLastPawnWasVehicle || bPreviousWasVehicle)
        {
            LastRecoveredCharacterPawn.Reset();
        }
    }

    AOCCharacter* Character = Cast<AOCCharacter>(CurrentPawn);
    if (!Character)
    {
        ScheduleNextPoll(bLastPawnWasVehicle ? TransitionPollSeconds : StablePollSeconds);
        return;
    }

    // Do not fight legitimate UI locks. While a deployment/settings transition is active, keep the
    // original 20 Hz response so input is restored promptly after the UI releases its lock.
    const bool bIntentionalUILock =
        PC->IsFrontendMenuVisible() ||
        PC->IsDeploymentPanelVisible() ||
        PC->IsAdminPanelVisible() ||
        PC->IsChatInputActive() ||
        PC->IsSettingsVisible();
    if (bIntentionalUILock)
    {
        ScheduleNextPoll(TransitionPollSeconds);
        return;
    }

    if (LastRecoveredCharacterPawn.Get() == Character)
    {
        ScheduleNextPoll(StablePollSeconds);
        return;
    }

    RestoreCharacterInput(*PC);
    LastRecoveredCharacterPawn = Character;

    UE_LOG(LogTemp, Display,
        TEXT("PASS31_GAMEPLAY_INPUT_READY pawn=%s moveIgnored=%d lookIgnored=%d"),
        *GetNameSafe(Character),
        PC->IsMoveInputIgnored() ? 1 : 0,
        PC->IsLookInputIgnored() ? 1 : 0);

    ScheduleNextPoll(StablePollSeconds);
}

void UOCVehicleExitInputRecoverySubsystem::RestoreCharacterInput(AOCPlayerController& PlayerController)
{
    ULocalPlayer* LocalPlayer = PlayerController.GetLocalPlayer();
    if (!LocalPlayer) return;

    if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
    {
        // Rebuild from a known state instead of leaving a stale high-priority vehicle/menu mapping
        // above the character context. UIApplyLocalPreferences immediately restores the controller
        // context and the possessed AOCCharacter's current user-remapped gameplay context.
        InputSubsystem->ClearAllMappings();
    }

    // SetIgnoreMoveInput / SetIgnoreLookInput are stack based. A deployment or frontend transition
    // can leave more than one ignore request behind, so reset the stacks instead of issuing one
    // matching false call.
    PlayerController.ResetIgnoreMoveInput();
    PlayerController.ResetIgnoreLookInput();
    PlayerController.bShowMouseCursor = false;
    PlayerController.SetInputMode(FInputModeGameOnly());
    PlayerController.UIApplyLocalPreferences();

    UE_LOG(LogTemp, Display,
        TEXT("Pass 31 character input recovery: restored GameOnly and rebuilt Enhanced Input mappings."));
}
