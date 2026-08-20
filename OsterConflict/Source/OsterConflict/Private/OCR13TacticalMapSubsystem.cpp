#include "OCR13TacticalMapSubsystem.h"

#include "OCPlayerController.h"
#include "OCR13TacticalMapWidget.h"

#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "InputCoreTypes.h"

bool UOCR13TacticalMapSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13TacticalMapSubsystem::Tick(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World || DeltaTime <= 0.0f) return;

    AOCPlayerController* PC = Cast<AOCPlayerController>(World->GetFirstPlayerController());
    if (!PC || !PC->IsLocalController())
    {
        if (bMapVisible) SetMapVisible(false);
        return;
    }

    const bool bMapKeyDown = PC->IsInputKeyDown(EKeys::M);
    const bool bBlockedByModalUI = PC->IsFrontendMenuVisible() || PC->IsDeploymentPanelVisible() ||
        PC->IsSettingsVisible() || PC->IsAdminPanelVisible() || PC->IsChatInputActive();

    if (bBlockedByModalUI || !PC->GetPawn())
    {
        if (bMapVisible) SetMapVisible(false);
    }
    else if (bMapKeyDown && !bMapKeyWasDown)
    {
        SetMapVisible(!bMapVisible);
    }

    // EKeys::M is the physical Latin M key; on a Ukrainian keyboard layout this is the same key labelled/typed Ь.
    bMapKeyWasDown = bMapKeyDown;
}

void UOCR13TacticalMapSubsystem::SetMapVisible(const bool bVisible)
{
    if (bMapVisible == bVisible && (!bVisible || TacticalMapWidget.IsValid())) return;
    bMapVisible = bVisible;

    if (!bVisible)
    {
        if (UOCR13TacticalMapWidget* Existing = TacticalMapWidget.Get()) Existing->RemoveFromParent();
        TacticalMapWidget.Reset();
        return;
    }

    UWorld* World = GetWorld();
    AOCPlayerController* PC = World ? Cast<AOCPlayerController>(World->GetFirstPlayerController()) : nullptr;
    if (!PC || !PC->IsLocalController())
    {
        bMapVisible = false;
        return;
    }

    UOCR13TacticalMapWidget* Widget = CreateWidget<UOCR13TacticalMapWidget>(
        PC, UOCR13TacticalMapWidget::StaticClass());
    if (!Widget)
    {
        bMapVisible = false;
        return;
    }

    Widget->AddToViewport(850);
    TacticalMapWidget = Widget;
}

TStatId UOCR13TacticalMapSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCR13TacticalMapSubsystem, STATGROUP_Tickables);
}
