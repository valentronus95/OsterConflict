#include "OCTacticalMapSubsystem.h"

#include "OCGeoReference.h"
#include "OCWorldSectorOster.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"

namespace
{
    constexpr float TacticalMapWidth = 1248.0f;
    constexpr float TacticalMapHeight = 702.0f;

    // Pass 44: user-approved central Oster battlefield. These are sector-local centimetres and are
    // intentionally identical to OCCentralPlayableAreaSubsystem. The tactical map must never grow
    // beyond these limits merely because an old procedural component or debug anchor exists elsewhere.
    constexpr float Pass44PlayableMinX = -78000.0f;
    constexpr float Pass44PlayableMaxX =  18000.0f;
    constexpr float Pass44PlayableMinY = -12000.0f;
    constexpr float Pass44PlayableMaxY =  82000.0f;

    const FLinearColor TacticalBackdrop(0.008f, 0.012f, 0.016f, 1.0f);
    const FLinearColor TacticalPanel(0.018f, 0.025f, 0.031f, 0.985f);
    const FLinearColor TacticalField(0.020f, 0.030f, 0.034f, 1.0f);
    const FLinearColor TacticalFrame(0.22f, 0.27f, 0.29f, 1.0f);
    const FLinearColor TacticalRoadPrimary(0.46f, 0.52f, 0.55f, 0.96f);
    const FLinearColor TacticalRoadSecondary(0.30f, 0.35f, 0.37f, 0.82f);
    const FLinearColor TacticalSidewalk(0.18f, 0.22f, 0.24f, 0.36f);
    const FLinearColor TacticalBuildingOutline(0.28f, 0.33f, 0.35f, 0.78f);
    const FLinearColor TacticalBuildingFill(0.045f, 0.058f, 0.068f, 0.96f);
    const FLinearColor TacticalResidentialOutline(0.19f, 0.23f, 0.25f, 0.56f);
    const FLinearColor TacticalResidentialFill(0.030f, 0.041f, 0.048f, 0.92f);
    const FLinearColor TacticalLandmarkOutline(0.92f, 0.58f, 0.18f, 0.78f);
    const FLinearColor TacticalLandmarkFill(0.12f, 0.10f, 0.060f, 0.98f);
    const FLinearColor TacticalPark(0.055f, 0.105f, 0.088f, 0.82f);
    const FLinearColor TacticalStadium(0.050f, 0.085f, 0.100f, 0.86f);
    const FLinearColor TacticalAmber(0.95f, 0.55f, 0.12f, 1.0f);
    const FLinearColor TacticalText(0.86f, 0.90f, 0.92f, 1.0f);
    const FLinearColor TacticalMuted(0.50f, 0.57f, 0.61f, 1.0f);

    void SetTextFontSize(UTextBlock* Text, const int32 Size)
    {
        if (!Text) return;
        FSlateFontInfo Font = Text->GetFont();
        Font.Size = Size;
        Text->SetFont(Font);
    }

    UCanvasPanelSlot* GetCanvasSlot(UWidget* Widget)
    {
        return Widget ? Cast<UCanvasPanelSlot>(Widget->Slot) : nullptr;
    }

    void StyleBorder(UWidgetTree* Tree, const FName Name, const FLinearColor& Color)
    {
        if (!Tree) return;
        if (UBorder* Border = Cast<UBorder>(Tree->FindWidget(Name))) Border->SetBrushColor(Color);
    }

    void StyleText(UWidgetTree* Tree, const FName Name, const int32 FontSize, const FLinearColor& Color)
    {
        if (!Tree) return;
        if (UTextBlock* Text = Cast<UTextBlock>(Tree->FindWidget(Name)))
        {
            SetTextFontSize(Text, FontSize);
            Text->SetColorAndOpacity(FSlateColor(Color));
        }
    }
}

void UOCTacticalMapWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (bProductionVisualLayerBuilt) return;
    bProductionVisualLayerBuilt = true;

    RestyleStaticTacticalChrome();
    BuildProductionVisualLayer();
    InstallTacticalIconography();
    ApplyProductionPolish();
}

bool UOCTacticalMapWidget::ReframeProjectionForCentralOster()
{
    AOCWorldSectorOster* Sector = WorldSector.Get();
    if (!IsValid(Sector)) return false;

    // Pass 44 replaces the old component auto-fit + 800 m half-width minimum. That historical clamp
    // forced a ~1.6 km map even after the user explicitly reduced the battlefield. Build the world
    // projection from the four corners of the authoritative compact playable area instead.
    FBox2D CompactWorldBounds(ForceInit);
    const FTransform SectorTransform = Sector->GetActorTransform();
    const FVector LocalCorners[] =
    {
        FVector(Pass44PlayableMinX, Pass44PlayableMinY, 0.0f),
        FVector(Pass44PlayableMinX, Pass44PlayableMaxY, 0.0f),
        FVector(Pass44PlayableMaxX, Pass44PlayableMinY, 0.0f),
        FVector(Pass44PlayableMaxX, Pass44PlayableMaxY, 0.0f),
    };
    for (const FVector& LocalCorner : LocalCorners)
    {
        const FVector WorldCorner = SectorTransform.TransformPosition(LocalCorner);
        CompactWorldBounds += FVector2D(WorldCorner.X, WorldCorner.Y);
    }

    if (!CompactWorldBounds.bIsValid) return false;

    Projection.WorldMin = CompactWorldBounds.Min;
    Projection.WorldMax = CompactWorldBounds.Max;
    Projection.bInvertX = false;
    Projection.bInvertY = true;

    MapZoom = 1.0f;
    MapPan = FVector2D::ZeroVector;
    ApplyMapViewTransform();

    const float WidthMeters = (Projection.WorldMax.X - Projection.WorldMin.X) / 100.0f;
    const float HeightMeters = (Projection.WorldMax.Y - Projection.WorldMin.Y) / 100.0f;
    if (UTextBlock* ScaleText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("TacticalMapScale"))))
    {
        ScaleText->SetText(FText::FromString(FString::Printf(TEXT("МАСШТАБ · %.0f × %.0f м"), WidthMeters, HeightMeters)));
    }

    UE_LOG(LogTemp, Display,
        TEXT("PASS44_TACTICAL_MAP_COMPACT_BOUNDS_READY bounds_m=%.0fx%.0f x_m=[-780,180] y_m=[-120,820] auto_component_fit=0 old_min_halfwidth_800m=0 reference=oster_central_playable_area_20260824"),
        WidthMeters,
        HeightMeters);
    return Projection.IsValid();
}

