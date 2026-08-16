#include "OCUIRuntimePolishSubsystem.h"

#include "OCCharacter.h"
#include "OCGameUIRootWidget.h"
#include "OCPlayerController.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ContentWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/Widget.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/Texture2D.h"
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

    void SetText(UVerticalBox* Box, int32 Index, const FText& Text)
    {
        if (!Box || Index < 0 || Index >= Box->GetChildrenCount()) return;
        if (UTextBlock* TextBlock = Cast<UTextBlock>(Box->GetChildAt(Index))) TextBlock->SetText(Text);
    }

    void SetButtonText(UVerticalBox* Box, int32 Index, const FText& Text)
    {
        if (!Box || Index < 0 || Index >= Box->GetChildrenCount()) return;
        if (UButton* Button = Cast<UButton>(Box->GetChildAt(Index)))
        {
            if (UTextBlock* Label = Cast<UTextBlock>(Button->GetContent())) Label->SetText(Text);
        }
    }

    void PolishButtons(UWidget* Widget)
    {
        if (!Widget) return;
        if (UButton* Button = Cast<UButton>(Widget))
        {
            Button->SetBackgroundColor(FLinearColor(0.025f, 0.042f, 0.060f, 0.98f));
            if (UTextBlock* Label = Cast<UTextBlock>(Button->GetContent()))
            {
                Label->SetColorAndOpacity(FSlateColor(FLinearColor(0.94f, 0.96f, 0.98f, 1.0f)));
                FSlateFontInfo Font = Label->GetFont();
                Font.Size = 16;
                Label->SetFont(Font);
            }
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

        // Vehicle-specific mappings must never survive possession hand-off back to infantry.
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
                        if (!IsValid(Context)) continue;
                        const FName ContextName = Context->GetFName();
                        if (ContextName == TEXT("IMC_VehicleRuntime") || ContextName == TEXT("IMC_DriverTurretRuntime"))
                        {
                            InputSubsystem->RemoveMappingContext(Context);
                        }
                    }
                }
            }
        }

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

        // R13.1 deployment: one readable decision flow. No bot dump, no giant debug rectangle, no mystery buttons.
        if (UBorder* DeploymentPanel = Cast<UBorder>(Root->GetWidgetFromName(TEXT("DeploymentPanel"))))
        {
            static TWeakObjectPtr<UTexture2D> CachedOsterBackground;
            static bool bTriedBackground = false;
            if (!bTriedBackground)
            {
                bTriedBackground = true;
                CachedOsterBackground = LoadObject<UTexture2D>(nullptr, TEXT("/Game/R13/UI/Oster_Menu_BG.Oster_Menu_BG"));
            }

            if (CachedOsterBackground.IsValid())
            {
                DeploymentPanel->SetBrushFromTexture(CachedOsterBackground.Get());
                // Dark photo treatment keeps Oster clearly visible while preserving text contrast.
                DeploymentPanel->SetBrushColor(FLinearColor(0.20f, 0.23f, 0.27f, 1.0f));
            }
            else
            {
                DeploymentPanel->SetBrushColor(FLinearColor(0.010f, 0.016f, 0.024f, 0.985f));
            }
            DeploymentPanel->SetPadding(FMargin(30.0f));
            PolishButtons(DeploymentPanel->GetContent());

            if (UHorizontalBox* Columns = Cast<UHorizontalBox>(DeploymentPanel->GetContent()))
            {
                if (Columns->GetChildrenCount() >= 3)
                {
                    if (UVerticalBox* Left = Cast<UVerticalBox>(Columns->GetChildAt(0)))
                    {
                        SetText(Left, 0, NSLOCTEXT("OCR13UI", "DeployTitle", "OSTER CONFLICT"));
                        SetText(Left, 1, NSLOCTEXT("OCR13UI", "DeploySubtitle", "ОСТЕР  •  ВИБЕРІТЬ КОМАНДУ, КЛАС І ГРУПУ"));
                        SetButtonText(Left, 2, NSLOCTEXT("OCR13UI", "Team1", "1  КОМАНДА 1"));
                        SetButtonText(Left, 3, NSLOCTEXT("OCR13UI", "Team2", "1  КОМАНДА 2"));
                        SetButtonText(Left, 4, NSLOCTEXT("OCR13UI", "Role", "2  КЛАС  •  ЗМІНИТИ"));
                        SetButtonText(Left, 5, NSLOCTEXT("OCR13UI", "Squad", "3  ГРУПА  •  ЗМІНИТИ"));
                    }
                    if (UVerticalBox* Spawn = Cast<UVerticalBox>(Columns->GetChildAt(1)))
                    {
                        SetText(Spawn, 0, NSLOCTEXT("OCR13UI", "SpawnTitle", "4  ТОЧКА ПОЯВИ"));
                        SetText(Spawn, 1, NSLOCTEXT("OCR13UI", "SpawnHint", "ОБЕРІТЬ МІСЦЕ ПОЯВИ"));
                        SetButtonText(Spawn, 2, NSLOCTEXT("OCR13UI", "Base", "БАЗА"));
                        SetButtonText(Spawn, 3, NSLOCTEXT("OCR13UI", "PointA", "ТОЧКА A"));
                        SetButtonText(Spawn, 4, NSLOCTEXT("OCR13UI", "PointB", "ТОЧКА B"));
                        SetButtonText(Spawn, 5, NSLOCTEXT("OCR13UI", "PointC", "ТОЧКА C"));
                        SetButtonText(Spawn, 6, NSLOCTEXT("OCR13UI", "Deploy", "5  У БІЙ"));
                    }
                    if (UVerticalBox* DebugColumn = Cast<UVerticalBox>(Columns->GetChildAt(2)))
                    {
                        DebugColumn->SetVisibility(ESlateVisibility::Collapsed);
                    }
                }
            }

            if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(DeploymentPanel->Slot))
            {
                Slot->SetPosition(FVector2D(190.0f, 150.0f));
                Slot->SetSize(FVector2D(1220.0f, 560.0f));
            }
        }

        // Escape during a match is a pause menu, not the direct-connect frontend.
        if (UBorder* FrontendPanel = Cast<UBorder>(Root->GetWidgetFromName(TEXT("FrontendPanel"))))
        {
            const bool bInGameplaySession = PC->GetNetMode() != NM_Standalone;
            UVerticalBox* Frontend = Cast<UVerticalBox>(FrontendPanel->GetContent());

            if (bInGameplaySession && Frontend)
            {
                if (Frontend->GetChildrenCount() > 0)
                    if (UTextBlock* Title = Cast<UTextBlock>(Frontend->GetChildAt(0)))
                        Title->SetText(NSLOCTEXT("OCR13UI", "PauseMenuTitle", "МЕНЮ ГРИ"));

                SetChildVisibility(Frontend, 1, ESlateVisibility::Collapsed);
                SetChildVisibility(Frontend, 2, ESlateVisibility::Collapsed);
                SetChildVisibility(Frontend, 3, ESlateVisibility::Collapsed);
                SetChildVisibility(Frontend, 4, ESlateVisibility::Collapsed);
                SetChildVisibility(Frontend, 5, ESlateVisibility::Collapsed);
                SetChildVisibility(Frontend, 6, ESlateVisibility::Visible);
                SetChildVisibility(Frontend, 7, ESlateVisibility::Visible);
                SetChildVisibility(Frontend, 8, ESlateVisibility::Collapsed);
                SetButtonText(Frontend, 6, NSLOCTEXT("OCR13UI", "PauseSettings", "НАЛАШТУВАННЯ"));
                SetButtonText(Frontend, 7, NSLOCTEXT("OCR13UI", "PauseContinue", "ПРОДОВЖИТИ ГРУ"));

                FrontendPanel->SetBrushColor(FLinearColor(0.010f, 0.016f, 0.024f, 0.985f));
                FrontendPanel->SetPadding(FMargin(26.0f));
                PolishButtons(Frontend);
                if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(FrontendPanel->Slot))
                {
                    Slot->SetPosition(FVector2D(555.0f, 285.0f));
                    Slot->SetSize(FVector2D(490.0f, 250.0f));
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
