#include "OCR13TravelLoadingSubsystem.h"

#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "UObject/CoreUObjectDelegates.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"

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
    if (!LoadedWorld) return;
    HideOverlay();
}

void UOCR13TravelLoadingSubsystem::ShowOverlay()
{
    if (LoadingWidget.IsValid()) return;

    UGameInstance* GameInstance = GetGameInstance();
    UGameViewportClient* Viewport = GameInstance ? GameInstance->GetGameViewportClient() : nullptr;
    if (!Viewport) return;

    LoadingWidget =
        SNew(SBorder)
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Center)
        .BorderBackgroundColor(FLinearColor(0.018f, 0.022f, 0.027f, 1.0f))
        [
            SNew(STextBlock)
            .Text(FText::FromString(TEXT("OSTER CONFLICT\nЗАВАНТАЖЕННЯ")))
            .Justification(ETextJustify::Center)
        ];

    Viewport->AddViewportWidgetContent(LoadingWidget.ToSharedRef(), 10000);
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
