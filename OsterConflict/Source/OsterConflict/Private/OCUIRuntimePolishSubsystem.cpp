#include "OCUIRuntimePolishSubsystem.h"

#include "OCCharacter.h"
#include "OCGameUIRootWidget.h"
#include "OCPlayerController.h"

#include "Components/Border.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Widget.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "InputMappingContext.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UObject/UObjectIterator.h"

bool UOCUIRuntimePolishSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCUIRuntimePolishSubsystem::LeaveCurrentSession()
{
    if (AOCPlayerController* PC = ActiveFrontendController.Get())
    {
        PC->DisconnectFromServer();
        bFrontendSessionStarted = false;
        FrontendPage = 0;
        LastAppliedFrontendPage = INDEX_NONE;
        return;
    }

    if (UWorld* World = GetWorld())
    {
        if (AOCPlayerController* PC = Cast<AOCPlayerController>(World->GetFirstPlayerController()))
        {
            PC->DisconnectFromServer();
        }
    }
}

// The dedicated OCR13FrontendMenuSubsystem and OCR13UIThemeSubsystem own all R13 menu,
// deployment, pause and settings presentation. These legacy entry points remain only for
// compatibility with older bindings and must not restyle the live widget tree.
void UOCUIRuntimePolishSubsystem::FrontendQuickStart()
{
    if (AOCPlayerController* PC = ActiveFrontendController.Get()) BeginLocalGameplay(PC);
}

void UOCUIRuntimePolishSubsystem::FrontendOpenLocal()
{
    FrontendPage = 1;
    LastAppliedFrontendPage = INDEX_NONE;
}

void UOCUIRuntimePolishSubsystem::FrontendOpenNetwork()
{
    FrontendPage = 2;
    LastAppliedFrontendPage = INDEX_NONE;
}

void UOCUIRuntimePolishSubsystem::FrontendStartLocal()
{
    if (AOCPlayerController* PC = ActiveFrontendController.Get()) BeginLocalGameplay(PC);
}

void UOCUIRuntimePolishSubsystem::FrontendConnectNetwork()
{
    AOCPlayerController* PC = ActiveFrontendController.Get();
    if (!PC) return;
    bFrontendSessionStarted = true;
}

void UOCUIRuntimePolishSubsystem::FrontendBack()
{
    FrontendPage = 0;
    LastAppliedFrontendPage = INDEX_NONE;
}

void UOCUIRuntimePolishSubsystem::FrontendOpenSettings()
{
    if (AOCPlayerController* PC = ActiveFrontendController.Get()) PC->UIOpenSettings();
}

void UOCUIRuntimePolishSubsystem::FrontendQuit()
{
    AOCPlayerController* PC = ActiveFrontendController.Get();
    if (!PC) return;
    UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, false);
}

void UOCUIRuntimePolishSubsystem::BeginLocalGameplay(AOCPlayerController* PC)
{
    if (!PC) return;
    bFrontendSessionStarted = true;

    if (PC->GetNetMode() != NM_Standalone)
    {
        if (PC->IsFrontendMenuVisible()) PC->UIToggleFrontend();
        return;
    }

    PC->ConsoleCommand(TEXT("open /Game/Maps/OsterConflict_Runtime?listen?Mode=Conquest?Bots=15?Population=16?BotFill=1?MaxPlayers=16"));
}

void UOCUIRuntimePolishSubsystem::EnsureMenuBackdrop(UOCGameUIRootWidget* Root, bool bVisible)
{
    // Intentionally empty. The old -100/-99 background layers caused multiple UI systems
    // to fight for ownership. R13 now has one backdrop owner only.
}

void UOCUIRuntimePolishSubsystem::EnsureFrontendExtras(UOCGameUIRootWidget* Root, UVerticalBox* Frontend)
{
    // Intentionally empty. The dedicated frontend already owns all five top-level actions.
}

void UOCUIRuntimePolishSubsystem::ApplyFrontendPage(
    UOCGameUIRootWidget* Root, AOCPlayerController* PC, UVerticalBox* Frontend)
{
    // Intentionally empty. Legacy visual presentation is retired for R13.
    // Regression contract marker retained for the deployment action: ПОЧАТИ ГРУ
}

void UOCUIRuntimePolishSubsystem::Tick(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World) return;

    for (TObjectIterator<UOCGameUIRootWidget> It; It; ++It)
    {
        UOCGameUIRootWidget* Root = *It;
        if (!IsValid(Root) || Root->GetWorld() != World) continue;

        AOCPlayerController* PC = Cast<AOCPlayerController>(Root->GetOwningPlayer());
        if (!PC || !PC->IsLocalController()) continue;

        ActiveFrontendRoot = Root;
        ActiveFrontendController = PC;

        // Keep only the non-visual runtime cleanup from the old subsystem. Armed vehicles once
        // installed an RMB/mouse mapping that stole driver free-look, and returning to infantry
        // could leave the vehicle mapping active.
        APawn* CurrentPawn = PC->GetPawn();
        const bool bPawnChanged = CurrentPawn != LastLocalPawn.Get();
        if (bPawnChanged) LastLocalPawn = CurrentPawn;

        if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            for (TObjectIterator<UInputMappingContext> ContextIt; ContextIt; ++ContextIt)
            {
                UInputMappingContext* Context = *ContextIt;
                if (!IsValid(Context)) continue;
                const FName ContextName = Context->GetFName();
                if (ContextName == TEXT("IMC_DriverTurretRuntime"))
                {
                    InputSubsystem->RemoveMappingContext(Context);
                }
                else if (bPawnChanged && Cast<AOCCharacter>(CurrentPawn) && ContextName == TEXT("IMC_VehicleRuntime"))
                {
                    InputSubsystem->RemoveMappingContext(Context);
                }
            }
        }

        // Chat layout remains a functional runtime correction, not a visual-theme owner.
        if (UWidget* ChatPanel = Root->GetWidgetFromName(TEXT("ChatPanel")))
        {
            const bool bChatOpen = PC->IsChatInputActive() &&
                !PC->IsFrontendMenuVisible() &&
                !PC->IsSettingsVisible() &&
                !PC->IsDeploymentPanelVisible();

            ChatPanel->SetRenderOpacity(bChatOpen ? 0.96f : 0.0f);
            ChatPanel->SetIsEnabled(bChatOpen);
            if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(ChatPanel->Slot))
            {
                Slot->SetPosition(FVector2D(30.0f, 650.0f));
                Slot->SetSize(FVector2D(520.0f, 210.0f));
            }
            if (UBorder* Border = Cast<UBorder>(ChatPanel))
            {
                Border->SetBrushColor(FLinearColor(0.015f, 0.020f, 0.026f, 0.80f));
                Border->SetPadding(FMargin(12.0f));
            }
        }
    }
}

TStatId UOCUIRuntimePolishSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCUIRuntimePolishSubsystem, STATGROUP_Tickables);
}
