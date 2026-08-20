#include "OCR13TravelLoadingSubsystem.h"

#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "UObject/UObjectGlobals.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
    // R13 still contains several procedural landmark passes. Until those are fully source-authored, keep them behind
    // the loading presentation instead of showing a half-built city and letting the player watch geometry pop/change.
    // The last retained startup cleanup currently runs at 5.85 s (Silpo anti-shimmer correction).
    constexpr float GameplayPresentationSettleSeconds = 6.15f;
}

void UOCR13TravelLoadingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UOCR13TravelLoadingSubsystem::HandlePreLoadMap);
    FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UOCR13TravelLoadingSubsystem::HandlePostLoadMap);
}

void UOCR13TravelLoadingSubsystem::Deinitialize()
{
    FCoreUObjectDelegates::PreLoadMap.RemoveAll(this);
    FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
    HideOverlay();
    Super::Deinitialize();
}

void UOCR13TravelLoadingSubsystem::HandlePreLoadMap(const FString& MapName)
{
    // The startup map can load before the game viewport exists. The visible regression happens on the explicit
    // frontend -> OsterConflict_Runtime gameplay travel, where the GameInstance and viewport both remain alive.
    if (!MapName.Contains(TEXT("OsterConflict_Runtime"))) return;
    ShowOverlay();
}

void UOCR13TravelLoadingSubsystem::HandlePostLoadMap(UWorld* LoadedWorld)
{
    if (!LoadedWorld)
    {
        HideOverlay();
        return;
    }

    // Do not reveal deployment while the procedural city is still mutating. Previously PostLoadMap removed the
    // overlay almost immediately, so museum/Silpo/stadium/repair passes visibly flashed and also competed with the
    // first team-selection interaction. The wait is explicit and deterministic until these passes are refactored out.
    FTimerHandle HideTimer;
    LoadedWorld->GetTimerManager().SetTimer(
        HideTimer,
        FTimerDelegate::CreateWeakLambda(this, [this]() { HideOverlay(); }),
        GameplayPresentationSettleSeconds,
        false);
}

void UOCR13TravelLoadingSubsystem::ShowOverlay()
{
    if (LoadingWidget.IsValid()) return;

    UGameInstance* GameInstance = GetGameInstance();
    UGameViewportClient* Viewport = GameInstance ? GameInstance->GetGameViewportClient() : nullptr;
    if (!Viewport) return;

    LoadingWidget =
        SNew(SOverlay)
        + SOverlay::Slot()
        .HAlign(HAlign_Fill)
        .VAlign(VAlign_Fill)
        [
            SNew(SBorder)
            .BorderBackgroundColor(FLinearColor(0.018f, 0.022f, 0.027f, 1.0f))
        ]
        + SOverlay::Slot()
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Center)
        [
            SNew(STextBlock)
            .Text(FText::FromString(TEXT("OSTER CONFLICT\nЗАВАНТАЖЕННЯ")))
            .Justification(ETextJustify::Center)
        ];

    Viewport->AddViewportWidgetContent(LoadingWidget.ToSharedRef(), 30000);
}

void UOCR13TravelLoadingSubsystem::HideOverlay()
{
    if (!LoadingWidget.IsValid()) return;

    UGameInstance* GameInstance = GetGameInstance();
    UGameViewportClient* Viewport = GameInstance ? GameInstance->GetGameViewportClient() : nullptr;
    if (Viewport)
    {
        Viewport->RemoveViewportWidgetContent(LoadingWidget.ToSharedRef());
    }
    LoadingWidget.Reset();
}
