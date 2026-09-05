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
                // The flow panel itself must also be opaque. The old 0.87 alpha was visible in the user's
                // deployment screenshots even when the backdrop happened to exist.
                Border->SetBrushColor(FLinearColor(0.008f, 0.012f, 0.016f, 1.0f));
            }
            else
            {
                Border->SetBrushColor(FLinearColor(0.030f, 0.036f, 0.041f, 1.0f));
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
    SetPresentationVisible(PC->IsDeploymentPanelVisible() && !PC->IsSettingsVisible());

    if (!bUpdateBudgetLogged)
    {
        bUpdateBudgetLogged = true;
        UE_LOG(LogTemp, Display,
            TEXT("PASS40_DEPLOYMENT_PRESENTATION_BUDGET_READY update_hz=10 root_scan=cache_miss visibility_writes=deduped style_writes=once_per_root"));
    }
}

UOCGameUIRootWidget* UOCR13DeploymentPresentationSubsystem::ResolveRoot(UWorld* World, AOCPlayerController* PC)
{
    if (!World || !PC) return nullptr;

    if (UOCGameUIRootWidget* ExistingRoot = ActiveRoot.Get())
    {
        if (ActiveController.Get() == PC && ExistingRoot->GetWorld() == World && ExistingRoot->GetOwningPlayer() == PC)
        {
            return ExistingRoot;
        }
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

    // Build the full-screen opaque backdrop independently of the dynamically-created flow panel.
    // Previously FindObjectFast(R13_DeploymentFlowPanel) ran first; when it missed the late widget we returned
    // before creating any backdrop, which is exactly why the world kept showing through the deployment menu.
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
            Shade->SetBrushColor(FLinearColor(0.012f, 0.016f, 0.020f, 1.0f));
            Shade->SetIsEnabled(false);
            Shade->SetRenderOpacity(1.0f);
            Shade->SetVisibility(ESlateVisibility::Collapsed);
            FillCanvas(Canvas->AddChildToCanvas(Shade), 9189);
            BackdropShade = Shade;
            bPresentationVisibilityValid = false;
        }
    }

    UBorder* FlowPanel = FindObjectFast<UBorder>(Root, TEXT("R13_DeploymentFlowPanel"));
    if (!FlowPanel)
    {
        // The backdrop is already owned and can be shown this tick; styling the late flow panel can wait.
        return;
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
    FlowPanel->SetRenderOpacity(1.0f);
    RestyleWidgetRecursive(FlowPanel, FlowPanel);
    FlowPanel->InvalidateLayoutAndVolatility();
}

void UOCR13DeploymentPresentationSubsystem::SetPresentationVisible(const bool bVisible)
{
    if (bPresentationVisibilityValid && bLastPresentationVisible == bVisible) return;

    const ESlateVisibility Visibility = bVisible
        ? ESlateVisibility::SelfHitTestInvisible
        : ESlateVisibility::Collapsed;

    if (BackdropBlur.IsValid()) BackdropBlur->SetVisibility(ESlateVisibility::Collapsed);
    if (BackdropShade.IsValid()) BackdropShade->SetVisibility(Visibility);

    bPresentationVisibilityValid = true;
    bLastPresentationVisible = bVisible;
}

TStatId UOCR13DeploymentPresentationSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCR13DeploymentPresentationSubsystem, STATGROUP_Tickables);
}