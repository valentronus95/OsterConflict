#include "OCTacticalMapSubsystem.h"

#include "OCCharacter.h"
#include "OCGeoReference.h"
#include "OCPlayerController.h"
#include "OCWorldSectorOster.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/PrimitiveComponent.h"
#include "Components/ScaleBox.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/SceneCapture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "InputAction.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "UObject/UObjectGlobals.h"

namespace
{
    constexpr float DesignWidth = 1600.0f;
    constexpr float DesignHeight = 900.0f;
    constexpr float MapWidth = 1248.0f;
    constexpr float MapHeight = 702.0f;
    constexpr float MapFrameLeft = 315.0f;
    constexpr float MapFrameTop = 92.0f;
    constexpr uint32 CaptureWidth = 1600;
    constexpr uint32 CaptureHeight = 900;
    constexpr float CaptureAspect = static_cast<float>(CaptureWidth) / static_cast<float>(CaptureHeight);

    const FLinearColor ColorBackdrop(0.012f, 0.016f, 0.020f, 0.985f);
    const FLinearColor ColorPanel(0.025f, 0.032f, 0.038f, 0.96f);
    const FLinearColor ColorMap(0.055f, 0.075f, 0.060f, 1.0f);
    const FLinearColor ColorPrimaryText(0.86f, 0.89f, 0.91f, 1.0f);
    const FLinearColor ColorMutedText(0.53f, 0.58f, 0.61f, 1.0f);
    const FLinearColor ColorGrid(0.50f, 0.58f, 0.54f, 0.14f);
    const FLinearColor ColorFriendly(0.18f, 0.52f, 0.94f, 1.0f);
    const FLinearColor ColorPlayer(0.40f, 0.80f, 0.28f, 1.0f);
    const FLinearColor ColorObjective(0.95f, 0.50f, 0.08f, 1.0f);

    void SetTextSize(UTextBlock* Text, const int32 Size)
    {
        if (!Text) return;
        FSlateFontInfo Font = Text->GetFont();
        Font.Size = Size;
        Text->SetFont(Font);
    }

    UCanvasPanelSlot* PlaceOnCanvas(
        UCanvasPanel* Canvas,
        UWidget* Widget,
        const FVector2D Position,
        const FVector2D Size,
        const FVector2D Alignment = FVector2D::ZeroVector,
        const int32 ZOrder = 0)
    {
        if (!Canvas || !Widget) return nullptr;
        UCanvasPanelSlot* Slot = Canvas->AddChildToCanvas(Widget);
        if (!Slot) return nullptr;
        Slot->SetPosition(Position);
        Slot->SetSize(Size);
        Slot->SetAlignment(Alignment);
        Slot->SetZOrder(ZOrder);
        return Slot;
    }

    UTextBlock* AddCanvasText(
        UWidgetTree* WidgetTree,
        UCanvasPanel* Canvas,
        const FName Name,
        const FString& Value,
        const FVector2D Position,
        const FVector2D Size,
        const int32 FontSize,
        const FLinearColor& Color,
        const int32 ZOrder = 0)
    {
        if (!WidgetTree || !Canvas) return nullptr;
        UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
        Text->SetText(FText::FromString(Value));
        Text->SetColorAndOpacity(FSlateColor(Color));
        SetTextSize(Text, FontSize);
        PlaceOnCanvas(Canvas, Text, Position, Size, FVector2D::ZeroVector, ZOrder);
        return Text;
    }

    FBox ResolveSectorContentBounds(AOCWorldSectorOster& Sector)
    {
        FBox ContentBounds(ForceInit);
        TArray<UPrimitiveComponent*> Components;
        Sector.GetComponents<UPrimitiveComponent>(Components);

        for (UPrimitiveComponent* Component : Components)
        {
            if (!IsValid(Component)) continue;
            const FString ComponentName = Component->GetName();
            if (ComponentName == TEXT("Ground") ||
                ComponentName == TEXT("ReferenceMarkers") ||
                ComponentName.Contains(TEXT("Label")))
            {
                continue;
            }

            const FBox ComponentBounds = Component->Bounds.GetBox();
            if (ComponentBounds.IsValid) ContentBounds += ComponentBounds;
        }

        if (!ContentBounds.IsValid)
        {
            ContentBounds = Sector.GetComponentsBoundingBox(true);
        }
        return ContentBounds;
    }

