#include "OCR13TacticalMapWidget.h"

#include "OCCapturePoint.h"
#include "OCWorldSectorOster.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"

namespace
{
    constexpr float CompactMinX = -70000.0f;
    constexpr float CompactMaxX =  25000.0f;
    constexpr float CompactMinY = -25000.0f;
    constexpr float CompactMaxY =  50000.0f;
    constexpr int32 MaxRoadSegmentsOnMap = 96;

    constexpr float MapPadLeft = 42.0f;
    constexpr float MapPadRight = 42.0f;
    constexpr float MapPadTop = 82.0f;
    constexpr float MapPadBottom = 48.0f;

    UInstancedStaticMeshComponent* FindISM(AActor* Actor, const FName Name)
    {
        if (!Actor) return nullptr;
        TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
        Actor->GetComponents(Components);
        for (UInstancedStaticMeshComponent* Component : Components)
        {
            if (Component && Component->GetFName() == Name) return Component;
        }
        return nullptr;
    }

    void PlaceCanvasWidget(UWidget* Widget, const FVector2D& Position, const FVector2D& Size, const int32 ZOrder)
    {
        if (!Widget) return;
        if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Widget->Slot))
        {
            CanvasSlot->SetAutoSize(false);
            CanvasSlot->SetPosition(Position);
            CanvasSlot->SetSize(Size);
            CanvasSlot->SetZOrder(ZOrder);
        }
    }
}

TSharedRef<SWidget> UOCR13TacticalMapWidget::RebuildWidget()
{
    if (!WidgetTree->RootWidget) BuildWidgetTree();
    return Super::RebuildWidget();
}

void UOCR13TacticalMapWidget::BuildWidgetTree()
{
    RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("R13_TacticalMapRoot"));
    WidgetTree->RootWidget = RootCanvas;

    Backdrop = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("R13_TacticalMapBackdrop"));
    Backdrop->SetBrushColor(FLinearColor(0.018f, 0.024f, 0.030f, 0.965f));
    RootCanvas->AddChild(Backdrop);
    if (UCanvasPanelSlot* BackdropCanvasSlot = Cast<UCanvasPanelSlot>(Backdrop->Slot))
    {
        BackdropCanvasSlot->SetAnchors(FAnchors(0.055f, 0.045f, 0.945f, 0.955f));
        BackdropCanvasSlot->SetOffsets(FMargin(0.0f));
        BackdropCanvasSlot->SetZOrder(0);
    }

    MapCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("R13_TacticalMapCanvas"));
    Backdrop->SetContent(MapCanvas);

    UTextBlock* Title = MakeMarkerText(TEXT("ТАКТИЧНА КАРТА  ·  ОСТЕР"), 28);
    MapCanvas->AddChild(Title);
    PlaceCanvasWidget(Title, FVector2D(34.0f, 20.0f), FVector2D(580.0f, 42.0f), 20);

    UTextBlock* Help = MakeMarkerText(TEXT("M / Ь  ЗАКРИТИ     ▲  ВИ     A/B/C  ТОЧКИ"), 14);
    Help->SetColorAndOpacity(FSlateColor(FLinearColor(0.68f, 0.71f, 0.72f, 1.0f)));
    MapCanvas->AddChild(Help);
    if (UCanvasPanelSlot* HelpCanvasSlot = Cast<UCanvasPanelSlot>(Help->Slot))
    {
        HelpCanvasSlot->SetAnchors(FAnchors(0.0f, 1.0f, 0.0f, 1.0f));
        HelpCanvasSlot->SetAlignment(FVector2D(0.0f, 1.0f));
        HelpCanvasSlot->SetPosition(FVector2D(34.0f, -18.0f));
        HelpCanvasSlot->SetSize(FVector2D(560.0f, 28.0f));
        HelpCanvasSlot->SetZOrder(20);
    }

    PositionText = MakeMarkerText(TEXT("ПОЗИЦІЯ"), 14);
    PositionText->SetJustification(ETextJustify::Right);
    MapCanvas->AddChild(PositionText);
    if (UCanvasPanelSlot* PositionCanvasSlot = Cast<UCanvasPanelSlot>(PositionText->Slot))
    {
        PositionCanvasSlot->SetAnchors(FAnchors(1.0f, 0.0f, 1.0f, 0.0f));
        PositionCanvasSlot->SetAlignment(FVector2D(1.0f, 0.0f));
        PositionCanvasSlot->SetPosition(FVector2D(-34.0f, 24.0f));
        PositionCanvasSlot->SetSize(FVector2D(480.0f, 34.0f));
        PositionCanvasSlot->SetZOrder(20);
    }

    PlayerMarker = MakeMarkerText(TEXT("▲"), 25);
    PlayerMarker->SetColorAndOpacity(FSlateColor(FLinearColor(0.96f, 0.86f, 0.28f, 1.0f)));
    PlayerMarker->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
    MapCanvas->AddChild(PlayerMarker);
    PlaceCanvasWidget(PlayerMarker, FVector2D::ZeroVector, FVector2D(32.0f, 32.0f), 30);

    const TCHAR* ObjectiveLabels[] = { TEXT("A"), TEXT("B"), TEXT("C") };
    for (const TCHAR* Label : ObjectiveLabels)
    {
        UTextBlock* Marker = MakeMarkerText(Label, 20);
        Marker->SetColorAndOpacity(FSlateColor(FLinearColor(0.95f, 0.95f, 0.93f, 1.0f)));
        MapCanvas->AddChild(Marker);
        PlaceCanvasWidget(Marker, FVector2D::ZeroVector, FVector2D(28.0f, 28.0f), 25);
        ObjectiveWidgets.Add(Marker);
    }

    const TCHAR* LandmarkLabels[] = { TEXT("МУЗЕЙ"), TEXT("ПАРК"), TEXT("КОЛЕДЖ"), TEXT("СТАДІОН") };
    LandmarkLocations = {
        AOCWorldSectorOster::MuseumAnchor(),
        AOCWorldSectorOster::ParkAnchor(),
        AOCWorldSectorOster::CollegeAnchor(),
        AOCWorldSectorOster::StadiumAnchor(),
    };
    for (const TCHAR* Label : LandmarkLabels)
    {
        UTextBlock* Marker = MakeMarkerText(Label, 12);
        Marker->SetColorAndOpacity(FSlateColor(FLinearColor(0.62f, 0.69f, 0.69f, 1.0f)));
        MapCanvas->AddChild(Marker);
        PlaceCanvasWidget(Marker, FVector2D::ZeroVector, FVector2D(110.0f, 22.0f), 15);
        LandmarkWidgets.Add(Marker);
    }

    SetVisibility(ESlateVisibility::HitTestInvisible);
}

