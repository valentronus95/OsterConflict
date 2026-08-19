#include "OCR13ResidentialInfillSubsystem.h"

#include "OCGameMode.h"

#include "Engine/World.h"

bool UOCR13ResidentialInfillSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13ResidentialInfillSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    // LOCATION-FIRST R13:
    // The previous implementation inferred new houses from road length and alternating roadside samples.
    // That is useful for a generic village generator, but it invents Oster geometry and makes later
    // reference-driven reconstruction harder to reason about. Keep this subsystem as a migration stub
    // until explicit street/block placement fully replaces the old R13 runtime patch stack.
    BuildResidentialInfill(InWorld);
}

void UOCR13ResidentialInfillSubsystem::BuildResidentialInfill(UWorld& World)
{
    if (bApplied) return;
    bApplied = true;

    UE_LOG(LogTemp, Display,
        TEXT("R13 location-first: procedural residential infill disabled; houses must come from explicit Oster street/block placement."));

    (void)World;
}