    void FitProjectionBoundsToAspect(
        FVector2D& Min,
        FVector2D& Max,
        const float TargetAspect,
        const float PaddingFraction)
    {
        const FVector2D Center = (Min + Max) * 0.5f;
        FVector2D Half = (Max - Min) * 0.5f;
        Half.X = FMath::Max(Half.X, 1000.0f);
        Half.Y = FMath::Max(Half.Y, 1000.0f);
        Half *= 1.0f + PaddingFraction;

        const float CurrentAspect = Half.X / Half.Y;
        if (CurrentAspect < TargetAspect) Half.X = Half.Y * TargetAspect;
        else Half.Y = Half.X / TargetAspect;

        Min = Center - Half;
        Max = Center + Half;
    }

    bool BuildProjectionFromSector(AOCWorldSectorOster& Sector, FOCTacticalMapProjection& Projection)
    {
        const FBox Bounds = ResolveSectorContentBounds(Sector);
        if (!Bounds.IsValid ||
            Bounds.Max.X <= Bounds.Min.X + 100.0f ||
            Bounds.Max.Y <= Bounds.Min.Y + 100.0f)
        {
            return false;
        }

        Projection.WorldMin = FVector2D(Bounds.Min.X, Bounds.Min.Y);
        Projection.WorldMax = FVector2D(Bounds.Max.X, Bounds.Max.Y);
        FitProjectionBoundsToAspect(Projection.WorldMin, Projection.WorldMax, CaptureAspect, 0.025f);
        Projection.bInvertX = false;
        Projection.bInvertY = true;
        return Projection.IsValid();
    }
}

void UOCTacticalMapWidget::ConfigureWorldMap(
    const FOCTacticalMapProjection& InProjection,
    AOCWorldSectorOster* InWorldSector,
    UTextureRenderTarget2D* InWorldMapTexture)
{
    Projection = InProjection;
    WorldSector = InWorldSector;
    WorldMapTexture = InWorldMapTexture;
    bConfiguredFromSubsystem = Projection.IsValid();
}

