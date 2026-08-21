#include "OCR137MuseumSiteReplacementSubsystem.h"

#include "Engine/World.h"

bool UOCR137MuseumSiteReplacementSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    // Disabled 2026-08-21. The old pass woke at 4.95 s and mutated the museum site immediately
    // before the separate 5.10 s museum rebuild. UOCR137MuseumPhotoModelSubsystem already performs
    // the required scoped source cleanup itself, so this second delayed owner only caused visible
    // late mutation/flicker and complicated landmark ownership.
    return false;
}

void UOCR137MuseumSiteReplacementSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
}

void UOCR137MuseumSiteReplacementSubsystem::PrepareMuseumSite(UWorld& World)
{
    // Intentionally retired. Museum site cleanup belongs to the authoritative museum build path.
}
