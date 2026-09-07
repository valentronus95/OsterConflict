#include "OCPass45ImportedHUDSubsystem.h"

#include "Engine/World.h"

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

    // Runtime rejection 2026-09-05:
    // The generic CrosshairFreePack resolver selected a non-crosshair icon in the live game and
    // the raw Slate overlay held an FSlateBrush whose UObject resource was not GC-owned. The live
    // symptom was a flashing vest-like icon followed by a Slate/CoreUObject UObjectArray assertion.
    // Keep the proven legacy HUD/crosshair until an exact authored crosshair asset is explicitly
    // bound and owned by a UPROPERTY-backed widget. Do not add any raw viewport Slate overlay here.
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_IMPORTED_HUD_RETIRED reason=unsafe_generic_texture_binding legacy_hud_preserved=1 raw_slate_overlay=0 runtime_acceptance=0"));
}

void UOCPass45ImportedHUDSubsystem::Deinitialize()
{
    // No viewport Slate content is owned by this subsystem after the runtime rejection above.
    CrosshairOverlay.Reset();
    CrosshairBrush.Reset();
    bSuppressedLegacyCrosshair = false;
    Super::Deinitialize();
}
