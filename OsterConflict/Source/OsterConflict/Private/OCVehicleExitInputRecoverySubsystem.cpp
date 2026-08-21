#include "OCVehicleExitInputRecoverySubsystem.h"

#include "OCCharacter.h"
#include "OCPlayerController.h"
#include "OCVehicleBase.h"

#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

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

    InWorld.GetTimerManager().SetTimer(
        PossessionPollTimer,
        this,
        &UOCVehicleExitInputRecoverySubsystem::PollLocalPossession,
        0.05f,
        true,
        0.05f);
}

void UOCVehicleExitInputRecoverySubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PossessionPollTimer);
    }
    LastLocalPawn.Reset();
    bLastPawnWasVehicle = false;
    Super::Deinitialize();
}

void UOCVehicleExitInputRecoverySubsystem::PollLocalPossession()
{
    UWorld* World = GetWorld();
    if (!World) return;

    AOCPlayerController* PC = Cast<AOCPlayerController>(World->GetFirstPlayerController());
    if (!PC || !PC->IsLocalController()) return;

    APawn* CurrentPawn = PC->GetPawn();
    if (CurrentPawn == LastLocalPawn.Get()) return;

    const bool bPreviousWasVehicle = bLastPawnWasVehicle;
    LastLocalPawn = CurrentPawn;
    bLastPawnWasVehicle = CurrentPawn && CurrentPawn->IsA<AOCVehicleBase>();

    if (bPreviousWasVehicle && CurrentPawn && CurrentPawn->IsA<AOCCharacter>())
    {
        RestoreCharacterInput(*PC);
    }
}

void UOCVehicleExitInputRecoverySubsystem::RestoreCharacterInput(AOCPlayerController& PlayerController)
{
    ULocalPlayer* LocalPlayer = PlayerController.GetLocalPlayer();
    if (!LocalPlayer) return;

    if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
    {
        // Rebuild from a known state instead of stacking the character context underneath the old
        // priority-20 vehicle context. UIApplyLocalPreferences re-adds the controller context and
        // the possessed AOCCharacter's current user-remapped gameplay context immediately below.
        InputSubsystem->ClearAllMappings();
    }

    PlayerController.ResetIgnoreMoveInput();
    PlayerController.ResetIgnoreLookInput();
    PlayerController.bShowMouseCursor = false;
    PlayerController.SetInputMode(FInputModeGameOnly());
    PlayerController.UIApplyLocalPreferences();

    UE_LOG(LogTemp, Display,
        TEXT("Vehicle exit input recovery: restored GameOnly, controller mappings and character mappings."));
}