TSharedRef<SWidget> UOCTacticalMapWidget::RebuildWidget()
{
    if (!WidgetTree)
    {
        WidgetTree = NewObject<UWidgetTree>(this, TEXT("TacticalMapWidgetTree"));
    }

    UScaleBox* RootScale = WidgetTree->ConstructWidget<UScaleBox>(
        UScaleBox::StaticClass(), TEXT("TacticalMapRootScale"));
    RootScale->SetStretch(EStretch::ScaleToFit);
    RootScale->SetStretchDirection(EStretchDirection::Both);
    WidgetTree->RootWidget = RootScale;

    USizeBox* DesignSize = WidgetTree->ConstructWidget<USizeBox>(
        USizeBox::StaticClass(), TEXT("TacticalMapDesignSize"));
    DesignSize->SetWidthOverride(DesignWidth);
    DesignSize->SetHeightOverride(DesignHeight);
    RootScale->SetContent(DesignSize);

    UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(
        UCanvasPanel::StaticClass(), TEXT("TacticalMapRoot"));
    DesignSize->SetContent(Root);

    UBorder* Backdrop = WidgetTree->ConstructWidget<UBorder>(
        UBorder::StaticClass(), TEXT("TacticalMapBackdrop"));
    Backdrop->SetBrushColor(ColorBackdrop);
    if (UCanvasPanelSlot* BackdropSlot = Root->AddChildToCanvas(Backdrop))
    {
        BackdropSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
        BackdropSlot->SetOffsets(FMargin(0.0f));
        BackdropSlot->SetZOrder(0);
    }

    AddCanvasText(WidgetTree, Root, TEXT("TacticalMapTitle"), TEXT("TACTICAL MAP"),
        FVector2D(36.0f, 20.0f), FVector2D(500.0f, 44.0f), 29, ColorPrimaryText, 5);
    AddCanvasText(WidgetTree, Root, TEXT("TacticalMapGameTitle"), TEXT("OSTER CONFLICT"),
        FVector2D(38.0f, 56.0f), FVector2D(300.0f, 28.0f), 15, ColorObjective, 5);
    AddCanvasText(WidgetTree, Root, TEXT("TacticalMapSector"), TEXT("СЕКТОР: ОСТЕР  |  WORLD-SYNC"),
        FVector2D(1190.0f, 28.0f), FVector2D(360.0f, 28.0f), 14, ColorMutedText, 5);

    UBorder* LegendPanel = WidgetTree->ConstructWidget<UBorder>(
        UBorder::StaticClass(), TEXT("TacticalMapLegendPanel"));
    LegendPanel->SetBrushColor(ColorPanel);
    PlaceOnCanvas(Root, LegendPanel, FVector2D(20.0f, MapFrameTop), FVector2D(270.0f, 390.0f), FVector2D::ZeroVector, 2);

    AddCanvasText(WidgetTree, Root, TEXT("LegendTitle"), TEXT("ЛЕГЕНДА"),
        FVector2D(42.0f, 118.0f), FVector2D(210.0f, 32.0f), 17, ColorPrimaryText, 4);
    AddCanvasText(WidgetTree, Root, TEXT("LegendPlayer"), TEXT("▲   ГРАВЕЦЬ"),
        FVector2D(44.0f, 174.0f), FVector2D(210.0f, 28.0f), 16, ColorPlayer, 4);
    AddCanvasText(WidgetTree, Root, TEXT("LegendSquad"), TEXT("●   ЧЛЕНИ ЗАГОНУ"),
        FVector2D(44.0f, 226.0f), FVector2D(220.0f, 28.0f), 16, ColorFriendly, 4);
    AddCanvasText(WidgetTree, Root, TEXT("LegendVehicle"), TEXT("▣   ТРАНСПОРТ"),
        FVector2D(44.0f, 278.0f), FVector2D(220.0f, 28.0f), 16, ColorFriendly, 4);
    AddCanvasText(WidgetTree, Root, TEXT("LegendObjective"), TEXT("◇   ЦІЛЬ"),
        FVector2D(44.0f, 330.0f), FVector2D(220.0f, 28.0f), 16, ColorObjective, 4);
    AddCanvasText(WidgetTree, Root, TEXT("LegendPOI"), TEXT("○   ТОЧКА ІНТЕРЕСУ"),
        FVector2D(44.0f, 382.0f), FVector2D(220.0f, 28.0f), 16, ColorPrimaryText, 4);

    UBorder* MapFrame = WidgetTree->ConstructWidget<UBorder>(
        UBorder::StaticClass(), TEXT("TacticalMapFrame"));
    MapFrame->SetBrushColor(FLinearColor(0.12f, 0.15f, 0.15f, 1.0f));
    PlaceOnCanvas(Root, MapFrame, FVector2D(MapFrameLeft, MapFrameTop), FVector2D(MapWidth, MapHeight), FVector2D::ZeroVector, 2);

    MapCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(
        UCanvasPanel::StaticClass(), TEXT("TacticalMapCanvas"));
    MapCanvas->SetClipping(EWidgetClipping::ClipToBounds);
    MapFrame->SetContent(MapCanvas);

    UBorder* MapField = WidgetTree->ConstructWidget<UBorder>(
        UBorder::StaticClass(), TEXT("TacticalMapField"));
    MapField->SetBrushColor(ColorMap);
    if (UCanvasPanelSlot* MapFieldSlot = MapCanvas->AddChildToCanvas(MapField))
    {
        MapFieldSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
        MapFieldSlot->SetOffsets(FMargin(2.0f));
        MapFieldSlot->SetZOrder(0);
    }

    if (!bConfiguredFromSubsystem) ResolveProjectionFromWorld();

    if (WorldMapTexture)
    {
        UImage* WorldMapImage = WidgetTree->ConstructWidget<UImage>(
            UImage::StaticClass(), TEXT("TacticalMapWorldCapture"));
        FSlateBrush MapBrush;
        MapBrush.SetResourceObject(WorldMapTexture);
        MapBrush.ImageSize = FVector2D(MapWidth, MapHeight);
        WorldMapImage->SetBrush(MapBrush);
        WorldMapImage->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
        WorldMapImage->SetRenderScale(FVector2D(-1.0f, 1.0f));

        if (UCanvasPanelSlot* WorldMapSlot = MapCanvas->AddChildToCanvas(WorldMapImage))
        {
            WorldMapSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
            WorldMapSlot->SetOffsets(FMargin(0.0f));
            WorldMapSlot->SetZOrder(1);
        }
    }

    AddGrid();
    AddLandmarkMarker(TEXT("МУЗЕЙ"), ResolveSectorWorldLocation(AOCWorldSectorOster::MuseumAnchor()));
    AddLandmarkMarker(TEXT("СТАДІОН"), ResolveSectorWorldLocation(AOCWorldSectorOster::StadiumAnchor()));
    AddLandmarkMarker(TEXT("ПАРК"), ResolveSectorWorldLocation(AOCWorldSectorOster::ParkAnchor()));
    AddLandmarkMarker(TEXT("ЦЕНТР"), ResolveSectorWorldLocation(AOCWorldSectorOster::FormerCityAdministrationAnchor()));

    const FOCGeoReferencePoint SilpoRef = FOCGeoReference::Silpo();
    const FVector SilpoLocal = FOCGeoReference::ToLocalCm(SilpoRef.Latitude, SilpoRef.Longitude, 0.0f);
    AddLandmarkMarker(TEXT("СІЛЬПО"), ResolveSectorWorldLocation(SilpoLocal));

    AddCanvasText(WidgetTree, MapCanvas, TEXT("NorthIndicator"), TEXT("N\n↑"),
        FVector2D(MapWidth - 54.0f, 18.0f), FVector2D(36.0f, 58.0f), 20, ColorPrimaryText, 8);

    PlayerMarker = WidgetTree->ConstructWidget<UTextBlock>(
        UTextBlock::StaticClass(), TEXT("TacticalMapPlayerMarker"));
    PlayerMarker->SetText(FText::FromString(TEXT("▲")));
    PlayerMarker->SetColorAndOpacity(FSlateColor(ColorPlayer));
    PlayerMarker->SetJustification(ETextJustify::Center);
    PlayerMarker->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
    SetTextSize(PlayerMarker, 24);
    PlaceOnCanvas(MapCanvas, PlayerMarker, FVector2D(MapWidth * 0.5f, MapHeight * 0.5f),
        FVector2D(42.0f, 42.0f), FVector2D(0.5f, 0.5f), 20);

    PlayerCoordinates = AddCanvasText(WidgetTree, MapCanvas, TEXT("TacticalMapPlayerCoordinates"), TEXT(""),
        FVector2D(18.0f, MapHeight - 34.0f), FVector2D(720.0f, 24.0f), 13, ColorMutedText, 10);

    const float WidthMeters = (Projection.WorldMax.X - Projection.WorldMin.X) / 100.0f;
    AddCanvasText(WidgetTree, MapCanvas, TEXT("TacticalMapScale"),
        FString::Printf(TEXT("ШИРИНА СЕКТОРА: %.0f м"), WidthMeters),
        FVector2D(MapWidth - 250.0f, MapHeight - 34.0f), FVector2D(230.0f, 24.0f), 13, ColorMutedText, 10);

    AddCanvasText(WidgetTree, Root, TEXT("TacticalMapHintClose"), TEXT("M  ЗАКРИТИ МАПУ"),
        FVector2D(38.0f, 846.0f), FVector2D(240.0f, 28.0f), 14, ColorMutedText, 5);
    AddCanvasText(WidgetTree, Root, TEXT("TacticalMapHintNext"),
        TEXT("МАСШТАБ · ПЕРЕМІЩЕННЯ · МАРКЕРИ — НАСТУПНИЙ ЕТАП"),
        FVector2D(590.0f, 846.0f), FVector2D(680.0f, 28.0f), 14, ColorMutedText, 5);

    return RootScale->TakeWidget();
}