void UOCTacticalMapWidget::RestyleStaticTacticalChrome()
{
    if (!WidgetTree) return;

    StyleBorder(WidgetTree, TEXT("TacticalMapBackdrop"), TacticalBackdrop);
    StyleBorder(WidgetTree, TEXT("TacticalMapLegendPanel"), TacticalPanel);
    StyleBorder(WidgetTree, TEXT("TacticalMapFrame"), TacticalFrame);
    StyleBorder(WidgetTree, TEXT("TacticalMapField"), TacticalField);
    StyleBorder(WidgetTree, TEXT("TacticalMapFooterPanel"), TacticalPanel);

    if (UImage* RawCapture = Cast<UImage>(WidgetTree->FindWidget(TEXT("TacticalMapWorldCapture"))))
    {
        // The raw scene capture remains available to the minimap subsystem but is not the visual basis of the M map.
        RawCapture->SetVisibility(ESlateVisibility::Collapsed);
    }
    if (UBorder* RawFilter = Cast<UBorder>(WidgetTree->FindWidget(TEXT("TacticalMapWorldFilter"))))
        RawFilter->SetVisibility(ESlateVisibility::Collapsed);

    StyleText(WidgetTree, TEXT("TacticalMapTitle"), 27, TacticalText);
    StyleText(WidgetTree, TEXT("TacticalMapGameTitle"), 13, TacticalAmber);
    StyleText(WidgetTree, TEXT("TacticalMapSector"), 11, TacticalMuted);
    StyleText(WidgetTree, TEXT("LegendTitle"), 14, TacticalText);
    StyleText(WidgetTree, TEXT("LegendPlayer"), 11, FLinearColor(0.46f, 0.82f, 0.30f, 1.0f));
    StyleText(WidgetTree, TEXT("LegendSquad"), 11, FLinearColor(0.20f, 0.56f, 0.96f, 1.0f));
    StyleText(WidgetTree, TEXT("LegendVehicle"), 11, FLinearColor(0.20f, 0.56f, 0.96f, 1.0f));
    StyleText(WidgetTree, TEXT("LegendObjective"), 11, TacticalAmber);
    StyleText(WidgetTree, TEXT("LegendPOI"), 11, TacticalText);

    if (UTextBlock* Sector = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("TacticalMapSector"))))
        Sector->SetText(FText::FromString(TEXT("ОСТЕР · ТАКТИЧНА СІТКА · NORTH-UP")));

    // Coordinates were useful during source bring-up, but they read as debug UI in the production map.
    if (UTextBlock* Coordinates = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("TacticalMapPlayerCoordinates"))))
        Coordinates->SetVisibility(ESlateVisibility::Collapsed);

    if (UTextBlock* HintClose = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("TacticalMapHintClose"))))
        HintClose->SetText(FText::FromString(TEXT("M  ЗАКРИТИ")));
    if (UTextBlock* HintZoom = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("TacticalMapHintZoom"))))
        HintZoom->SetText(FText::FromString(TEXT("КОЛЕСО  МАСШТАБ")));
    if (UTextBlock* HintPan = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("TacticalMapHintPan"))))
        HintPan->SetText(FText::FromString(TEXT("ЛКМ  ПЕРЕМІЩЕННЯ")));
    if (UTextBlock* HintPing = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("TacticalMapHintPing"))))
        HintPing->SetText(FText::FromString(TEXT("ПКМ  ПОСТАВИТИ МІТКУ")));

    for (const FName HintName : { FName(TEXT("TacticalMapHintClose")), FName(TEXT("TacticalMapHintZoom")),
                                  FName(TEXT("TacticalMapHintPan")), FName(TEXT("TacticalMapHintPing")) })
        StyleText(WidgetTree, HintName, 10, TacticalMuted);

    if (UCanvasPanel* Root = Cast<UCanvasPanel>(WidgetTree->FindWidget(TEXT("TacticalMapRoot"))))
    {
        UBorder* Accent = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("TacticalMapProductionAccent"));
        Accent->SetBrushColor(FLinearColor(0.95f, 0.55f, 0.12f, 0.92f));
        if (UCanvasPanelSlot* AccentCanvasSlot = Root->AddChildToCanvas(Accent))
        {
            AccentCanvasSlot->SetPosition(FVector2D(315.0f, 84.0f));
            AccentCanvasSlot->SetSize(FVector2D(1248.0f, 2.0f));
            AccentCanvasSlot->SetZOrder(6);
        }
    }
}

