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
    constexpr float TacticalMapAspect = TacticalMapWidth / TacticalMapHeight;

    const FLinearColor TacticalBackdrop(0.008f, 0.012f, 0.016f, 1.0f);
    const FLinearColor TacticalPanel(0.018f, 0.025f, 0.031f, 0.985f);
    const FLinearColor TacticalField(0.020f, 0.030f, 0.034f, 1.0f);
    const FLinearColor TacticalFrame(0.22f, 0.27f, 0.29f, 1.0f);
    const FLinearColor TacticalRoad(0.31f, 0.36f, 0.38f, 0.92f);
    const FLinearColor TacticalSidewalk(0.22f, 0.27f, 0.29f, 0.62f);
    const FLinearColor TacticalBuildingOutline(0.31f, 0.36f, 0.38f, 0.90f);
    const FLinearColor TacticalBuildingFill(0.050f, 0.066f, 0.076f, 0.98f);
    const FLinearColor TacticalLandmarkOutline(0.92f, 0.58f, 0.18f, 0.72f);
    const FLinearColor TacticalLandmarkFill(0.12f, 0.10f, 0.060f, 0.98f);
    const FLinearColor TacticalPark(0.055f, 0.105f, 0.088f, 0.88f);
    const FLinearColor TacticalStadium(0.050f, 0.085f, 0.100f, 0.90f);
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

    void AccumulateComponentBounds2D(const UInstancedStaticMeshComponent* Component, FBox2D& Bounds)
    {
        if (!IsValid(Component) || Component->GetInstanceCount() <= 0) return;
        const FBox WorldBounds = Component->Bounds.GetBox();
        if (!WorldBounds.IsValid) return;
        Bounds += FVector2D(WorldBounds.Min.X, WorldBounds.Min.Y);
        Bounds += FVector2D(WorldBounds.Max.X, WorldBounds.Max.Y);
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
}

bool UOCTacticalMapWidget::ReframeProjectionForCentralOster()
{
    AOCWorldSectorOster* Sector = WorldSector.Get();
    if (!IsValid(Sector)) return false;

    FBox2D CoreBounds(ForceInit);
    AccumulateComponentBounds2D(Sector->GetTacticalBuildings(), CoreBounds);
    AccumulateComponentBounds2D(Sector->GetTacticalResidentialRoofs(), CoreBounds);
    AccumulateComponentBounds2D(Sector->GetTacticalLandmarkBlocks(), CoreBounds);
    AccumulateComponentBounds2D(Sector->GetTacticalLandmarkRoofs(), CoreBounds);
    AccumulateComponentBounds2D(Sector->GetTacticalParkGeometry(), CoreBounds);
    AccumulateComponentBounds2D(Sector->GetTacticalStadiumGeometry(), CoreBounds);

    auto AddAnchor = [&CoreBounds, Sector](const FVector& SectorLocal)
    {
        const FVector World = Sector->GetActorTransform().TransformPosition(SectorLocal);
        CoreBounds += FVector2D(World.X, World.Y);
    };

    AddAnchor(AOCWorldSectorOster::MuseumAnchor());
    AddAnchor(AOCWorldSectorOster::StadiumAnchor());
    AddAnchor(AOCWorldSectorOster::ParkAnchor());
    AddAnchor(AOCWorldSectorOster::CollegeAnchor());
    AddAnchor(AOCWorldSectorOster::FormerCityAdministrationAnchor());

    const FOCGeoReferencePoint SilpoRef = FOCGeoReference::Silpo();
    AddAnchor(FOCGeoReference::ToLocalCm(SilpoRef.Latitude, SilpoRef.Longitude, 0.0f));

    if (!CoreBounds.bIsValid) return false;

    FVector2D Center = CoreBounds.GetCenter();
    FVector2D HalfSize = CoreBounds.GetExtent();

    // The M map opens on the actual playable city core, not the distant hydrography/debug extents.
    HalfSize += FVector2D(30000.0f, 26000.0f);
    HalfSize.X = FMath::Clamp(HalfSize.X, 80000.0f, 120000.0f);
    HalfSize.Y = FMath::Clamp(HalfSize.Y, 45000.0f, 67500.0f);

    if (HalfSize.X / HalfSize.Y < TacticalMapAspect)
        HalfSize.X = HalfSize.Y * TacticalMapAspect;
    else
        HalfSize.Y = HalfSize.X / TacticalMapAspect;

    HalfSize.X = FMath::Min(HalfSize.X, 120000.0f);
    HalfSize.Y = HalfSize.X / TacticalMapAspect;

    Projection.WorldMin = Center - HalfSize;
    Projection.WorldMax = Center + HalfSize;
    Projection.bInvertX = false;
    Projection.bInvertY = true;

    MapZoom = 1.0f;
    MapPan = FVector2D::ZeroVector;
    ApplyMapViewTransform();

    if (UTextBlock* ScaleText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("TacticalMapScale"))))
    {
        const float WidthMeters = (Projection.WorldMax.X - Projection.WorldMin.X) / 100.0f;
        ScaleText->SetText(FText::FromString(FString::Printf(TEXT("МАСШТАБ · %.0f м"), WidthMeters)));
    }

    UE_LOG(LogTemp, Display,
        TEXT("Tactical Map production framing: central Oster %.0fm x %.0fm."),
        (Projection.WorldMax.X - Projection.WorldMin.X) / 100.0f,
        (Projection.WorldMax.Y - Projection.WorldMin.Y) / 100.0f);
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

    // Area fills first, then roads, then building footprints. Existing grid remains above these at Z 3/4.
    AddComponent(Sector->GetTacticalParkGeometry(), TacticalPark, TacticalPark, 1, false, 2.0f, 256);
    AddComponent(Sector->GetTacticalStadiumGeometry(), TacticalStadium, TacticalStadium, 1, false, 2.0f, 256);
    AddComponent(Sector->GetTacticalSidewalks(), TacticalSidewalk, TacticalSidewalk, 1, false, 1.0f, 768);
    AddComponent(Sector->GetTacticalRoads(), TacticalRoad, TacticalRoad, 2, false, 2.0f, 768);
    AddComponent(Sector->GetTacticalBuildings(), TacticalBuildingFill, TacticalBuildingOutline, 2, true, 2.0f, 768);
    AddComponent(Sector->GetTacticalResidentialRoofs(), TacticalBuildingFill, TacticalBuildingOutline, 2, true, 2.0f, 768);
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
        TEXT("Tactical Map production vector layer built from actual Oster Roads/Buildings/Landmarks/Park/Stadium geometry."));
}
