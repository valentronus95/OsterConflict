#include "OCTacticalMapSubsystem.h"

#include "OCCharacter.h"
#include "OCPlayerController.h"
#include "OCWorldSectorOster.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "InputAction.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "UObject/UObjectGlobals.h"

namespace
{
    constexpr float MapLeft = 70.0f;
    constexpr float MapTop = 100.0f;
    constexpr float MapWidth = 960.0f;
    constexpr float MapHeight = 500.0f;

    void SetTextSize(UTextBlock* Text, int32 Size)
    {
        if (!Text) return;
        FSlateFontInfo Font = Text->GetFont();
        Font.Size = Size;
        Text->SetFont(Font);
    }
}

TSharedRef<SWidget> UOCTacticalMapWidget::RebuildWidget()
{
    if (!WidgetTree)
    {
        WidgetTree = NewObject<UWidgetTree>(this, TEXT("TacticalMapWidgetTree"));
    }

    UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("TacticalMapRoot"));
    WidgetTree->RootWidget = Root;

    UBorder* Backdrop = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("TacticalMapBackdrop"));
    Backdrop->SetBrushColor(FLinearColor(0.015f, 0.020f, 0.026f, 0.96f));
    UCanvasPanelSlot* BackdropSlot = Root->AddChildToCanvas(Backdrop);
    BackdropSlot->SetAnchors(FAnchors(0.5f, 0.5f));
    BackdropSlot->SetAlignment(FVector2D(0.5f, 0.5f));
    BackdropSlot->SetPosition(FVector2D::ZeroVector);
    BackdropSlot->SetSize(FVector2D(1100.0f, 680.0f));

    MapCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("TacticalMapCanvas"));
    Backdrop->SetContent(MapCanvas);

    UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TacticalMapTitle"));
    Title->SetText(FText::FromString(TEXT("ТАКТИЧНА МАПА · ОСТЕР")));
    Title->SetColorAndOpacity(FSlateColor(FLinearColor(0.86f, 0.91f, 0.94f, 1.0f)));
    SetTextSize(Title, 28);
    if (UCanvasPanelSlot* Slot = MapCanvas->AddChildToCanvas(Title))
    {
        Slot->SetPosition(FVector2D(42.0f, 28.0f));
        Slot->SetSize(FVector2D(560.0f, 42.0f));
    }

    UTextBlock* Hint = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TacticalMapHint"));
    Hint->SetText(FText::FromString(TEXT("M / Esc — закрити")));
    Hint->SetColorAndOpacity(FSlateColor(FLinearColor(0.62f, 0.68f, 0.72f, 1.0f)));
    SetTextSize(Hint, 15);
    if (UCanvasPanelSlot* Slot = MapCanvas->AddChildToCanvas(Hint))
    {
        Slot->SetPosition(FVector2D(860.0f, 34.0f));
        Slot->SetSize(FVector2D(190.0f, 30.0f));
    }

    UBorder* MapField = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("TacticalMapField"));
    MapField->SetBrushColor(FLinearColor(0.045f, 0.060f, 0.068f, 1.0f));
    if (UCanvasPanelSlot* Slot = MapCanvas->AddChildToCanvas(MapField))
    {
        Slot->SetPosition(FVector2D(MapLeft, MapTop));
        Slot->SetSize(FVector2D(MapWidth, MapHeight));
    }

    const FVector Anchors[] =
    {
        AOCWorldSectorOster::MuseumAnchor(),
        AOCWorldSectorOster::StadiumAnchor(),
        AOCWorldSectorOster::ParkAnchor(),
        AOCWorldSectorOster::CollegeAnchor(),
        AOCWorldSectorOster::CultureParkNorthAnchor(),
        AOCWorldSectorOster::FormerCityAdministrationAnchor(),
        AOCWorldSectorOster::HistoricCourtAnchor(),
        AOCWorldSectorOster::ResurrectionChurchAnchor()
    };

    float MinX = Anchors[0].X;
    float MaxX = Anchors[0].X;
    float MinY = Anchors[0].Y;
    float MaxY = Anchors[0].Y;
    for (const FVector& Anchor : Anchors)
    {
        MinX = FMath::Min(MinX, Anchor.X);
        MaxX = FMath::Max(MaxX, Anchor.X);
        MinY = FMath::Min(MinY, Anchor.Y);
        MaxY = FMath::Max(MaxY, Anchor.Y);
    }

    const float PadX = FMath::Max(6000.0f, (MaxX - MinX) * 0.10f);
    const float PadY = FMath::Max(6000.0f, (MaxY - MinY) * 0.10f);
    WorldMin = FVector2D(MinX - PadX, MinY - PadY);
    WorldMax = FVector2D(MaxX + PadX, MaxY + PadY);

    AddLandmarkMarker(TEXT("МУЗЕЙ"), AOCWorldSectorOster::MuseumAnchor());
    AddLandmarkMarker(TEXT("СТАДІОН"), AOCWorldSectorOster::StadiumAnchor());
    AddLandmarkMarker(TEXT("ПАРК"), AOCWorldSectorOster::ParkAnchor());
    AddLandmarkMarker(TEXT("КОЛЕДЖ"), AOCWorldSectorOster::CollegeAnchor());
    AddLandmarkMarker(TEXT("ПІВН. ПАРК"), AOCWorldSectorOster::CultureParkNorthAnchor());
    AddLandmarkMarker(TEXT("АДМІН."), AOCWorldSectorOster::FormerCityAdministrationAnchor());
    AddLandmarkMarker(TEXT("СУД"), AOCWorldSectorOster::HistoricCourtAnchor());
    AddLandmarkMarker(TEXT("ЦЕРКВА"), AOCWorldSectorOster::ResurrectionChurchAnchor());

    PlayerMarker = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TacticalMapPlayerMarker"));
    PlayerMarker->SetText(FText::FromString(TEXT("▲ ВИ")));
    PlayerMarker->SetColorAndOpacity(FSlateColor(FLinearColor(0.96f, 0.89f, 0.28f, 1.0f)));
    SetTextSize(PlayerMarker, 17);
    if (UCanvasPanelSlot* Slot = MapCanvas->AddChildToCanvas(PlayerMarker))
    {
        Slot->SetPosition(FVector2D(MapLeft + MapWidth * 0.5f, MapTop + MapHeight * 0.5f));
        Slot->SetSize(FVector2D(120.0f, 28.0f));
    }

    PlayerCoordinates = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TacticalMapPlayerCoordinates"));
    PlayerCoordinates->SetColorAndOpacity(FSlateColor(FLinearColor(0.72f, 0.77f, 0.80f, 1.0f)));
    SetTextSize(PlayerCoordinates, 15);
    if (UCanvasPanelSlot* Slot = MapCanvas->AddChildToCanvas(PlayerCoordinates))
    {
        Slot->SetPosition(FVector2D(70.0f, 620.0f));
        Slot->SetSize(FVector2D(850.0f, 30.0f));
    }

    return Root->TakeWidget();
}

void UOCTacticalMapWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    const APlayerController* PC = GetOwningPlayer();
    const APawn* Pawn = PC ? PC->GetPawn() : nullptr;
    if (!Pawn || !PlayerMarker) return;

    const FVector Location = Pawn->GetActorLocation();
    if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(PlayerMarker->Slot))
    {
        Slot->SetPosition(WorldToMap(Location));
    }
    if (PlayerCoordinates)
    {
        PlayerCoordinates->SetText(FText::FromString(FString::Printf(
            TEXT("ПОЗИЦІЯ: X %.0f · Y %.0f · Z %.0f"), Location.X, Location.Y, Location.Z)));
    }
}

FVector2D UOCTacticalMapWidget::WorldToMap(const FVector& WorldLocation) const
{
    const float SpanX = FMath::Max(1.0f, WorldMax.X - WorldMin.X);
    const float SpanY = FMath::Max(1.0f, WorldMax.Y - WorldMin.Y);
    const float NX = FMath::Clamp((WorldLocation.X - WorldMin.X) / SpanX, 0.0f, 1.0f);
    const float NY = FMath::Clamp((WorldLocation.Y - WorldMin.Y) / SpanY, 0.0f, 1.0f);
    return FVector2D(MapLeft + NX * (MapWidth - 80.0f), MapTop + (1.0f - NY) * (MapHeight - 45.0f));
}

