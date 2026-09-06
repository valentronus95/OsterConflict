#include "OCDeploymentLoadingSubsystem.h"

#include "OCAuthoredWorldSurfaceUpgradeSubsystem.h"
#include "OCDenseGroundFoliageSubsystem.h"
#include "OCLandmarkStartupCoordinatorSubsystem.h"
#include "OCPass45ImportedGrenadeVisualSubsystem.h"
#include "OCPlayerController.h"
#include "OCProductionCharacterAssetsSubsystem.h"

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
    CardSlot->SetSize(FVector2D(560.0f, 205.0f));

    UVerticalBox* Stack = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("DeploymentLoadingStack"));
    Card->SetContent(Stack);

    UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DeploymentLoadingTitle"));
    Title->SetText(FText::FromString(TEXT("ЗАВАНТАЖЕННЯ")));
    Title->SetColorAndOpacity(FSlateColor(FLinearColor(0.95f, 0.96f, 0.97f, 1.0f)));
    FSlateFontInfo TitleFont = Title->GetFont();
    TitleFont.Size = 22;
    Title->SetFont(TitleFont);
    Stack->AddChildToVerticalBox(Title)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));

    StatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DeploymentLoadingStatus"));
    StatusText->SetText(FText::FromString(TEXT("ПІДГОТОВКА КАРТИ")));
    StatusText->SetColorAndOpacity(FSlateColor(FLinearColor(0.72f, 0.75f, 0.78f, 1.0f)));
    FSlateFontInfo StatusFont = StatusText->GetFont();
    StatusFont.Size = 14;
    StatusText->SetFont(StatusFont);
    Stack->AddChildToVerticalBox(StatusText)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 18.0f));

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

void UOCDeploymentLoadingWidget::SetLoadingStatus(const FText& Status)
{
    if (StatusText) StatusText->SetText(Status);
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
        Widget->SetLoadingProgress(0.02f);
        Widget->SetLoadingStatus(FText::FromString(TEXT("ПІДГОТОВКА КАРТИ")));
        LoadingWidget = Widget;
    }

    UE_LOG(LogTemp, Display,
        TEXT("GAME_RECOVERY_DEPLOYMENT_LOADING_BEGIN wait_for_landmarks=1 wait_for_surfaces=1 wait_for_foliage=1 wait_for_characters=1 wait_for_grenades=1 spawn_before_ready=0"));
}

