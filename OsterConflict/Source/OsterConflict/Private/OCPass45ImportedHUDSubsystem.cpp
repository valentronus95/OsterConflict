#include "OCPass45ImportedHUDSubsystem.h"

#include "OCLocalInboxHUDOverlayWidget.h"
#include "OCLocalInboxRuntimeSubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "TimerManager.h"

bool UOCPass45ImportedHUDSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && World->GetNetMode() != NM_DedicatedServer &&
        (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCPass45ImportedHUDSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    // Validation mode already owns its HUD through OCLocalInboxRuntimeSubsystem. Normal gameplay used to return
    // before that binding path, leaving only the legacy Canvas text. Bind exactly one GC-owned UUserWidget here.
    if (FParse::Param(FCommandLine::Get(), TEXT("ValidateLocalInbox")))
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_IMPORTED_HUD_DEFERRED validation_mode=1 owner=local_inbox_runtime"));
        return;
    }

    BindAttempts = 0;
    InWorld.GetTimerManager().SetTimer(
        BindTimer,
        this,
        &UOCPass45ImportedHUDSubsystem::BindGameplayHUD,
        0.25f,
        true,
        0.05f);
}

void UOCPass45ImportedHUDSubsystem::BindGameplayHUD()
{
    UWorld* World = GetWorld();
    if (!World) return;

    ++BindAttempts;
    if (IsValid(ImportedHUDWidget))
    {
        World->GetTimerManager().ClearTimer(BindTimer);
        return;
    }

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC)
    {
        if (BindAttempts >= 12)
        {
            World->GetTimerManager().ClearTimer(BindTimer);
            UE_LOG(LogTemp, Error,
                TEXT("GAME_RECOVERY_HUD_BIND_FAIL reason=no_player_controller attempts=%d"), BindAttempts);
        }
        return;
    }

    UUserWidget* Widget = nullptr;
    if (UClass* HUDClass = UOCLocalInboxRuntimeSubsystem::LoadHUDWidgetClass())
    {
        Widget = CreateWidget<UUserWidget>(PC, TSubclassOf<UUserWidget>(HUDClass));
        if (Widget)
        {
            UE_LOG(LogTemp, Display,
                TEXT("GAME_RECOVERY_HUD_WIDGET_READY class=%s owner=pass45_imported_hud normal_gameplay=1"),
                *HUDClass->GetPathName());
        }
    }
    else if (UTexture2D* HUDTexture = UOCLocalInboxRuntimeSubsystem::LoadHUDTexture())
    {
        if (UOCLocalInboxHUDOverlayWidget* Overlay = CreateWidget<UOCLocalInboxHUDOverlayWidget>(
            PC, UOCLocalInboxHUDOverlayWidget::StaticClass()))
        {
            Overlay->SetHUDTexture(HUDTexture);
            Widget = Overlay;
            UE_LOG(LogTemp, Display,
                TEXT("GAME_RECOVERY_HUD_TEXTURE_READY texture=%s owner=pass45_imported_hud normal_gameplay=1"),
                *HUDTexture->GetPathName());
        }
    }

    if (Widget)
    {
        Widget->AddToViewport(20);
        ImportedHUDWidget = Widget;
        World->GetTimerManager().ClearTimer(BindTimer);
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_IMPORTED_HUD_READY safe_user_widget=1 raw_slate_overlay=0 attempts=%d runtime_acceptance=0"),
            BindAttempts);
        return;
    }

    if (BindAttempts >= 12)
    {
        World->GetTimerManager().ClearTimer(BindTimer);
        UE_LOG(LogTemp, Error,
            TEXT("GAME_RECOVERY_HUD_BIND_FAIL reason=no_bound_hud_asset attempts=%d legacy_hud_preserved=1"),
            BindAttempts);
    }
}

void UOCPass45ImportedHUDSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(BindTimer);
    if (IsValid(ImportedHUDWidget)) ImportedHUDWidget->RemoveFromParent();
    ImportedHUDWidget = nullptr;
    BindAttempts = 0;
    Super::Deinitialize();
}