void UOCTacticalMapWidget::AddLandmarkMarker(const FString& Label, const FVector& WorldLocation)
{
    if (!WidgetTree || !MapCanvas) return;
    UTextBlock* Marker = WidgetTree->ConstructWidget<UTextBlock>();
    Marker->SetText(FText::FromString(FString::Printf(TEXT("• %s"), *Label)));
    Marker->SetColorAndOpacity(FSlateColor(FLinearColor(0.76f, 0.83f, 0.86f, 1.0f)));
    SetTextSize(Marker, 14);
    if (UCanvasPanelSlot* Slot = MapCanvas->AddChildToCanvas(Marker))
    {
        Slot->SetPosition(WorldToMap(WorldLocation));
        Slot->SetSize(FVector2D(150.0f, 24.0f));
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

    InWorld.GetTimerManager().SetTimer(
        InputPollTimer,
        this,
        &UOCTacticalMapSubsystem::PollInput,
        0.025f,
        true,
        0.05f);
}

void UOCTacticalMapSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(InputPollTimer);
    }
    if (MapWidget)
    {
        MapWidget->RemoveFromParent();
        MapWidget = nullptr;
    }
    RemappedCharacter.Reset();
    bMapOpen = false;
    Super::Deinitialize();
}

void UOCTacticalMapSubsystem::PollInput()
{
    UWorld* World = GetWorld();
    AOCPlayerController* PC = World ? Cast<AOCPlayerController>(World->GetFirstPlayerController()) : nullptr;
    if (!PC || !PC->IsLocalController()) return;

    AOCCharacter* Character = Cast<AOCCharacter>(PC->GetPawn());
    if (Character && RemappedCharacter.Get() != Character)
    {
        EnsureExclusiveMapBinding(*Character);
        RemappedCharacter = Character;
    }

    const bool bMDown = PC->IsInputKeyDown(EKeys::M);
    const bool bEscapeDown = PC->IsInputKeyDown(EKeys::Escape);
    const bool bMPressed = bMDown && !bPreviousMDown;
    const bool bEscapePressed = bEscapeDown && !bPreviousEscapeDown;
    bPreviousMDown = bMDown;
    bPreviousEscapeDown = bEscapeDown;

    if (bMapOpen)
    {
        if (bMPressed || bEscapePressed || !Character)
        {
            CloseMap(*PC);
        }
        return;
    }

    if (bMPressed && Character && CanOpenMap(*PC))
    {
        OpenMap(*PC);
    }
}

void UOCTacticalMapSubsystem::EnsureExclusiveMapBinding(AOCCharacter& Character)
{
    UInputMappingContext* CharacterContext = FindObject<UInputMappingContext>(&Character, TEXT("IMC_RuntimeDefault"));
    UInputAction* TrapAction = FindObject<UInputAction>(&Character, TEXT("IA_DeployTrap"));
    if (!CharacterContext || !TrapAction)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("Tactical map: could not find character deploy-trap mapping; M exclusivity is not proven."));
        return;
    }

    CharacterContext->UnmapKey(TrapAction, EKeys::M);
    CharacterContext->UnmapKey(TrapAction, EKeys::V);
    CharacterContext->MapKey(TrapAction, EKeys::V);
    UE_LOG(LogTemp, Display, TEXT("Tactical map owns M exclusively; DeployTrap moved from M to V."));
}

bool UOCTacticalMapSubsystem::CanOpenMap(const AOCPlayerController& PlayerController) const
{
    return !PlayerController.IsFrontendMenuVisible() &&
        !PlayerController.IsDeploymentPanelVisible() &&
        !PlayerController.IsAdminPanelVisible() &&
        !PlayerController.IsChatInputActive() &&
        !PlayerController.IsSettingsVisible();
}

void UOCTacticalMapSubsystem::OpenMap(AOCPlayerController& PlayerController)
{
    if (bMapOpen || !CanOpenMap(PlayerController)) return;

    MapWidget = CreateWidget<UOCTacticalMapWidget>(&PlayerController, UOCTacticalMapWidget::StaticClass());
    if (!MapWidget) return;

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

void UOCTacticalMapSubsystem::CloseMap(AOCPlayerController& PlayerController)
{
    if (MapWidget)
    {
        MapWidget->RemoveFromParent();
        MapWidget = nullptr;
    }

    PlayerController.ResetIgnoreMoveInput();
    PlayerController.ResetIgnoreLookInput();
    PlayerController.bShowMouseCursor = false;
    PlayerController.SetInputMode(FInputModeGameOnly());
    PlayerController.UIApplyLocalPreferences();
    bMapOpen = false;
}
