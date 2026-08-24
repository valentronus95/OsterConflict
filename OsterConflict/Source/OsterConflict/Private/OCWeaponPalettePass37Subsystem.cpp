#include "OCWeaponPalettePass37Subsystem.h"

#include "OCGameMode.h"
#include "Engine/World.h"

bool UOCWeaponPalettePass37Subsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCWeaponPalettePass37Subsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    if (InWorld.GetNetMode() == NM_DedicatedServer) return;
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    // Pass 44: user runtime disproved both the old forced palette and the later "placeholder-only"
    // BasicShapeMaterial recovery. A missing/default authored material must stay visible as a content gap;
    // runtime is not allowed to fabricate a grey/orange replacement and then call the weapon ready.
    // The dedicated UOCRealWeaponFallbackSubsystem now audits material truth and emits PASS44 gap evidence.
    UE_LOG(LogTemp, Display,
        TEXT("PASS44_WEAPON_PALETTE_MUTATION_DISABLED runtime_material_creation=0 set_material_calls=0 polling=0 authored_material_truth=1"));
    UE_LOG(LogTemp, Display,
        TEXT("PASS38_WEAPON_PALETTE_SCAN_STOPPED reason=retired_by_pass44 passes=0 permanent_scan=0"));
}