void UOCDeploymentLoadingSubsystem::Tick(float DeltaTime)
{
    (void)DeltaTime;
    if (!bActive) return;

    AOCPlayerController* Controller = PendingController.Get();
    if (!Controller)
    {
        FinishDeploymentTransition();
        return;
    }

    UWorld* World = GetWorld();
    const UOCLandmarkStartupCoordinatorSubsystem* Landmarks =
        World ? World->GetSubsystem<UOCLandmarkStartupCoordinatorSubsystem>() : nullptr;
    const UOCAuthoredWorldSurfaceUpgradeSubsystem* Surfaces =
        World ? World->GetSubsystem<UOCAuthoredWorldSurfaceUpgradeSubsystem>() : nullptr;
    const UOCDenseGroundFoliageSubsystem* Foliage =
        World ? World->GetSubsystem<UOCDenseGroundFoliageSubsystem>() : nullptr;
    const UOCProductionCharacterAssetsSubsystem* Characters =
        World ? World->GetSubsystem<UOCProductionCharacterAssetsSubsystem>() : nullptr;
    const UOCPass45ImportedGrenadeVisualSubsystem* Grenades =
        World ? World->GetSubsystem<UOCPass45ImportedGrenadeVisualSubsystem>() : nullptr;

    const bool bLandmarksReady = Landmarks == nullptr || Landmarks->IsWorldStartupReady();
    const bool bSurfacesReady = Surfaces == nullptr || Surfaces->IsWorldSurfaceReady();
    const bool bFoliageReady = Foliage == nullptr || Foliage->IsWorldFoliageReady();
    const bool bCharactersReady = Characters == nullptr || Characters->IsCharacterAssetsReady();
    const bool bGrenadesReady = Grenades == nullptr || Grenades->IsGrenadePresentationReady();
    const bool bWorldReady = bLandmarksReady && bSurfacesReady && bFoliageReady && bCharactersReady && bGrenadesReady;

    const float LandmarksProgress = Landmarks ? Landmarks->GetStartupProgress() : 1.0f;
    const float SurfacesProgress = Surfaces ? Surfaces->GetWorldSurfaceProgress() : 1.0f;
    const float FoliageProgress = Foliage ? Foliage->GetWorldFoliageProgress() : 1.0f;
    const float CharacterProgress = Characters ? Characters->GetCharacterAssetsProgress() : 1.0f;
    const float GrenadeProgress = Grenades ? Grenades->GetGrenadePresentationProgress() : 1.0f;
    const float WorldProgress = FMath::Clamp(
        LandmarksProgress * 0.40f + SurfacesProgress * 0.18f + FoliageProgress * 0.22f +
        CharacterProgress * 0.10f + GrenadeProgress * 0.10f,
        0.0f, 1.0f);

    const double Now = FPlatformTime::Seconds();
    const double Elapsed = FMath::Max(0.0, Now - StartTimeSeconds);

    // Do not use the player pawn as a loading screen. Ready/restart is held until critical world presentation,
    // production character packages and grenade first-use assets have completed behind the deployment UI.
    if (!bReadySent && bWorldReady && Elapsed >= 0.12)
    {
        bReadySent = true;
        Controller->UIReadyDeploy();
        UE_LOG(LogTemp, Display,
            TEXT("GAME_RECOVERY_DEPLOYMENT_WORLD_READY landmarks=1 surfaces=1 foliage=1 characters=1 grenades=1 ready_request_sent=1 elapsed=%.2f"),
            Elapsed);
    }

    float Progress = bReadySent
        ? FMath::Min(0.94f, 0.90f + static_cast<float>(Elapsed) * 0.01f)
        : FMath::Clamp(0.03f + WorldProgress * 0.87f, 0.03f, 0.90f);

    if (UOCDeploymentLoadingWidget* Widget = LoadingWidget.Get())
    {
        Widget->SetLoadingProgress(Progress);
        Widget->SetLoadingStatus(FText::FromString(bReadySent ? TEXT("ПОЯВА НА КАРТІ") : TEXT("ПІДГОТОВКА КАРТИ")));
    }

    const bool bPossessedAndReleased = bReadySent && Controller->GetPawn() != nullptr && !Controller->IsDeploymentPanelVisible();
    if (bPossessedAndReleased)
    {
        if (CompletionStartSeconds < 0.0) CompletionStartSeconds = Now;
        const float CompletionAlpha = FMath::Clamp(static_cast<float>((Now - CompletionStartSeconds) / 0.25), 0.0f, 1.0f);
        Progress = FMath::Lerp(0.94f, 1.0f, CompletionAlpha);
        if (UOCDeploymentLoadingWidget* Widget = LoadingWidget.Get())
        {
            Widget->SetLoadingProgress(Progress);
            Widget->SetLoadingStatus(FText::FromString(TEXT("ГОТОВО")));
        }
        if (CompletionAlpha >= 1.0f)
        {
            UE_LOG(LogTemp, Display,
                TEXT("GAME_RECOVERY_DEPLOYMENT_COMPLETE world_ready_before_spawn=1 character_packages_ready_before_spawn=1 grenade_assets_ready_before_spawn=1 post_spawn_world_builds=0 elapsed=%.2f"),
                Elapsed);
            FinishDeploymentTransition();
            return;
        }
    }

    // Fail closed: if critical preparation cannot finish, return control to deployment instead of spawning
    // into a half-built city or allowing first-use grenade package loads on the game thread.
    if (!bReadySent && Elapsed >= 45.0)
    {
        UE_LOG(LogTemp, Error,
            TEXT("GAME_RECOVERY_DEPLOYMENT_TIMEOUT landmarks=%d surfaces=%d foliage=%d characters=%d grenades=%d progress=%.3f elapsed=%.2f"),
            bLandmarksReady ? 1 : 0,
            bSurfacesReady ? 1 : 0,
            bFoliageReady ? 1 : 0,
            bCharactersReady ? 1 : 0,
            bGrenadesReady ? 1 : 0,
            WorldProgress,
            Elapsed);
        FinishDeploymentTransition();
        return;
    }

    if (bReadySent && Elapsed >= 60.0)
    {
        UE_LOG(LogTemp, Error,
            TEXT("GAME_RECOVERY_POSSESSION_TIMEOUT pawn=%d deployment_visible=%d elapsed=%.2f"),
            Controller->GetPawn() != nullptr ? 1 : 0,
            Controller->IsDeploymentPanelVisible() ? 1 : 0,
            Elapsed);
        FinishDeploymentTransition();
    }
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