void UOCTacticalMapWidget::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    const APlayerController* PC = GetOwningPlayer();
    const APawn* Pawn = PC ? PC->GetPawn() : nullptr;
    if (!Pawn || !PlayerMarker) return;

    const FVector Location = Pawn->GetActorLocation();
    if (UCanvasPanelSlot* MarkerSlot = Cast<UCanvasPanelSlot>(PlayerMarker->Slot))
    {
        MarkerSlot->SetPosition(WorldToMap(Location));
    }
    PlayerMarker->SetRenderTransformAngle(Projection.WorldYawToMapDegrees(Pawn->GetActorRotation().Yaw));

    if (PlayerCoordinates)
    {
        PlayerCoordinates->SetText(FText::FromString(FString::Printf(
            TEXT("WORLD  X %.0f  ·  Y %.0f  ·  Z %.0f"), Location.X, Location.Y, Location.Z)));
    }
}

bool UOCTacticalMapWidget::ResolveProjectionFromWorld()
{
    WorldSector.Reset();
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AOCWorldSectorOster> It(World); It; ++It)
        {
            AOCWorldSectorOster* Sector = *It;
            if (!IsValid(Sector)) continue;
            if (BuildProjectionFromSector(*Sector, Projection))
            {
                WorldSector = Sector;
                UE_LOG(LogTemp, Display,
                    TEXT("Tactical Map 2.0: widget fallback uses actual Oster content bounds Min(%.0f, %.0f) Max(%.0f, %.0f)."),
                    Projection.WorldMin.X, Projection.WorldMin.Y, Projection.WorldMax.X, Projection.WorldMax.Y);
                return true;
            }
        }
    }

    Projection.WorldMin = FVector2D(-120000.0f, -67500.0f);
    Projection.WorldMax = FVector2D(120000.0f, 67500.0f);
    Projection.bInvertX = false;
    Projection.bInvertY = true;
    UE_LOG(LogTemp, Warning,
        TEXT("Tactical Map 2.0: world sector not found; using deterministic 16:9 source fallback bounds."));
    return false;
}

