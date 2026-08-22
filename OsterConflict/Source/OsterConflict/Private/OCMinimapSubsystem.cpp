#include "OCMinimapSubsystem.h"

#include "OCPlayerController.h"
#include "OCTacticalMapSubsystem.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"

namespace
{
    constexpr float MinimapOuterSize = 184.0f;
    constexpr float MinimapInnerSize = 172.0f;
}

void UOCMinimapWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (!WidgetTree || WidgetTree->RootWidget) return;

    UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("MinimapRoot"));
    WidgetTree->RootWidget = Root;

    UBorder* Frame = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("MinimapFrame"));
    Frame->SetBrushColor(FLinearColor(0.018f, 0.024f, 0.029f, 0.86f));
    Frame->SetPadding(FMargin(6.0f));
    UCanvasPanelSlot* FrameSlot = Root->AddChildToCanvas(Frame);
    FrameSlot->SetAnchors(FAnchors(0.0f, 0.0f));
    FrameSlot->SetPosition(FVector2D(22.0f, 72.0f));
    FrameSlot->SetSize(FVector2D(MinimapOuterSize, MinimapOuterSize));

    UCanvasPanel* MapCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("MinimapCanvas"));
    Frame->SetContent(MapCanvas);

    MapImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("MinimapImage"));
    UCanvasPanelSlot* ImageSlot = MapCanvas->AddChildToCanvas(MapImage);
    ImageSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
    ImageSlot->SetOffsets(FMargin(0.0f));
    MapImage->SetColorAndOpacity(FLinearColor(0.92f, 0.92f, 0.92f, 0.94f));

    PlayerMarker = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("MinimapPlayerMarker"));
    PlayerMarker->SetText(FText::FromString(TEXT("▲")));
    PlayerMarker->SetColorAndOpacity(FSlateColor(FLinearColor(0.98f, 0.98f, 0.98f, 1.0f)));
    PlayerMarker->SetJustification(ETextJustify::Center);
    PlayerMarker->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
    FSlateFontInfo MarkerFont = PlayerMarker->GetFont();
    MarkerFont.Size = 15;
    PlayerMarker->SetFont(MarkerFont);
    UCanvasPanelSlot* MarkerSlot = MapCanvas->AddChildToCanvas(PlayerMarker);
    MarkerSlot->SetSize(FVector2D(26.0f, 26.0f));
    MarkerSlot->SetAlignment(FVector2D(0.5f, 0.5f));
    MarkerSlot->SetPosition(FVector2D(MinimapInnerSize * 0.5f, MinimapInnerSize * 0.5f));
}

void UOCMinimapWidget::Configure(UTextureRenderTarget2D* InMapTexture, const FOCTacticalMapProjection& InProjection)
{
    if (!MapImage || !InMapTexture || !InProjection.IsValid()) return;

    Projection = InProjection;
    FSlateBrush Brush;
    Brush.SetResourceObject(InMapTexture);
    Brush.ImageSize = FVector2D(MinimapInnerSize, MinimapInnerSize);
    MapImage->SetBrush(Brush);
    bConfigured = true;
}

void UOCMinimapWidget::UpdatePlayerMarker(const FVector& WorldLocation, float WorldYawDegrees)
{
    if (!bConfigured || !PlayerMarker) return;

    const FVector2D UV = Projection.WorldToUV(WorldLocation, true);
    if (UCanvasPanelSlot* MarkerSlot = Cast<UCanvasPanelSlot>(PlayerMarker->Slot))
    {
        MarkerSlot->SetPosition(FVector2D(UV.X * MinimapInnerSize, UV.Y * MinimapInnerSize));
    }
    PlayerMarker->SetRenderTransformAngle(Projection.WorldYawToMapDegrees(WorldYawDegrees));
}

bool UOCMinimapSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && World->GetNetMode() != NM_DedicatedServer &&
        (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

TStatId UOCMinimapSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCMinimapSubsystem, STATGROUP_Tickables);
}

void UOCMinimapSubsystem::EnsureMinimap(AOCPlayerController& PlayerController)
{
    if (MinimapWidget.IsValid()) return;

    UWorld* World = GetWorld();
    if (!World) return;

    const double Now = FPlatformTime::Seconds();
    if (Now < NextBuildAttemptSeconds) return;
    NextBuildAttemptSeconds = Now + 1.0;

    UOCTacticalMapSubsystem* TacticalMap = World->GetSubsystem<UOCTacticalMapSubsystem>();
    if (!TacticalMap || !TacticalMap->EnsureMapSnapshot() || !TacticalMap->GetMapRenderTarget() ||
        !TacticalMap->GetMapProjection().IsValid())
    {
        return;
    }

    UOCMinimapWidget* Widget = CreateWidget<UOCMinimapWidget>(&PlayerController, UOCMinimapWidget::StaticClass());
    if (!Widget) return;

    Widget->AddToViewport(120);
    Widget->Configure(TacticalMap->GetMapRenderTarget(), TacticalMap->GetMapProjection());
    MinimapWidget = Widget;
}

void UOCMinimapSubsystem::Tick(float DeltaTime)
{
    UWorld* World = GetWorld();
    AOCPlayerController* PlayerController = World ? Cast<AOCPlayerController>(World->GetFirstPlayerController()) : nullptr;
    if (!PlayerController || !PlayerController->IsLocalController()) return;

    EnsureMinimap(*PlayerController);
    UOCMinimapWidget* Widget = MinimapWidget.Get();
    if (!Widget) return;

    UOCTacticalMapSubsystem* TacticalMap = World->GetSubsystem<UOCTacticalMapSubsystem>();
    APawn* Pawn = PlayerController->GetPawn();
    const bool bBlocked = !Pawn || PlayerController->IsFrontendMenuVisible() || PlayerController->IsDeploymentPanelVisible() ||
        PlayerController->IsAdminPanelVisible() || PlayerController->IsChatInputActive() || PlayerController->IsSettingsVisible() ||
        (TacticalMap && TacticalMap->IsMapOpen());

    Widget->SetVisibility(bBlocked ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
    if (!bBlocked)
    {
        Widget->UpdatePlayerMarker(Pawn->GetActorLocation(), Pawn->GetActorRotation().Yaw);
    }
}

void UOCMinimapSubsystem::Deinitialize()
{
    if (UOCMinimapWidget* Widget = MinimapWidget.Get()) Widget->RemoveFromParent();
    MinimapWidget.Reset();
    Super::Deinitialize();
}
