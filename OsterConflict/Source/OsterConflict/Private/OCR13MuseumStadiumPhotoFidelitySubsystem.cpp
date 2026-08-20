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

    // Retired compatibility subsystem.
    // Museum presentation is owned by the dedicated museum owner and Stadion Oster is owned exclusively by
    // UOCR13StadiumSurfaceSubsystem. Do not schedule delayed geometry replacement here: doing so previously caused
    // duplicate stadium actors and visible scene rebuilding several seconds after BeginPlay.
    ApplyPhotoFidelity(InWorld);
}

void UOCR13MuseumStadiumPhotoFidelitySubsystem::ApplyPhotoFidelity(UWorld& World)
{
    (void)World;
    UE_LOG(LogTemp, Display,
        TEXT("R13 museum/stadium legacy photo-fidelity pass retired: Stadion Oster presentation ownership is exclusive to OCR13StadiumSurfaceSubsystem."));
}

void UOCR13MuseumStadiumPhotoFidelitySubsystem::SuppressLegacyMuseumPresentation(UWorld& World)
{
    (void)World;
}

void UOCR13MuseumStadiumPhotoFidelitySubsystem::SuppressLegacyStadiumPresentation(UWorld& World)
{
    (void)World;
}

void UOCR13MuseumStadiumPhotoFidelitySubsystem::BuildMuseum(UWorld& World)
{
    (void)World;
}

void UOCR13MuseumStadiumPhotoFidelitySubsystem::BuildStadium(UWorld& World)
{
    (void)World;
}