FVector UOCTacticalMapWidget::ResolveSectorWorldLocation(const FVector& SectorLocalLocation) const
{
    if (const AOCWorldSectorOster* Sector = WorldSector.Get())
    {
        return Sector->GetActorTransform().TransformPosition(SectorLocalLocation);
    }
    return SectorLocalLocation;
}

FVector2D UOCTacticalMapWidget::WorldToMap(const FVector& WorldLocation) const
{
    const FVector2D UV = Projection.WorldToUV(WorldLocation, true);
    return FVector2D(UV.X * MapWidth, UV.Y * MapHeight);
}

void UOCTacticalMapWidget::AddLandmarkMarker(const FString& Label, const FVector& WorldLocation)
{
    if (!WidgetTree || !MapCanvas) return;
    UTextBlock* Marker = WidgetTree->ConstructWidget<UTextBlock>();
    Marker->SetText(FText::FromString(FString::Printf(TEXT("○ %s"), *Label)));
    Marker->SetColorAndOpacity(FSlateColor(ColorPrimaryText));
    Marker->SetJustification(ETextJustify::Center);
    SetTextSize(Marker, 14);
    PlaceOnCanvas(MapCanvas, Marker, WorldToMap(WorldLocation), FVector2D(150.0f, 26.0f),
        FVector2D(0.5f, 0.5f), 12);
}

void UOCTacticalMapWidget::AddGrid()
{
    if (!WidgetTree || !MapCanvas) return;
    constexpr int32 Columns = 10;
    constexpr int32 Rows = 10;

    for (int32 Index = 0; Index <= Columns; ++Index)
    {
        const float X = MapWidth * static_cast<float>(Index) / static_cast<float>(Columns);
        UBorder* Line = WidgetTree->ConstructWidget<UBorder>(
            UBorder::StaticClass(), FName(*FString::Printf(TEXT("MapGridV_%02d"), Index)));
        Line->SetBrushColor(ColorGrid);
        PlaceOnCanvas(MapCanvas, Line, FVector2D(X, 0.0f), FVector2D(1.0f, MapHeight), FVector2D::ZeroVector, 3);
        if (Index < Columns)
        {
            const TCHAR ColumnLetter = static_cast<TCHAR>('A' + Index);
            AddCanvasText(WidgetTree, MapCanvas,
                FName(*FString::Printf(TEXT("MapGridCol_%02d"), Index)), FString::Chr(ColumnLetter),
                FVector2D(X + MapWidth / Columns * 0.5f - 8.0f, 7.0f), FVector2D(24.0f, 22.0f),
                12, ColorMutedText, 4);
        }
    }

    for (int32 Index = 0; Index <= Rows; ++Index)
    {
        const float Y = MapHeight * static_cast<float>(Index) / static_cast<float>(Rows);
        UBorder* Line = WidgetTree->ConstructWidget<UBorder>(
            UBorder::StaticClass(), FName(*FString::Printf(TEXT("MapGridH_%02d"), Index)));
        Line->SetBrushColor(ColorGrid);
        PlaceOnCanvas(MapCanvas, Line, FVector2D(0.0f, Y), FVector2D(MapWidth, 1.0f), FVector2D::ZeroVector, 3);
        if (Index < Rows)
        {
            AddCanvasText(WidgetTree, MapCanvas,
                FName(*FString::Printf(TEXT("MapGridRow_%02d"), Index)), FString::FromInt(Index + 1),
                FVector2D(8.0f, Y + MapHeight / Rows * 0.5f - 8.0f), FVector2D(30.0f, 22.0f),
                12, ColorMutedText, 4);
        }
    }
}

