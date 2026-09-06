#include "OCPass45DeploymentInputRecoverySubsystem.h"

#include "OCPlayerController.h"

#include "Engine/World.h"
#include "GameFramework/PlayerInput.h"

bool UOCPass45DeploymentInputRecoverySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && World->GetNetMode() != NM_DedicatedServer &&
        (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCPass45DeploymentInputRecoverySubsystem::Tick(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World) return;

    AOCPlayerController* PC = Cast<AOCPlayerController>(World->GetFirstPlayerController());
    const bool bDeploymentOnly = PC && PC->IsLocalController() &&
        PC->IsDeploymentPanelVisible() && !PC->IsFrontendMenuVisible() && !PC->IsSettingsVisible();

    if (!bDeploymentOnly)
    {
        if (ArmedController.Get() != PC)
        {
            ArmedController.Reset();
        }
        bDeploymentInputArmed = false;
        return;
    }

    if (bDeploymentInputArmed && ArmedController.Get() == PC)
    {
        return;
    }

    // One transition, one input arm. Never repeat SetInputMode while Slate is routing a click.
    PC->ResetIgnoreMoveInput();
    PC->ResetIgnoreLookInput();
    PC->SetIgnoreMoveInput(true);
    PC->SetIgnoreLookInput(true);
    PC->bShowMouseCursor = true;
    PC->bEnableClickEvents = true;
    PC->bEnableMouseOverEvents = true;
    if (PC->PlayerInput)
    {
        PC->PlayerInput->FlushPressedKeys();
    }

    FInputModeGameAndUI Mode;
    Mode.SetHideCursorDuringCapture(false);
    Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    PC->SetInputMode(Mode);

    ArmedController = PC;
    bDeploymentInputArmed = true;
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_DEPLOYMENT_INPUT_RECOVERY_READY one_shot=1 mouse_unlocked=1 click_events=1 frontend_hidden=1"));
}

TStatId UOCPass45DeploymentInputRecoverySubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCPass45DeploymentInputRecoverySubsystem, STATGROUP_Tickables);
}
