#include "OCPass45ImportedHUDSubsystem.h"

#include "OCGameMode.h"
#include "OCPass45LocalAssetResolver.h"
#include "OCPlayerUserSettings.h"

#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/SOverlay.h"

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
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    UTexture2D* CrosshairTexture = OCPass45FindLocalTexture(
        { FName(TEXT("/Game/CrosshairFreePack")) },
        { TEXT("crosshair"), TEXT("reticle"), TEXT("dot") });
    if (!CrosshairTexture)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_IMPORTED_HUD_CONTENT_GAP package=/Game/CrosshairFreePack local_crosshair_overlay=0 legacy_hud_preserved=1"));
        return;
    }

    UOCPlayerUserSettings* Settings = UOCPlayerUserSettings::Get();
    bOriginalCrosshairSetting = Settings ? Settings->bShowCrosshair : true;
    if (!bOriginalCrosshairSetting)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_IMPORTED_HUD_SKIPPED reason=user_crosshair_disabled preference_preserved=1"));
        return;
    }

    if (!GEngine || !GEngine->GameViewport) return;

    const float HUDScale = Settings ? FMath::Clamp(Settings->HUDScale, 0.75f, 1.25f) : 1.0f;
    CrosshairBrush = MakeShared<FSlateBrush>();
    CrosshairBrush->SetResourceObject(CrosshairTexture);
    CrosshairBrush->ImageSize = FVector2D(36.0f, 36.0f) * HUDScale;
    CrosshairBrush->DrawAs = ESlateBrushDrawType::Image;

    CrosshairOverlay =
        SNew(SOverlay)
        + SOverlay::Slot()
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Center)
        [
            SNew(SImage)
            .Image(CrosshairBrush.Get())
        ];

    GEngine->GameViewport->AddViewportWidgetContent(CrosshairOverlay.ToSharedRef(), 950);
    if (Settings)
    {
        // Presentation-only session override. Never persist this temporary suppression to config.
        Settings->bShowCrosshair = false;
        bSuppressedLegacyCrosshair = true;
    }

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_IMPORTED_HUD_READY asset=%s local_crosshair_overlay=1 legacy_line_crosshair_suppressed_in_memory=%d settings_saved=0 runtime_acceptance=0"),
        *CrosshairTexture->GetPathName(), bSuppressedLegacyCrosshair ? 1 : 0);
}

void UOCPass45ImportedHUDSubsystem::Deinitialize()
{
    if (CrosshairOverlay.IsValid() && GEngine && GEngine->GameViewport)
    {
        GEngine->GameViewport->RemoveViewportWidgetContent(CrosshairOverlay.ToSharedRef());
    }
    CrosshairOverlay.Reset();
    CrosshairBrush.Reset();

    if (bSuppressedLegacyCrosshair)
    {
        if (UOCPlayerUserSettings* Settings = UOCPlayerUserSettings::Get())
        {
            Settings->bShowCrosshair = bOriginalCrosshairSetting;
        }
    }
    bSuppressedLegacyCrosshair = false;
    Super::Deinitialize();
}
