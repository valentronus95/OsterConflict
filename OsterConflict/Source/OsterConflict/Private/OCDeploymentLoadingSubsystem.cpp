#include "OCDeploymentLoadingSubsystem.h"

#include "OCPlayerController.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/World.h"

void UOCDeploymentLoadingWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (!WidgetTree || WidgetTree->RootWidget) return;

    UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("DeploymentLoadingCanvas"));
    WidgetTree->RootWidget = Canvas;

    UBorder* Scrim = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("DeploymentLoadingScrim"));
    // The transition is intentionally fully blocking. A translucent scrim exposed the deployment panel changing
    // underneath it and made the START -> spawn transition look like another broken intermediate screen.
    Scrim->SetBrushColor(FLinearColor(0.006f, 0.009f, 0.012f, 1.0f));
    UCanvasPanelSlot* ScrimSlot = Canvas->AddChildToCanvas(Scrim);
    ScrimSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
    ScrimSlot->SetOffsets(FMargin(0.0f));

    UBorder* Card = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("DeploymentLoadingCard"));
    Card->SetBrushColor(FLinearColor(0.035f, 0.043f, 0.050f, 0.985f));
    Card->SetPadding(FMargin(30.0f, 24.0f));
    UCanvasPanelSlot* CardSlot = Canvas->AddChildToCanvas(Card);
    CardSlot->SetAnchors(FAnchors(0.5f, 0.5f));
    CardSlot->SetAlignment(FVector2D(0.5f, 0.5f));
    CardSlot->SetPosition(FVector2D::ZeroVector);
    CardSlot->SetSize(FVector2D(560.0f, 170.0f));

    UVerticalBox* Stack = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("DeploymentLoadingStack"));
    Card->SetContent(Stack);

    UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DeploymentLoadingTitle"));
    Title->SetText(FText::FromString(TEXT("ЗАВАНТАЖЕННЯ")));
    Title->SetColorAndOpacity(FSlateColor(FLinearColor(0.95f, 0.96f, 0.97f, 1.0f)));
    FSlateFontInfo TitleFont = Title->GetFont();
    TitleFont.Size = 22;
    Title->SetFont(TitleFont);
    Stack->AddChildToVerticalBox(Title)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 18.0f));

    ProgressBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("DeploymentLoadingProgress"));
    ProgressBar->SetPercent(0.0f);
    ProgressBar->SetFillColorAndOpacity(FLinearColor(0.80f, 0.82f, 0.84f, 1.0f));
    if (UVerticalBoxSlot* ProgressSlot = Stack->AddChildToVerticalBox(ProgressBar))
    {
        ProgressSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 12.0f));
    }

    PercentText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DeploymentLoadingPercent"));
    PercentText->SetText(FText::FromString(TEXT("0%")));
    PercentText->SetColorAndOpacity(FSlateColor(FLinearColor(0.72f, 0.75f, 0.78f, 1.0f)));
    FSlateFontInfo PercentFont = PercentText->GetFont();
    PercentFont.Size = 15;
    PercentText->SetFont(PercentFont);
    Stack->AddChildToVerticalBox(PercentText);
}

void UOCDeploymentLoadingWidget::SetLoadingProgress(float NormalizedProgress)
{
    const float Clamped = FMath::Clamp(NormalizedProgress, 0.0f, 1.0f);
    if (ProgressBar) ProgressBar->SetPercent(Clamped);
    if (PercentText)
    {
        PercentText->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), FMath::RoundToInt(Clamped * 100.0f))));
    }
}

bool UOCDeploymentLoadingSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

TStatId UOCDeploymentLoadingSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCDeploymentLoadingSubsystem, STATGROUP_Tickables);
}

void UOCDeploymentLoadingSubsystem::BeginDeployment(AOCPlayerController* Controller)
{
    if (!Controller || !Controller->IsLocalController() || bActive) return;

    PendingController = Controller;
    StartTimeSeconds = FPlatformTime::Seconds();
    CompletionStartSeconds = -1.0;
    bReadySent = false;
    bActive = true;

    UOCDeploymentLoadingWidget* Widget = CreateWidget<UOCDeploymentLoadingWidget>(Controller, UOCDeploymentLoadingWidget::StaticClass());
    if (Widget)
    {
        Widget->AddToViewport(5000);
        Widget->SetLoadingProgress(0.0f);
        LoadingWidget = Widget;
    }
}

void UOCDeploymentLoadingSubsystem::Tick(float DeltaTime)
{
    if (!bActive) return;

    AOCPlayerController* Controller = PendingController.Get();
    if (!Controller)
    {
        FinishDeploymentTransition();
        return;
    }

    const double Now = FPlatformTime::Seconds();
    const double Elapsed = FMath::Max(0.0, Now - StartTimeSeconds);

    // Keep the 0% loading frame visible before the authoritative restart request. The opaque transition layer
    // prevents the underlying deployment panel from visibly shifting while the server changes possession state.
    if (!bReadySent && Elapsed >= 0.12)
    {
        bReadySent = true;
        Controller->UIReadyDeploy();
    }

    float Progress = bReadySent
        ? FMath::Min(0.92f, 0.14f + static_cast<float>(Elapsed) * 0.22f)
        : FMath::Min(0.12f, static_cast<float>(Elapsed) * 0.95f);

    const bool bPossessedAndReleased = bReadySent && Controller->GetPawn() != nullptr && !Controller->IsDeploymentPanelVisible();
    if (bPossessedAndReleased)
    {
        if (CompletionStartSeconds < 0.0) CompletionStartSeconds = Now;
        const float CompletionAlpha = FMath::Clamp(static_cast<float>((Now - CompletionStartSeconds) / 0.25), 0.0f, 1.0f);
        Progress = FMath::Lerp(0.92f, 1.0f, CompletionAlpha);
        if (CompletionAlpha >= 1.0f)
        {
            if (UOCDeploymentLoadingWidget* Widget = LoadingWidget.Get()) Widget->SetLoadingProgress(1.0f);
            FinishDeploymentTransition();
            return;
        }
    }

    // Never trap input behind a dead overlay if the server refuses or fails to possess the player.
    if (Elapsed >= 12.0)
    {
        FinishDeploymentTransition();
        return;
    }

    if (UOCDeploymentLoadingWidget* Widget = LoadingWidget.Get()) Widget->SetLoadingProgress(Progress);
}

void UOCDeploymentLoadingSubsystem::FinishDeploymentTransition()
{
    if (UOCDeploymentLoadingWidget* Widget = LoadingWidget.Get()) Widget->RemoveFromParent();
    LoadingWidget.Reset();
    PendingController.Reset();
    StartTimeSeconds = 0.0;
    CompletionStartSeconds = -1.0;
    bReadySent = false;
    bActive = false;
}
