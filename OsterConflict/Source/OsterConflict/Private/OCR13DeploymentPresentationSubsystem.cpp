#include "OCR13DeploymentPresentationSubsystem.h"

#include "OCGameUIRootWidget.h"
#include "OCPlayerController.h"

#include "Components/BackgroundBlur.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "Styling/SlateTypes.h"
#include "UObject/UObjectIterator.h"

namespace
{
    void FillCanvas(UCanvasPanelSlot* Slot, const int32 ZOrder)
    {
        if (!Slot) return;
        Slot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
        Slot->SetOffsets(FMargin(0.0f));
        Slot->SetAlignment(FVector2D::ZeroVector);
        Slot->SetZOrder(ZOrder);
    }

    void RestyleWidgetRecursive(UWidget* Widget, UBorder* RootPanel)
    {
        if (!Widget) return;

        if (UButton* Button = Cast<UButton>(Widget))
        {
            FButtonStyle Style = Button->GetStyle();
            Style.Normal.TintColor = FSlateColor(FLinearColor(0.025f, 0.030f, 0.034f, 0.14f));
            Style.Hovered.TintColor = FSlateColor(FLinearColor(0.17f, 0.19f, 0.20f, 0.34f));
            Style.Pressed.TintColor = FSlateColor(FLinearColor(0.42f, 0.35f, 0.20f, 0.42f));
            Style.Disabled.TintColor = FSlateColor(FLinearColor(0.018f, 0.022f, 0.026f, 0.11f));
            Style.NormalPadding = FMargin(1.0f);
            Style.PressedPadding = FMargin(1.0f, 2.0f, 1.0f, 0.0f);
            Button->SetStyle(Style);
            Button->SetBackgroundColor(FLinearColor::White);
        }
        else if (UBorder* Border = Cast<UBorder>(Widget))
        {
            if (Border == RootPanel)
            {
                Border->SetBrushColor(FLinearColor(0.008f, 0.012f, 0.016f, 0.87f));
            }
            else
            {
                Border->SetBrushColor(FLinearColor(0.030f, 0.036f, 0.041f, 0.76f));
            }
        }

        if (UPanelWidget* Panel = Cast<UPanelWidget>(Widget))
        {
            for (int32 Index = 0; Index < Panel->GetChildrenCount(); ++Index)
            {
                RestyleWidgetRecursive(Panel->GetChildAt(Index), RootPanel);
            }
        }
    }
}

bool UOCR13DeploymentPresentationSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13DeploymentPresentationSubsystem::Tick(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World || DeltaTime <= 0.0f) return;

    AOCPlayerController* PC = Cast<AOCPlayerController>(World->GetFirstPlayerController());
    if (!PC || !PC->IsLocalController())
    {
        SetPresentationVisible(false);
        return;
    }

    UOCGameUIRootWidget* Root = nullptr;
    for (TObjectIterator<UOCGameUIRootWidget> It; It; ++It)
    {
        if (IsValid(*It) && It->GetWorld() == World && It->GetOwningPlayer() == PC)
        {
            Root = *It;
            break;
        }
    }
    if (!Root)
    {
        SetPresentationVisible(false);
        return;
    }

    EnsurePresentation(Root);
    SetPresentationVisible(PC->IsDeploymentPanelVisible() && !PC->IsSettingsVisible());
}

void UOCR13DeploymentPresentationSubsystem::EnsurePresentation(UOCGameUIRootWidget* Root)
{
    if (!Root) return;

    UBorder* FlowPanel = FindObjectFast<UBorder>(Root, TEXT("R13_DeploymentFlowPanel"));
    if (!FlowPanel) return;

    if (ActiveRoot.Get() != Root)
    {
        ActiveRoot = Root;
        BackdropBlur.Reset();
        BackdropShade.Reset();
        StyledFlowPanel.Reset();
        bStyleApplied = false;
    }

    UCanvasPanel* Canvas = Cast<UCanvasPanel>(Root->GetWidgetFromName(TEXT("OC_UI_Root")));
    if (!Canvas) return;

    if (!BackdropBlur.IsValid())
    {
        UBackgroundBlur* Blur = NewObject<UBackgroundBlur>(Root, TEXT("R13_DeploymentBackdropBlur"));
        if (Blur)
        {
            // Mild blur only. UE's BackgroundBlur cost grows with strength/radius, so this is intentionally restrained
            // for the user's laptop while still separating the four-step UI from the menu artwork underneath.
            Blur->SetBlurStrength(3.5f);
            Blur->SetOverrideAutoRadiusCalculation(true);
            Blur->SetBlurRadius(4);
            Blur->SetApplyAlphaToBlur(true);
            Blur->SetRenderOpacity(0.72f);
            Blur->SetVisibility(ESlateVisibility::Collapsed);
            FillCanvas(Canvas->AddChildToCanvas(Blur), 9188);
            BackdropBlur = Blur;
        }
    }

    if (!BackdropShade.IsValid())
    {
        UBorder* Shade = NewObject<UBorder>(Root, TEXT("R13_DeploymentBackdropShade"));
        if (Shade)
        {
            Shade->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.22f));
            Shade->SetIsEnabled(false);
            Shade->SetVisibility(ESlateVisibility::Collapsed);
            FillCanvas(Canvas->AddChildToCanvas(Shade), 9189);
            BackdropShade = Shade;
        }
    }

    StyledFlowPanel = FlowPanel;
    if (!bStyleApplied)
    {
        ApplyWidgetStyle(FlowPanel);
        bStyleApplied = true;
    }
}

void UOCR13DeploymentPresentationSubsystem::ApplyWidgetStyle(UBorder* FlowPanel)
{
    if (!FlowPanel) return;
    FlowPanel->SetPadding(FMargin(28.0f));
    RestyleWidgetRecursive(FlowPanel, FlowPanel);
    FlowPanel->InvalidateLayoutAndVolatility();
}

void UOCR13DeploymentPresentationSubsystem::SetPresentationVisible(const bool bVisible)
{
    const ESlateVisibility Visibility = bVisible
        ? ESlateVisibility::SelfHitTestInvisible
        : ESlateVisibility::Collapsed;
    if (BackdropBlur.IsValid()) BackdropBlur->SetVisibility(Visibility);
    if (BackdropShade.IsValid()) BackdropShade->SetVisibility(Visibility);
}

TStatId UOCR13DeploymentPresentationSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCR13DeploymentPresentationSubsystem, STATGROUP_Tickables);
}
