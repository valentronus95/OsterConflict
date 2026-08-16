#include "OCUIRuntimePolishSubsystem.h"

#include "OCCharacter.h"
#include "OCGameUIRootWidget.h"
#include "OCPlayerController.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/Widget.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "InputMappingContext.h"
#include "UObject/UObjectIterator.h"

namespace
{
    void SetChildVisibility(UVerticalBox* Box, int32 Index, ESlateVisibility Visibility)
    {
        if (!Box || Index < 0 || Index >= Box->GetChildrenCount()) return;
        if (UWidget* Child = Box->GetChildAt(Index)) Child->SetVisibility(Visibility);
    }

    void PolishButtons(UWidget* Widget)
    {
        if (!Widget) return;
        if (UButton* Button = Cast<UButton>(Widget))
        {
            Button->SetBackgroundColor(FLinearColor(0.12f, 0.15f, 0.19f, 1.0f));
        }
        if (UPanelWidget* Panel = Cast<UPanelWidget>(Widget))
        {
            for (int32 Index = 0; Index < Panel->GetChildrenCount(); ++Index)
            {
                PolishButtons(Panel->GetChildAt(Index));
            }
        }
    }
}

bool UOCUIRuntimePolishSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    if (!World) return false;
    return World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE;
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

        // R12.2 vehicle-exit hotfix. The runtime vehicle mapping context used WASD at a higher priority than
        // character input and could survive a possession hand-off. Remove only that vehicle context when the
        // local pawn changes back to an OCCharacter. This preserves all normal player/controller mappings.
        APawn* CurrentPawn = PC->GetPawn();
        if (CurrentPawn != LastLocalPawn.Get())
        {
            LastLocalPawn = CurrentPawn;
            if (Cast<AOCCharacter>(CurrentPawn))
            {
                if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
                    ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
                {
                    for (TObjectIterator<UInputMappingContext> ContextIt; ContextIt; ++ContextIt)
                    {
                        UInputMappingContext* Context = *ContextIt;
                        if (IsValid(Context) && Context->GetFName() == TEXT("IMC_VehicleRuntime"))
                        {
                            InputSubsystem->RemoveMappingContext(Context);
                        }
                    }
                }
            }
        }

        // The old widget kept ChatPanel as SelfHitTestInvisible even while chat input was closed.
        // Hide it visually and functionally until the player explicitly opens chat.
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

        // Deployment is a player decision screen, not a server console. Keep team/role/squad/spawn/deploy controls,
        // remove the giant 15-bot text dump and fit the whole flow into a compact dark panel.
        if (UBorder* DeploymentPanel = Cast<UBorder>(Root->GetWidgetFromName(TEXT("DeploymentPanel"))))
        {
            DeploymentPanel->SetBrushColor(FLinearColor(0.018f, 0.024f, 0.032f, 0.96f));
            DeploymentPanel->SetPadding(FMargin(22.0f));
            PolishButtons(DeploymentPanel->GetContent());

            if (UHorizontalBox* Columns = Cast<UHorizontalBox>(DeploymentPanel->GetContent()))
            {
                if (Columns->GetChildrenCount() >= 3)
                {
                    if (UVerticalBox* RightColumn = Cast<UVerticalBox>(Columns->GetChildAt(2)))
                    {
                        // Child 0 = compact human/bot population summary, child 1 = huge debug roster.
                        SetChildVisibility(RightColumn, 0, ESlateVisibility::Visible);
                        SetChildVisibility(RightColumn, 1, ESlateVisibility::Collapsed);
                    }
                }
            }

            if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(DeploymentPanel->Slot))
            {
                Slot->SetPosition(FVector2D(240.0f, 170.0f));
                Slot->SetSize(FVector2D(1120.0f, 500.0f));
            }
        }

        // In an actual listen-server/client match the old Frontend is really the Escape menu. Do not show
        // username/IP/connect controls there. Keep a clear game-menu title, Settings and Continue/Close.
        if (UBorder* FrontendPanel = Cast<UBorder>(Root->GetWidgetFromName(TEXT("FrontendPanel"))))
        {
            const bool bInGameplaySession = PC->GetNetMode() != NM_Standalone;
            UVerticalBox* Frontend = Cast<UVerticalBox>(FrontendPanel->GetContent());

            if (bInGameplaySession && Frontend)
            {
                if (Frontend->GetChildrenCount() > 0)
                    if (UTextBlock* Title = Cast<UTextBlock>(Frontend->GetChildAt(0)))
                        Title->SetText(NSLOCTEXT("OCR12UI", "PauseMenuTitle", "МЕНЮ ГРИ"));

                SetChildVisibility(Frontend, 1, ESlateVisibility::Collapsed);
                SetChildVisibility(Frontend, 2, ESlateVisibility::Collapsed);
                SetChildVisibility(Frontend, 3, ESlateVisibility::Collapsed);
                SetChildVisibility(Frontend, 4, ESlateVisibility::Collapsed);
                SetChildVisibility(Frontend, 5, ESlateVisibility::Collapsed);
                SetChildVisibility(Frontend, 6, ESlateVisibility::Visible);
                SetChildVisibility(Frontend, 7, ESlateVisibility::Visible);
                SetChildVisibility(Frontend, 8, ESlateVisibility::Collapsed);

                FrontendPanel->SetBrushColor(FLinearColor(0.018f, 0.024f, 0.032f, 0.96f));
                FrontendPanel->SetPadding(FMargin(22.0f));
                PolishButtons(Frontend);
                if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(FrontendPanel->Slot))
                {
                    Slot->SetPosition(FVector2D(540.0f, 260.0f));
                    Slot->SetSize(FVector2D(520.0f, 300.0f));
                }
            }
            else if (Frontend)
            {
                if (Frontend->GetChildrenCount() > 0)
                    if (UTextBlock* Title = Cast<UTextBlock>(Frontend->GetChildAt(0)))
                        Title->SetText(NSLOCTEXT("OCGameUIRootWidget", "Title", "OSTER CONFLICT"));

                for (int32 Index = 1; Index < Frontend->GetChildrenCount(); ++Index)
                    SetChildVisibility(Frontend, Index, ESlateVisibility::Visible);
                SetChildVisibility(Frontend, 7, ESlateVisibility::Collapsed);
                PolishButtons(Frontend);

                if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(FrontendPanel->Slot))
                {
                    Slot->SetPosition(FVector2D(500.0f, 120.0f));
                    Slot->SetSize(FVector2D(620.0f, 560.0f));
                }
            }
        }
    }
}

TStatId UOCUIRuntimePolishSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCUIRuntimePolishSubsystem, STATGROUP_Tickables);
}