void UOCTacticalMapWidget::BuildProductionVisualLayer()
{
    if (!WidgetTree || !MapContentCanvas) return;
    AOCWorldSectorOster* Sector = WorldSector.Get();
    if (!IsValid(Sector)) return;

    ReframeProjectionForCentralOster();

    // Remove the original POI label widgets. They were positioned before the production-core projection was applied.
    for (int32 ChildIndex = MapContentCanvas->GetChildrenCount() - 1; ChildIndex >= 0; --ChildIndex)
    {
        UWidget* Child = MapContentCanvas->GetChildAt(ChildIndex);
        if (const UCanvasPanelSlot* ChildCanvasSlot = GetCanvasSlot(Child))
        {
            const int32 Z = ChildCanvasSlot->GetZOrder();
            if (Z == 12 || Z == 13) MapContentCanvas->RemoveChildAt(ChildIndex);
        }
    }

    const float ProjectionWidth = FMath::Max(Projection.WorldMax.X - Projection.WorldMin.X, 1.0f);
    const float ProjectionHeight = FMath::Max(Projection.WorldMax.Y - Projection.WorldMin.Y, 1.0f);

    auto AddRect = [this, ProjectionWidth, ProjectionHeight](
        const FTransform& Transform,
        const FLinearColor& Fill,
        const FLinearColor& Outline,
        const int32 ZOrder,
        const bool bOutlined,
        const float MinPixelThickness)
    {
        const FVector WorldLocation = Transform.GetLocation();
        const FVector2D MapCenter = WorldToMap(WorldLocation);
        if (MapCenter.X < -80.0f || MapCenter.Y < -80.0f ||
            MapCenter.X > TacticalMapWidth + 80.0f || MapCenter.Y > TacticalMapHeight + 80.0f)
            return;

        const FVector Scale = Transform.GetScale3D().GetAbs();
        FVector2D PixelSize(
            Scale.X * 100.0f / ProjectionWidth * TacticalMapWidth,
            Scale.Y * 100.0f / ProjectionHeight * TacticalMapHeight);
        PixelSize.X = FMath::Max(PixelSize.X, MinPixelThickness);
        PixelSize.Y = FMath::Max(PixelSize.Y, MinPixelThickness);

        const float MapAngle = -Transform.Rotator().Yaw;

        UBorder* Outer = WidgetTree->ConstructWidget<UBorder>();
        Outer->SetBrushColor(bOutlined ? Outline : Fill);
        Outer->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
        Outer->SetRenderTransformAngle(MapAngle);
        if (UCanvasPanelSlot* OuterCanvasSlot = MapContentCanvas->AddChildToCanvas(Outer))
        {
            OuterCanvasSlot->SetPosition(MapCenter);
            OuterCanvasSlot->SetSize(PixelSize);
            OuterCanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
            OuterCanvasSlot->SetZOrder(ZOrder);
        }

        if (!bOutlined || PixelSize.X <= 3.0f || PixelSize.Y <= 3.0f) return;

        UBorder* Inner = WidgetTree->ConstructWidget<UBorder>();
        Inner->SetBrushColor(Fill);
        Inner->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
        Inner->SetRenderTransformAngle(MapAngle);
        if (UCanvasPanelSlot* InnerCanvasSlot = MapContentCanvas->AddChildToCanvas(Inner))
        {
            InnerCanvasSlot->SetPosition(MapCenter);
            InnerCanvasSlot->SetSize(FVector2D(FMath::Max(1.0f, PixelSize.X - 2.0f), FMath::Max(1.0f, PixelSize.Y - 2.0f)));
            InnerCanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
            InnerCanvasSlot->SetZOrder(ZOrder);
        }
    };

    auto AddComponent = [&AddRect](
        const UInstancedStaticMeshComponent* Component,
        const FLinearColor& Fill,
        const FLinearColor& Outline,
        const int32 ZOrder,
        const bool bOutlined,
        const float MinPixelThickness,
        const int32 MaxInstances)
    {
        if (!IsValid(Component)) return;
        const int32 Count = FMath::Min(Component->GetInstanceCount(), MaxInstances);
        for (int32 InstanceIndex = 0; InstanceIndex < Count; ++InstanceIndex)
        {
            FTransform WorldTransform;
            if (Component->GetInstanceTransform(InstanceIndex, WorldTransform, true))
                AddRect(WorldTransform, Fill, Outline, ZOrder, bOutlined, MinPixelThickness);
        }
    };

    auto AddRoadComponent = [&AddRect, ProjectionWidth, ProjectionHeight](const UInstancedStaticMeshComponent* Component)
    {
        if (!IsValid(Component)) return;
        const int32 Count = FMath::Min(Component->GetInstanceCount(), 768);
        for (int32 InstanceIndex = 0; InstanceIndex < Count; ++InstanceIndex)
        {
            FTransform WorldTransform;
            if (!Component->GetInstanceTransform(InstanceIndex, WorldTransform, true)) continue;

            const FVector Scale = WorldTransform.GetScale3D().GetAbs();
            const FVector2D NaturalPixels(
                Scale.X * 100.0f / ProjectionWidth * TacticalMapWidth,
                Scale.Y * 100.0f / ProjectionHeight * TacticalMapHeight);
            const float NaturalThickness = FMath::Min(NaturalPixels.X, NaturalPixels.Y);
            const bool bPrimaryRoad = NaturalThickness >= 2.25f;
            AddRect(
                WorldTransform,
                bPrimaryRoad ? TacticalRoadPrimary : TacticalRoadSecondary,
                bPrimaryRoad ? TacticalRoadPrimary : TacticalRoadSecondary,
                2,
                false,
                bPrimaryRoad ? 3.0f : 1.45f);
        }
    };

    // Areas first, then transport hierarchy, then structures. Residential footprints remain real,
    // but are deliberately quieter at overview scale so roads/objectives win the first visual read.
    AddComponent(Sector->GetTacticalParkGeometry(), TacticalPark, TacticalPark, 1, false, 2.0f, 256);
    AddComponent(Sector->GetTacticalStadiumGeometry(), TacticalStadium, TacticalStadium, 1, false, 2.0f, 256);
    AddComponent(Sector->GetTacticalSidewalks(), TacticalSidewalk, TacticalSidewalk, 1, false, 0.8f, 768);
    AddRoadComponent(Sector->GetTacticalRoads());
    AddComponent(Sector->GetTacticalBuildings(), TacticalBuildingFill, TacticalBuildingOutline, 2, true, 1.8f, 768);
    AddComponent(Sector->GetTacticalResidentialRoofs(), TacticalResidentialFill, TacticalResidentialOutline, 2, true, 1.4f, 768);
    AddComponent(Sector->GetTacticalLandmarkBlocks(), TacticalLandmarkFill, TacticalLandmarkOutline, 2, true, 3.0f, 256);
    AddComponent(Sector->GetTacticalLandmarkRoofs(), TacticalLandmarkFill, TacticalLandmarkOutline, 2, true, 3.0f, 256);

    // Re-create the production POI chips against the new core projection.
    AddLandmarkMarker(TEXT("МУЗЕЙ"), ResolveSectorWorldLocation(AOCWorldSectorOster::MuseumAnchor()));
    AddLandmarkMarker(TEXT("СТАДІОН"), ResolveSectorWorldLocation(AOCWorldSectorOster::StadiumAnchor()));
    AddLandmarkMarker(TEXT("ПАРК"), ResolveSectorWorldLocation(AOCWorldSectorOster::ParkAnchor()));
    AddLandmarkMarker(TEXT("ЦЕНТР"), ResolveSectorWorldLocation(AOCWorldSectorOster::FormerCityAdministrationAnchor()));
    const FOCGeoReferencePoint SilpoRef = FOCGeoReference::Silpo();
    AddLandmarkMarker(TEXT("СІЛЬПО"), ResolveSectorWorldLocation(
        FOCGeoReference::ToLocalCm(SilpoRef.Latitude, SilpoRef.Longitude, 0.0f)));

    // The player marker was created with the old projection but NativeTick will immediately place it using this projection.
    ApplyMapViewTransform();

    UE_LOG(LogTemp, Display,
        TEXT("Tactical Map production vector layer built with primary/secondary road hierarchy and quiet residential detail."));
}
