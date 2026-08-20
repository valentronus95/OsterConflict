#include "OCR13MuseumStadiumPhotoFidelitySubsystem.h"

#include "OCGameMode.h"

#include "Engine/World.h"

bool UOCR13MuseumStadiumPhotoFidelitySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13MuseumStadiumPhotoFidelitySubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    // Retired compatibility subsystem. It deliberately schedules nothing and creates no geometry.
    ApplyPhotoFidelity(InWorld);
}

void UOCR13MuseumStadiumPhotoFidelitySubsystem::ApplyPhotoFidelity(UWorld& World)
{
    (void)World;
    UE_LOG(LogTemp, Display,
        TEXT("R13 museum/stadium legacy photo-fidelity pass retired: Stadion Oster presentation ownership is exclusive to OCR13StadiumSurfaceSubsystem."));
}