UTextBlock* UOCR13TacticalMapWidget::MakeMarkerText(const FString& Label, const int32 FontSize)
{
    UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    if (!Text) return nullptr;
    Text->SetText(FText::FromString(Label));
    FSlateFontInfo Font = Text->GetFont();
    Font.Size = FontSize;
    Text->SetFont(Font);
    Text->SetColorAndOpacity(FSlateColor(FLinearColor(0.92f, 0.93f, 0.92f, 1.0f)));
    return Text;
}

void UOCR13TacticalMapWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    if (!MapCanvas) return;

    const FVector2D MapSize = MapCanvas->GetCachedGeometry().GetLocalSize();
    if (MapSize.X < 300.0f || MapSize.Y < 220.0f) return;

    if (!bRoadsBuilt) BuildRoadSchematic();
    UpdateRoadSchematic(MapSize);
    UpdatePlayerMarker(MapSize);
    UpdateObjectiveMarkers(MapSize);
    UpdateLandmarkMarkers(MapSize);
}

void UOCR13TacticalMapWidget::BuildRoadSchematic()
{
    UWorld* World = GetWorld();
    if (!World || !MapCanvas) return;

    AOCWorldSectorOster* Sector = nullptr;
    for (TActorIterator<AOCWorldSectorOster> It(World); It; ++It)
    {
        Sector = *It;
        if (Sector) break;
    }
    if (!Sector) return;

    UInstancedStaticMeshComponent* Roads = FindISM(Sector, TEXT("Roads"));
    if (!Roads) return;

    const int32 Count = FMath::Min(Roads->GetInstanceCount(), MaxRoadSegmentsOnMap);
    for (int32 Index = 0; Index < Count; ++Index)
    {
        FTransform Transform;
        if (!Roads->GetInstanceTransform(Index, Transform, true)) continue;

        UBorder* Road = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
        if (!Road) continue;
        Road->SetBrushColor(FLinearColor(0.30f, 0.34f, 0.35f, 0.72f));
        Road->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
        MapCanvas->AddChild(Road);
        PlaceCanvasWidget(Road, FVector2D::ZeroVector, FVector2D(4.0f, 4.0f), 2);
        RoadWidgets.Add(Road);
        RoadTransforms.Add(Transform);
    }

    bRoadsBuilt = RoadWidgets.Num() > 0;
}

FVector2D UOCR13TacticalMapWidget::WorldToMap(const FVector& WorldLocation, const FVector2D& MapSize) const
{
    const float UsableX = FMath::Max(1.0f, MapSize.X - MapPadLeft - MapPadRight);
    const float UsableY = FMath::Max(1.0f, MapSize.Y - MapPadTop - MapPadBottom);
    const float X01 = FMath::Clamp((WorldLocation.X - CompactMinX) / (CompactMaxX - CompactMinX), 0.0f, 1.0f);
    const float Y01 = FMath::Clamp((WorldLocation.Y - CompactMinY) / (CompactMaxY - CompactMinY), 0.0f, 1.0f);
    return FVector2D(MapPadLeft + X01 * UsableX, MapPadTop + (1.0f - Y01) * UsableY);
}