bool UOCTacticalMapSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCTacticalMapSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (InWorld.GetNetMode() == NM_DedicatedServer) return;

    MapToggleAction = NewObject<UInputAction>(this, TEXT("IA_TacticalMapRuntime"));
    MapToggleAction->ValueType = EInputActionValueType::Boolean;
    MapMappingContext = NewObject<UInputMappingContext>(this, TEXT("IMC_TacticalMapRuntime"));
    if (MapMappingContext && MapToggleAction) MapMappingContext->MapKey(MapToggleAction, EKeys::M);

    ResolveWorldMapSource();
    InWorld.GetTimerManager().SetTimer(InputSetupTimer, this,
        &UOCTacticalMapSubsystem::EnsureEnhancedInputBinding, 0.25f, true, 0.05f);
}

void UOCTacticalMapSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(InputSetupTimer);

    if (AOCPlayerController* PC = BoundPlayerController.Get())
    {
        if (MapMappingContext)
        {
            if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
                ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
            {
                InputSubsystem->RemoveMappingContext(MapMappingContext);
            }
        }
    }

    if (MapWidget)
    {
        MapWidget->RemoveFromParent();
        MapWidget = nullptr;
    }
    ReleaseCaptureResources();
    BoundInputComponent.Reset();
    BoundPlayerController.Reset();
    RemappedCharacter.Reset();
    MapMappingContext = nullptr;
    MapToggleAction = nullptr;
    bMapOpen = false;
    Super::Deinitialize();
}

void UOCTacticalMapSubsystem::EnsureEnhancedInputBinding()
{
    UWorld* World = GetWorld();
    AOCPlayerController* PC = World ? Cast<AOCPlayerController>(World->GetFirstPlayerController()) : nullptr;
    if (!PC || !PC->IsLocalController()) return;

    if (BoundPlayerController.Get() != PC)
    {
        if (AOCPlayerController* OldPC = BoundPlayerController.Get())
        {
            if (MapMappingContext)
            {
                if (UEnhancedInputLocalPlayerSubsystem* OldInputSubsystem =
                    ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(OldPC->GetLocalPlayer()))
                {
                    OldInputSubsystem->RemoveMappingContext(MapMappingContext);
                }
            }
        }
        BoundPlayerController = PC;
        BoundInputComponent.Reset();
    }

    if (MapMappingContext)
    {
        if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            if (!InputSubsystem->HasMappingContext(MapMappingContext))
            {
                InputSubsystem->AddMappingContext(MapMappingContext, 100);
            }
        }
    }

    UEnhancedInputComponent* EnhancedInput = PC->FindComponentByClass<UEnhancedInputComponent>();
    if (EnhancedInput && BoundInputComponent.Get() != EnhancedInput && MapToggleAction)
    {
        EnhancedInput->BindAction(MapToggleAction, ETriggerEvent::Started, this,
            &UOCTacticalMapSubsystem::HandleMapToggleAction);
        BoundInputComponent = EnhancedInput;
        UE_LOG(LogTemp, Display,
            TEXT("Tactical Map 2.0: M registered through Enhanced Input context priority 100."));
    }

    AOCCharacter* Character = Cast<AOCCharacter>(PC->GetPawn());
    if (Character && RemappedCharacter.Get() != Character)
    {
        EnsureExclusiveMapBinding(*Character);
        RemappedCharacter = Character;
    }

    if (!bMapOpen) return;
    if (HasBlockingUI(*PC))
    {
        CloseMap(*PC, false);
        return;
    }
    if (!Character) CloseMap(*PC, true);
}

