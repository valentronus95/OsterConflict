#include "OCPass45ImportedHUDSubsystem.h"

#include "OCLocalInboxRuntimeSubsystem.h"

#include "Blueprint/UserWidget.h"
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

    // Validation mode owns its HUD through OCLocalInboxRuntimeSubsystem.
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

    // Normal gameplay accepts a real UUserWidget only. The old texture fallback stretched an arbitrary imported
    // texture over the viewport and produced the giant cyan/blue rectangle seen in runtime. A missing widget must
    // fail open to the existing Canvas HUD instead of inventing a fullscreen overlay.
    if (UClass* HUDClass = UOCLocalInboxRuntimeSubsystem::LoadHUDWidgetClass())
    {
        if (UUserWidget* Widget = CreateWidget<UUserWidget>(PC, TSubclassOf<UUserWidget>(HUDClass)))
        {
            Widget->AddToViewport(20);
            ImportedHUDWidget = Widget;
            World->GetTimerManager().ClearTimer(BindTimer);
            UE_LOG(LogTemp, Display,
                TEXT("GAME_RECOVERY_HUD_WIDGET_READY class=%s owner=pass45_imported_hud normal_gameplay=1 texture_fallback=0"),
                *HUDClass->GetPathName());
            UE_LOG(LogTemp, Display,
                TEXT("PASS45_IMPORTED_HUD_READY safe_user_widget=1 texture_overlay=0 attempts=%d runtime_acceptance=0"),
                BindAttempts);
            return;
        }
    }

    if (BindAttempts >= 12)
    {
        World->GetTimerManager().ClearTimer(BindTimer);
        UE_LOG(LogTemp, Warning,
            TEXT("GAME_RECOVERY_HUD_WIDGET_UNAVAILABLE attempts=%d texture_fallback=0 legacy_hud_preserved=1"),
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