void UOCR13TacticalMapWidget::UpdateRoadSchematic(const FVector2D& MapSize)
{
    const float UsableX = FMath::Max(1.0f, MapSize.X - MapPadLeft - MapPadRight);
    const float UsableY = FMath::Max(1.0f, MapSize.Y - MapPadTop - MapPadBottom);
    const float ScreenPerWorldX = UsableX / (CompactMaxX - CompactMinX);
    const float ScreenPerWorldY = UsableY / (CompactMaxY - CompactMinY);

    const int32 Count = FMath::Min(RoadWidgets.Num(), RoadTransforms.Num());
    for (int32 Index = 0; Index < Count; ++Index)
    {
        UBorder* Road = RoadWidgets[Index];
        if (!Road) continue;
        const FTransform& Transform = RoadTransforms[Index];
        const FVector Scale = Transform.GetScale3D().GetAbs();
        const FVector2D Size(
            FMath::Max(2.0f, Scale.X * 100.0f * ScreenPerWorldX),
            FMath::Max(2.0f, Scale.Y * 100.0f * ScreenPerWorldY));
        const FVector2D Center = WorldToMap(Transform.GetLocation(), MapSize);
        PlaceCanvasWidget(Road, Center - Size * 0.5f, Size, 2);
        Road->SetRenderTransformAngle(-Transform.Rotator().Yaw);
    }
}

void UOCR13TacticalMapWidget::UpdatePlayerMarker(const FVector2D& MapSize)
{
    APlayerController* PC = GetOwningPlayer();
    APawn* Pawn = PC ? PC->GetPawn() : nullptr;
    if (!Pawn || !PlayerMarker)
    {
        if (PlayerMarker) PlayerMarker->SetVisibility(ESlateVisibility::Collapsed);
        return;
    }

    PlayerMarker->SetVisibility(ESlateVisibility::HitTestInvisible);
    const FVector Location = Pawn->GetActorLocation();
    const FVector2D Position = WorldToMap(Location, MapSize);
    PlaceCanvasWidget(PlayerMarker, Position - FVector2D(16.0f, 16.0f), FVector2D(32.0f, 32.0f), 30);
    PlayerMarker->SetRenderTransformAngle(90.0f - Pawn->GetActorRotation().Yaw);

    if (PositionText)
    {
        PositionText->SetText(FText::FromString(FString::Printf(
            TEXT("ВИ  ·  X %.0f м   Y %.0f м"), Location.X / 100.0f, Location.Y / 100.0f)));
    }
}

void UOCR13TacticalMapWidget::UpdateObjectiveMarkers(const FVector2D& MapSize)
{
    for (UTextBlock* Marker : ObjectiveWidgets)
    {
        if (Marker) Marker->SetVisibility(ESlateVisibility::Collapsed);
    }

    UWorld* World = GetWorld();
    if (!World) return;
    for (TActorIterator<AOCCapturePoint> It(World); It; ++It)
    {
        AOCCapturePoint* Point = *It;
        if (!Point) continue;
        const FString Id = Point->GetPointId().ToString().ToUpper();
        int32 MarkerIndex = INDEX_NONE;
        if (Id == TEXT("A")) MarkerIndex = 0;
        else if (Id == TEXT("B")) MarkerIndex = 1;
        else if (Id == TEXT("C")) MarkerIndex = 2;
        if (!ObjectiveWidgets.IsValidIndex(MarkerIndex) || !ObjectiveWidgets[MarkerIndex]) continue;

        UTextBlock* Marker = ObjectiveWidgets[MarkerIndex];
        const FVector2D Position = WorldToMap(Point->GetActorLocation(), MapSize);
        PlaceCanvasWidget(Marker, Position - FVector2D(14.0f, 14.0f), FVector2D(28.0f, 28.0f), 25);
        Marker->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
}

void UOCR13TacticalMapWidget::UpdateLandmarkMarkers(const FVector2D& MapSize)
{
    const int32 Count = FMath::Min(LandmarkWidgets.Num(), LandmarkLocations.Num());
    for (int32 Index = 0; Index < Count; ++Index)
    {
        UTextBlock* Marker = LandmarkWidgets[Index];
        if (!Marker) continue;
        const FVector& Location = LandmarkLocations[Index];
        const bool bInside = Location.X >= CompactMinX && Location.X <= CompactMaxX &&
            Location.Y >= CompactMinY && Location.Y <= CompactMaxY;
        Marker->SetVisibility(bInside ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
        if (!bInside) continue;
        const FVector2D Position = WorldToMap(Location, MapSize);
        PlaceCanvasWidget(Marker, Position + FVector2D(8.0f, -10.0f), FVector2D(110.0f, 22.0f), 15);
    }
}