void UOCTacticalMapSubsystem::HandleMapToggleAction()
{
    if (AOCPlayerController* PC = BoundPlayerController.Get()) ToggleMap(*PC);
}

void UOCTacticalMapSubsystem::ToggleMap(AOCPlayerController& PlayerController)
{
    if (!PlayerController.IsLocalController()) return;
    if (bMapOpen)
    {
        CloseMap(PlayerController, true);
        return;
    }
    if (PlayerController.GetPawn() && CanOpenMap(PlayerController)) OpenMap(PlayerController);
}

void UOCTacticalMapSubsystem::EnsureExclusiveMapBinding(AOCCharacter& Character)
{
    UInputMappingContext* CharacterContext =
        FindObject<UInputMappingContext>(&Character, TEXT("IMC_RuntimeDefault"));
    UInputAction* TrapAction = FindObject<UInputAction>(&Character, TEXT("IA_DeployTrap"));
    if (!CharacterContext || !TrapAction)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("Tactical Map 2.0: could not locate DeployTrap mapping; M exclusivity is not proven for this pawn."));
        return;
    }

    CharacterContext->UnmapKey(TrapAction, EKeys::M);
    CharacterContext->UnmapKey(TrapAction, EKeys::V);
    CharacterContext->MapKey(TrapAction, EKeys::V);

    if (APlayerController* PC = Cast<APlayerController>(Character.GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            InputSubsystem->RequestRebuildControlMappings();
        }
    }
    UE_LOG(LogTemp, Display,
        TEXT("Tactical Map 2.0: M reserved for map; DeployTrap moved to V for this pawn."));
}

bool UOCTacticalMapSubsystem::HasBlockingUI(const AOCPlayerController& PlayerController) const
{
    return PlayerController.IsFrontendMenuVisible() ||
        PlayerController.IsDeploymentPanelVisible() ||
        PlayerController.IsAdminPanelVisible() ||
        PlayerController.IsChatInputActive() ||
        PlayerController.IsSettingsVisible();
}

bool UOCTacticalMapSubsystem::CanOpenMap(const AOCPlayerController& PlayerController) const
{
    return !HasBlockingUI(PlayerController);
}

