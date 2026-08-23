#include "OCTacticalMapPlayerMarkerGuardSubsystem.h"

#include "OCTacticalMapSubsystem.h"

#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "UObject/UObjectIterator.h"

namespace
{
    constexpr float MarkerPollSeconds = 0.15f;
}

bool UOCTacticalMapPlayerMarkerGuardSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCTacticalMapPlayerMarkerGuardSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (InWorld.GetNetMode() == NM_DedicatedServer) return;
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    InWorld.GetTimerManager().SetTimer(
        MarkerPollTimer,
        this,
        &UOCTacticalMapPlayerMarkerGuardSubsystem::RefreshPlayerMarkerPriority,
        MarkerPollSeconds,
        true,
        MarkerPollSeconds);
}

void UOCTacticalMapPlayerMarkerGuardSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(MarkerPollTimer);
    LastAdjustedWidget.Reset();
    Super::Deinitialize();
}

void UOCTacticalMapPlayerMarkerGuardSubsystem::RefreshPlayerMarkerPriority()
{
    UWorld* World = GetWorld();
    if (!World) return;

    const UOCTacticalMapSubsystem* MapSubsystem = World->GetSubsystem<UOCTacticalMapSubsystem>();
    if (!MapSubsystem || !MapSubsystem->IsMapOpen())
    {
        LastAdjustedWidget.Reset();
        return;
    }

    UOCTacticalMapWidget* ActiveWidget = nullptr;
    for (TObjectIterator<UOCTacticalMapWidget> It; It; ++It)
    {
        UOCTacticalMapWidget* Candidate = *It;
        if (!IsValid(Candidate) || Candidate->GetWorld() != World || !Candidate->IsInViewport()) continue;
        ActiveWidget = Candidate;
        break;
    }
    if (!ActiveWidget || LastAdjustedWidget.Get() == ActiveWidget) return;

    UTextBlock* Marker = Cast<UTextBlock>(ActiveWidget->GetWidgetFromName(TEXT("TacticalMapPlayerMarker")));
    if (!Marker)
    {
        UE_LOG(LogTemp, Warning, TEXT("PASS35_TACTICAL_PLAYER_MARKER_NOT_FOUND"));
        return;
    }

    Marker->SetText(FText::FromString(TEXT("▲")));
    Marker->SetColorAndOpacity(FSlateColor(FLinearColor(0.46f, 1.0f, 0.22f, 1.0f)));
    Marker->SetShadowOffset(FVector2D(1.5f, 1.5f));
    Marker->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.95f));
    Marker->SetVisibility(ESlateVisibility::HitTestInvisible);
    Marker->SetRenderOpacity(1.0f);

    FSlateFontInfo Font = Marker->GetFont();
    Font.Size = 26;
    Marker->SetFont(Font);

    if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(Marker->Slot))
    {
        Slot->SetZOrder(60);
        Slot->SetSize(FVector2D(44.0f, 44.0f));
    }

    LastAdjustedWidget = ActiveWidget;
    UE_LOG(LogTemp, Display,
        TEXT("PASS35_TACTICAL_PLAYER_MARKER_FOREGROUND z=60 size=26 objective_z=22"));
}
