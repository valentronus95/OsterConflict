#include "OCTacticalMapSubsystem.h"

#include "OCCapturePoint.h"
#include "OCGeoReference.h"
#include "OCWorldSectorOster.h"

#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateImageBrush.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "EngineUtils.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"

namespace
{
    constexpr float PolishMapWidth = 1248.0f;
    constexpr float PolishMapHeight = 702.0f;

    const FLinearColor PolishGrid(0.46f, 0.53f, 0.55f, 0.075f);
    const FLinearColor PolishGridText(0.58f, 0.63f, 0.65f, 0.78f);
    const FLinearColor PolishMuted(0.58f, 0.63f, 0.65f, 0.92f);
    const FLinearColor PolishText(0.88f, 0.91f, 0.92f, 1.0f);
    const FLinearColor PolishAmber(0.95f, 0.55f, 0.12f, 1.0f);
    const FLinearColor PolishPlayer(0.50f, 0.90f, 0.32f, 1.0f);
    const FLinearColor PolishLive(0.34f, 0.82f, 0.48f, 1.0f);

    void SetPolishTextSize(UTextBlock* Text, const int32 Size)
    {
        if (!Text) return;
        FSlateFontInfo Font = Text->GetFont();
        Font.Size = Size;
        Text->SetFont(Font);
    }

    UCanvasPanelSlot* AddPolishWidget(
        UCanvasPanel* Canvas,
        UWidget* Widget,
        const FVector2D Position,
        const FVector2D Size,
        const int32 ZOrder,
        const FVector2D Alignment = FVector2D::ZeroVector)
    {
        if (!Canvas || !Widget) return nullptr;
        UCanvasPanelSlot* CanvasSlot = Canvas->AddChildToCanvas(Widget);
        if (!CanvasSlot) return nullptr;
        CanvasSlot->SetPosition(Position);
        CanvasSlot->SetSize(Size);
        CanvasSlot->SetAlignment(Alignment);
        CanvasSlot->SetZOrder(ZOrder);
        return CanvasSlot;
    }

    UTextBlock* AddPolishText(
        UWidgetTree* Tree,
        UCanvasPanel* Canvas,
        const FName Name,
        const FString& Value,
        const FVector2D Position,
        const FVector2D Size,
        const int32 FontSize,
        const FLinearColor& Color,
        const int32 ZOrder,
        const ETextJustify::Type Justification = ETextJustify::Left)
    {
        if (!Tree || !Canvas) return nullptr;
        UTextBlock* Text = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
        Text->SetText(FText::FromString(Value));
        Text->SetColorAndOpacity(FSlateColor(Color));
        Text->SetJustification(Justification);
        SetPolishTextSize(Text, FontSize);
        AddPolishWidget(Canvas, Text, Position, Size, ZOrder);
        return Text;
    }

    UBorder* AddPolishBar(
        UWidgetTree* Tree,
        UCanvasPanel* Canvas,
        const FName Name,
        const FVector2D Position,
        const FVector2D Size,
        const FLinearColor& Color,
        const int32 ZOrder,
        const float Rotation = 0.0f,
        const FVector2D Alignment = FVector2D::ZeroVector)
    {
        if (!Tree || !Canvas) return nullptr;
        UBorder* Bar = Tree->ConstructWidget<UBorder>(UBorder::StaticClass(), Name);
        Bar->SetBrushColor(Color);
        Bar->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
        Bar->SetRenderTransformAngle(Rotation);
        AddPolishWidget(Canvas, Bar, Position, Size, ZOrder, Alignment);
        return Bar;
    }

    FString ResolvePolishIconPath(const TCHAR* FileName)
    {
        return FPaths::ConvertRelativePathToFull(FPaths::Combine(
            FPaths::ProjectContentDir(), TEXT("UI/TacticalMap/Icons"), FileName));
    }

    void AddMapPinIcon(
        UWidgetTree* Tree,
        UCanvasPanel* Canvas,
        const FName Name,
        const FVector2D Position)
    {
        if (!Tree || !Canvas) return;
        const FString IconPath = ResolvePolishIconPath(TEXT("map-pin.svg"));
        if (!IFileManager::Get().FileExists(*IconPath)) return;

        UImage* Icon = Tree->ConstructWidget<UImage>(UImage::StaticClass(), Name);
        FSlateVectorImageBrush VectorBrush(
            IconPath,
            FVector2D(13.0f, 13.0f),
            FLinearColor(0.82f, 0.87f, 0.89f, 0.95f),
            ESlateBrushTileType::NoTile);
        Icon->SetBrush(VectorBrush);
        Icon->SetVisibility(ESlateVisibility::HitTestInvisible);
        AddPolishWidget(Canvas, Icon, Position, FVector2D(13.0f, 13.0f), 12, FVector2D(0.5f, 0.5f));
    }
}