bool UOCTacticalMapSubsystem::ResolveWorldMapSource()
{
    WorldSector.Reset();
    UWorld* World = GetWorld();
    if (!World) return false;

    for (TActorIterator<AOCWorldSectorOster> It(World); It; ++It)
    {
        AOCWorldSectorOster* Sector = *It;
        if (!IsValid(Sector)) continue;
        if (BuildProjectionFromSector(*Sector, MapProjection))
        {
            WorldSector = Sector;
            UE_LOG(LogTemp, Display,
                TEXT("Tactical Map 2.0: actual world source resolved Min(%.0f, %.0f) Max(%.0f, %.0f)."),
                MapProjection.WorldMin.X, MapProjection.WorldMin.Y, MapProjection.WorldMax.X, MapProjection.WorldMax.Y);
            return true;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Tactical Map 2.0: AOCWorldSectorOster source was not found."));
    return false;
}

bool UOCTacticalMapSubsystem::CaptureWorldMap()
{
    if ((!WorldSector.IsValid() || !MapProjection.IsValid()) && !ResolveWorldMapSource()) return false;

    UWorld* World = GetWorld();
    AOCWorldSectorOster* Sector = WorldSector.Get();
    if (!World || !Sector) return false;

    if (!MapRenderTarget)
    {
        MapRenderTarget = NewObject<UTextureRenderTarget2D>(this, TEXT("RT_TacticalMapRuntime"));
        if (!MapRenderTarget) return false;
        MapRenderTarget->ClearColor = ColorMap;
        MapRenderTarget->bHDR = false;
        MapRenderTarget->InitAutoFormat(CaptureWidth, CaptureHeight);
        MapRenderTarget->UpdateResourceImmediate(true);
    }

    const FBox ContentBounds = ResolveSectorContentBounds(*Sector);
    const FVector2D Center2D = (MapProjection.WorldMin + MapProjection.WorldMax) * 0.5f;
    const float HighestWorldZ = ContentBounds.IsValid ? ContentBounds.Max.Z : Sector->GetActorLocation().Z;
    const FVector CaptureLocation(Center2D.X, Center2D.Y, HighestWorldZ + 150000.0f);
    const FRotator CaptureRotation(-90.0f, 90.0f, 0.0f);

    if (!MapCaptureActor)
    {
        FActorSpawnParameters SpawnParameters;
        SpawnParameters.ObjectFlags |= RF_Transient;
        SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        MapCaptureActor = World->SpawnActor<ASceneCapture2D>(
            ASceneCapture2D::StaticClass(), CaptureLocation, CaptureRotation, SpawnParameters);
        if (!MapCaptureActor) return false;
    }
    else
    {
        MapCaptureActor->SetActorLocationAndRotation(
            CaptureLocation, CaptureRotation, false, nullptr, ETeleportType::TeleportPhysics);
    }

    USceneCaptureComponent2D* CaptureComponent = MapCaptureActor->GetCaptureComponent2D();
    if (!CaptureComponent)
    {
        ReleaseCaptureResources();
        return false;
    }

    CaptureComponent->ProjectionType = ECameraProjectionMode::Orthographic;
    CaptureComponent->OrthoWidth = MapProjection.WorldMax.X - MapProjection.WorldMin.X;
    CaptureComponent->TextureTarget = MapRenderTarget;
    CaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;

    // The sector owns extent/projection only. Capture all scene primitives so separately-owned
    // Museum/Silpo/Culture/roadside actors are represented exactly where the gameplay world placed them.
    CaptureComponent->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
    CaptureComponent->ClearHiddenComponents();

    // Pawns are dynamic and receive their own tactical marker; do not bake them into the background texture.
    for (TActorIterator<APawn> PawnIt(World); PawnIt; ++PawnIt)
    {
        APawn* Pawn = *PawnIt;
        if (IsValid(Pawn)) CaptureComponent->HideActorComponents(Pawn, true);
    }

    CaptureComponent->bCaptureEveryFrame = false;
    CaptureComponent->bCaptureOnMovement = false;
    CaptureComponent->bUpdateOrthoPlanes = true;
    CaptureComponent->bAutoCalculateOrthoPlanes = true;
    CaptureComponent->CaptureScene();

    UE_LOG(LogTemp, Display,
        TEXT("Tactical Map 2.0: captured current gameplay world to %ux%u render target; OrthoWidth %.0f cm."),
        CaptureWidth, CaptureHeight, CaptureComponent->OrthoWidth);
    return true;
}

void UOCTacticalMapSubsystem::ReleaseCaptureResources()
{
    if (MapCaptureActor)
    {
        MapCaptureActor->Destroy();
        MapCaptureActor = nullptr;
    }
    MapRenderTarget = nullptr;
    WorldSector.Reset();
}

void UOCTacticalMapSubsystem::OpenMap(AOCPlayerController& PlayerController)
{
    if (bMapOpen || !CanOpenMap(PlayerController) || !PlayerController.GetPawn()) return;

    CaptureWorldMap();
    MapWidget = CreateWidget<UOCTacticalMapWidget>(&PlayerController, UOCTacticalMapWidget::StaticClass());
    if (!MapWidget) return;

    if (MapProjection.IsValid())
    {
        MapWidget->ConfigureWorldMap(MapProjection, WorldSector.Get(), MapRenderTarget);
    }
    MapWidget->AddToViewport(900);

    PlayerController.ResetIgnoreMoveInput();
    PlayerController.ResetIgnoreLookInput();
    PlayerController.SetIgnoreMoveInput(true);
    PlayerController.SetIgnoreLookInput(true);
    PlayerController.bShowMouseCursor = true;
    FInputModeGameAndUI Mode;
    Mode.SetHideCursorDuringCapture(false);
    PlayerController.SetInputMode(Mode);
    bMapOpen = true;
}

void UOCTacticalMapSubsystem::CloseMap(AOCPlayerController& PlayerController, const bool bRestoreGameplayInput)
{
    if (MapWidget)
    {
        MapWidget->RemoveFromParent();
        MapWidget = nullptr;
    }
    bMapOpen = false;
    if (!bRestoreGameplayInput) return;

    PlayerController.ResetIgnoreMoveInput();
    PlayerController.ResetIgnoreLookInput();
    PlayerController.bShowMouseCursor = false;
    PlayerController.SetInputMode(FInputModeGameOnly());
}
