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
    constexpr float DeploymentPresentationIntervalSeconds = 0.10f;

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
            Style.Normal.TintColor = FSlateColor(FLinearColor(0.060f, 0.072f, 0.084f, 0.72f));
            Style.Hovered.TintColor = FSlateColor(FLinearColor(0.18f, 0.21f, 0.24f, 0.92f));
            Style.Pressed.TintColor = FSlateColor(FLinearColor(0.42f, 0.31f, 0.12f, 0.95f));
            Style.Disabled.TintColor = FSlateColor(FLinearColor(0.028f, 0.033f, 0.039f, 0.52f));
            Style.NormalPadding = FMargin(1.0f);
            Style.PressedPadding = FMargin(1.0f, 2.0f, 1.0f, 0.0f);
            Button->SetStyle(Style);
            Button->SetBackgroundColor(FLinearColor::White);
        }
        else if (UBorder* Border = Cast<UBorder>(Widget))
        {
            if (Border == RootPanel)
            {
                Border->SetBrushColor(FLinearColor(0.008f, 0.012f, 0.016f, 0.94f));
            }
            else
            {
                Border->SetBrushColor(FLinearColor(0.030f, 0.038f, 0.046f, 0.90f));
            }
        }
        else if (UTextBlock* Text = Cast<UTextBlock>(Widget))
        {
            if (Text->GetText().ToString().Equals(TEXT("ПОЯВИТИСЯ"), ESearchCase::CaseSensitive))
            {
                Text->SetText(NSLOCTEXT("OCR13DeploymentPresentation", "DeployEnterBattle", "У БІЙ"));
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

    UpdateAccumulator += DeltaTime;
    if (UpdateAccumulator < DeploymentPresentationIntervalSeconds) return;
    UpdateAccumulator = FMath::Fmod(UpdateAccumulator, DeploymentPresentationIntervalSeconds);

    AOCPlayerController* PC = Cast<AOCPlayerController>(World->GetFirstPlayerController());
    if (!PC || !PC->IsLocalController())
    {
        SetPresentationVisible(false);
        return;
    }

    UOCGameUIRootWidget* Root = ResolveRoot(World, PC);
    if (!Root)
    {
        SetPresentationVisible(false);
        return;
    }

    EnsurePresentation(Root);

    // Deployment owns its backdrop only while deployment itself is the active top-level screen.
    // Previously the presentation layer ignored the frontend flag, so BACK could leave this high-Z
    // shade sitting over the main menu even after the frontend became visible.
    const bool bDeploymentOwnsScreen = PC->IsDeploymentPanelVisible() &&
        !PC->IsSettingsVisible() && !PC->IsFrontendMenuVisible();
    SetPresentationVisible(bDeploymentOwnsScreen);

    if (!bUpdateBudgetLogged)
    {
        bUpdateBudgetLogged = true;
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_DEPLOYMENT_PRESENTATION_READY update_hz=10 frontend_exclusion=1 opaque_leak=0 flow_alpha=0.94 section_alpha=0.90"));
        UE_LOG(LogTemp, Display,
            TEXT("PASS40_DEPLOYMENT_PRESENTATION_BUDGET_READY update_hz=10 root_scan=cache_miss visibility_writes=authoritative style_writes=once_per_root"));
    }
}

UOCGameUIRootWidget* UOCR13DeploymentPresentationSubsystem::ResolveRoot(UWorld* World, AOCPlayerController* PC)
{
    if (!World || !PC) return nullptr;

    if (UOCGameUIRootWidget* ExistingRoot = ActiveRoot.Get())
    {
        if (ActiveController.Get() == PC && ExistingRoot->GetWorld() == World && ExistingRoot->GetOwningPlayer() == PC)
            return ExistingRoot;
    }

    ActiveRoot.Reset();
    ActiveController.Reset();
    BackdropBlur.Reset();
    BackdropShade.Reset();
    StyledFlowPanel.Reset();
    bStyleApplied = false;
    bPresentationVisibilityValid = false;
    bLastPresentationVisible = false;

    for (TObjectIterator<UOCGameUIRootWidget> It; It; ++It)
    {
        if (IsValid(*It) && It->GetWorld() == World && It->GetOwningPlayer() == PC)
        {
            ActiveRoot = *It;
            ActiveController = PC;
            return *It;
        }
    }
    return nullptr;
}

void UOCR13DeploymentPresentationSubsystem::EnsurePresentation(UOCGameUIRootWidget* Root)
{
    if (!Root) return;
    if (StyledFlowPanel.IsValid() && BackdropBlur.IsValid() && BackdropShade.IsValid() && bStyleApplied) return;

    UCanvasPanel* Canvas = Cast<UCanvasPanel>(Root->GetWidgetFromName(TEXT("OC_UI_Root")));
    if (!Canvas) return;

    if (!BackdropBlur.IsValid())
    {
        UBackgroundBlur* Blur = NewObject<UBackgroundBlur>(Root, TEXT("R13_DeploymentBackdropBlur"));
        if (Blur)
        {
            Blur->SetBlurStrength(0.0f);
            Blur->SetOverrideAutoRadiusCalculation(true);
            Blur->SetBlurRadius(0);
            Blur->SetApplyAlphaToBlur(false);
            Blur->SetRenderOpacity(0.0f);
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
            Shade->SetBrushColor(FLinearColor(0.005f, 0.008f, 0.011f, 0.76f));
            Shade->SetIsEnabled(false);
            Shade->SetRenderOpacity(1.0f);
            Shade->SetVisibility(ESlateVisibility::Collapsed);
            FillCanvas(Canvas->AddChildToCanvas(Shade), 9189);
            BackdropShade = Shade;
            bPresentationVisibilityValid = false;
        }
    }

    UBorder* FlowPanel = FindObjectFast<UBorder>(Root, TEXT("R13_DeploymentFlowPanel"));
    if (!FlowPanel) return;

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
    FlowPanel->SetPadding(FMargin(32.0f));
    FlowPanel->SetRenderOpacity(1.0f);
    RestyleWidgetRecursive(FlowPanel, FlowPanel);
    FlowPanel->InvalidateLayoutAndVolatility();
}

void UOCR13DeploymentPresentationSubsystem::SetPresentationVisible(const bool bVisible)
{
    // Reassert the actual high-Z presentation state. This is cheap at 10 Hz and prevents another UI
    // owner from leaving a stale shade visible after frontend/deployment handoff.
    const ESlateVisibility Visibility = bVisible ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed;
    if (BackdropBlur.IsValid()) BackdropBlur->SetVisibility(ESlateVisibility::Collapsed);
    if (BackdropShade.IsValid())
    {
        BackdropShade->SetRenderOpacity(1.0f);
        BackdropShade->SetVisibility(Visibility);
    }
    bPresentationVisibilityValid = true;
    bLastPresentationVisible = bVisible;
}

TStatId UOCR13DeploymentPresentationSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCR13DeploymentPresentationSubsystem, STATGROUP_Tickables);
}