void UOCTacticalMapWidget::ApplyProductionPolish()
{
    if (!WidgetTree || !MapCanvas || !MapContentCanvas) return;

    // The grid remains useful for callouts, but it should sit behind roads, landmarks and objectives.
    for (int32 Index = 0; Index <= 10; ++Index)
    {
        if (UBorder* Vertical = Cast<UBorder>(WidgetTree->FindWidget(
            FName(*FString::Printf(TEXT("MapGridV_%02d"), Index)))))
            Vertical->SetBrushColor(PolishGrid);

        if (UBorder* Horizontal = Cast<UBorder>(WidgetTree->FindWidget(
            FName(*FString::Printf(TEXT("MapGridH_%02d"), Index)))))
            Horizontal->SetBrushColor(PolishGrid);

        if (Index < 10)
        {
            if (UTextBlock* Column = Cast<UTextBlock>(WidgetTree->FindWidget(
                FName(*FString::Printf(TEXT("MapGridCol_%02d"), Index)))))
            {
                Column->SetColorAndOpacity(FSlateColor(PolishGridText));
                SetPolishTextSize(Column, 10);
            }
            if (UTextBlock* Row = Cast<UTextBlock>(WidgetTree->FindWidget(
                FName(*FString::Printf(TEXT("MapGridRow_%02d"), Index)))))
            {
                Row->SetColorAndOpacity(FSlateColor(PolishGridText));
                SetPolishTextSize(Row, 10);
            }
        }
    }

    // Player gets the strongest friendly read on the map without turning into a glowing billboard.
    if (PlayerMarker)
    {
        PlayerMarker->SetColorAndOpacity(FSlateColor(PolishPlayer));
        SetPolishTextSize(PlayerMarker, 24);
        PlayerMarker->SetShadowOffset(FVector2D(1.0f, 1.0f));
        PlayerMarker->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.90f));
    }

    // Replace the debug-ish sector-width readout with a proper overview scale bar.
    if (UTextBlock* LegacyScale = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("TacticalMapScale"))))
        LegacyScale->SetVisibility(ESlateVisibility::Collapsed);

    const float WidthMeters = FMath::Max((Projection.WorldMax.X - Projection.WorldMin.X) / 100.0f, 1.0f);
    const float BarMeters = WidthMeters <= 1800.0f ? 250.0f : 500.0f;
    const float BarPixels = FMath::Clamp((BarMeters / WidthMeters) * PolishMapWidth, 120.0f, 270.0f);
    const float BarLeft = PolishMapWidth - BarPixels - 28.0f;
    const float BarTop = PolishMapHeight - 31.0f;

    AddPolishBar(WidgetTree, MapCanvas, TEXT("TacticalScaleBarMain"),
        FVector2D(BarLeft, BarTop), FVector2D(BarPixels, 2.0f), PolishText, 31);
    AddPolishBar(WidgetTree, MapCanvas, TEXT("TacticalScaleBarTick0"),
        FVector2D(BarLeft, BarTop - 5.0f), FVector2D(2.0f, 12.0f), PolishText, 31);
    AddPolishBar(WidgetTree, MapCanvas, TEXT("TacticalScaleBarTickHalf"),
        FVector2D(BarLeft + BarPixels * 0.5f, BarTop - 4.0f), FVector2D(1.0f, 10.0f), PolishMuted, 31);
    AddPolishBar(WidgetTree, MapCanvas, TEXT("TacticalScaleBarTickEnd"),
        FVector2D(BarLeft + BarPixels - 2.0f, BarTop - 5.0f), FVector2D(2.0f, 12.0f), PolishText, 31);

    AddPolishText(WidgetTree, MapCanvas, TEXT("TacticalScaleBarZero"), TEXT("0"),
        FVector2D(BarLeft - 2.0f, BarTop - 22.0f), FVector2D(28.0f, 16.0f), 9, PolishMuted, 31);
    AddPolishText(WidgetTree, MapCanvas, TEXT("TacticalScaleBarHalf"),
        FString::Printf(TEXT("%.0f"), BarMeters * 0.5f),
        FVector2D(BarLeft + BarPixels * 0.5f - 18.0f, BarTop - 22.0f), FVector2D(40.0f, 16.0f),
        9, PolishMuted, 31, ETextJustify::Center);
    AddPolishText(WidgetTree, MapCanvas, TEXT("TacticalScaleBarEnd"),
        FString::Printf(TEXT("%.0f м"), BarMeters),
        FVector2D(BarLeft + BarPixels - 48.0f, BarTop - 22.0f), FVector2D(52.0f, 16.0f),
        9, PolishText, 31, ETextJustify::Right);

    // The old bullet POI anchors are replaced with proper pin icons; labels stay where they are.
    for (int32 ChildIndex = 0; ChildIndex < MapContentCanvas->GetChildrenCount(); ++ChildIndex)
    {
        UWidget* Child = MapContentCanvas->GetChildAt(ChildIndex);
        const UCanvasPanelSlot* CanvasSlot = Child ? Cast<UCanvasPanelSlot>(Child->Slot) : nullptr;
        if (CanvasSlot && CanvasSlot->GetZOrder() == 12)
        {
            if (UTextBlock* LegacyDot = Cast<UTextBlock>(Child))
                LegacyDot->SetVisibility(ESlateVisibility::Collapsed);
        }
    }

    const FOCGeoReferencePoint SilpoRef = FOCGeoReference::Silpo();
    AddMapPinIcon(WidgetTree, MapContentCanvas, TEXT("MapPOIIconMuseum"),
        WorldToMap(ResolveSectorWorldLocation(AOCWorldSectorOster::MuseumAnchor())));
    AddMapPinIcon(WidgetTree, MapContentCanvas, TEXT("MapPOIIconStadium"),
        WorldToMap(ResolveSectorWorldLocation(AOCWorldSectorOster::StadiumAnchor())));
    AddMapPinIcon(WidgetTree, MapContentCanvas, TEXT("MapPOIIconPark"),
        WorldToMap(ResolveSectorWorldLocation(AOCWorldSectorOster::ParkAnchor())));
    AddMapPinIcon(WidgetTree, MapContentCanvas, TEXT("MapPOIIconCenter"),
        WorldToMap(ResolveSectorWorldLocation(AOCWorldSectorOster::FormerCityAdministrationAnchor())));
    AddMapPinIcon(WidgetTree, MapContentCanvas, TEXT("MapPOIIconSilpo"),
        WorldToMap(ResolveSectorWorldLocation(
            FOCGeoReference::ToLocalCm(SilpoRef.Latitude, SilpoRef.Longitude, 0.0f))));

    // Objective backplates give A/B/C a stable visual target while the existing replicated text
    // continues to carry contested/owner/progress state above them.
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AOCCapturePoint> It(World); It; ++It)
        {
            AOCCapturePoint* Point = *It;
            if (!IsValid(Point) || Point->GetPointId().IsNone()) continue;

            const FString PointName = Point->GetPointId().ToString();
            const FVector2D PointPosition = WorldToMap(Point->GetActorLocation());
            const FName OuterName(*FString::Printf(TEXT("ObjectiveBackplateOuter_%s"), *PointName));
            const FName InnerName(*FString::Printf(TEXT("ObjectiveBackplateInner_%s"), *PointName));

            AddPolishBar(WidgetTree, MapContentCanvas, OuterName,
                PointPosition, FVector2D(34.0f, 34.0f),
                FLinearColor(0.010f, 0.016f, 0.020f, 0.96f), 20, 45.0f, FVector2D(0.5f, 0.5f));
            AddPolishBar(WidgetTree, MapContentCanvas, InnerName,
                PointPosition, FVector2D(27.0f, 27.0f),
                FLinearColor(PolishAmber.R, PolishAmber.G, PolishAmber.B, 0.20f), 21, 45.0f, FVector2D(0.5f, 0.5f));
        }
    }

    // Small live-state chip reinforces that markers are driven by current replicated world state.
    if (UCanvasPanel* Root = Cast<UCanvasPanel>(WidgetTree->FindWidget(TEXT("TacticalMapRoot"))))
    {
        AddPolishBar(WidgetTree, Root, TEXT("TacticalLiveDot"),
            FVector2D(1265.0f, 56.0f), FVector2D(6.0f, 6.0f), PolishLive, 7);
        AddPolishText(WidgetTree, Root, TEXT("TacticalLiveText"), TEXT("LIVE · WORLD SYNC"),
            FVector2D(1278.0f, 49.0f), FVector2D(220.0f, 18.0f), 9, PolishMuted, 7);
    }

    UE_LOG(LogTemp, Display,
        TEXT("Tactical Map production polish installed: quiet grid, hierarchy, scale bar, POI pins and objective backplates."));
}
