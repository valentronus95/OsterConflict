#include "OCUIRuntimePolishSubsystem.h"

#include "OCGameUIRootWidget.h"
#include "OCPlayerController.h"

#include "Components/Border.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/Widget.h"
#include "Engine/World.h"
#include "UObject/UObjectIterator.h"

namespace
{
    void SetChildVisibility(UVerticalBox* Box, int32 Index, ESlateVisibility Visibility)
    {
        if (!Box || Index < 0 || Index >= Box->GetChildrenCount()) return;
        if (UWidget* Child = Box->GetChildAt(Index)) Child->SetVisibility(Visibility);
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

        // R12.1: the old widget keeps ChatPanel as SelfHitTestInvisible even while the chat input is closed.
        // That produced the huge dark rectangle seen in the visual test. Opacity is independent from its legacy
        // visibility refresh, so this reliably removes the rectangle without touching the chat/network backend.
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

        // In an actual listen-server/client match the old 'Frontend' is really the Escape menu. Do not show
        // username/IP/connect controls there: they are connection-screen concepts and made the in-match menu
        // confusing. Keep only a clear title, Settings and Close/Continue. Standalone frontend remains unchanged.
        if (UBorder* FrontendPanel = Cast<UBorder>(Root->GetWidgetFromName(TEXT("FrontendPanel"))))
        {
            const bool bInGameplaySession = PC->GetNetMode() != NM_Standalone;
            UVerticalBox* Frontend = Cast<UVerticalBox>(FrontendPanel->GetContent());

            if (bInGameplaySession && Frontend)
            {
                // Source-built S17 child order: title, subtitle, username, address, connect, localhost,
                // settings, close, status. This branch deliberately keeps that backend intact.
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
                if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(FrontendPanel->Slot))
                {
                    Slot->SetPosition(FVector2D(540.0f, 260.0f));
                    Slot->SetSize(FVector2D(520.0f, 300.0f));
                }
            }
            else if (Frontend)
            {
                if (UTextBlock* Title = Cast<UTextBlock>(Frontend->GetChildAt(0)))
                    Title->SetText(NSLOCTEXT("OCGameUIRootWidget", "Title", "OSTER CONFLICT"));
                for (int32 Index = 1; Index < Frontend->GetChildrenCount(); ++Index)
                    SetChildVisibility(Frontend, Index, ESlateVisibility::Visible);
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
