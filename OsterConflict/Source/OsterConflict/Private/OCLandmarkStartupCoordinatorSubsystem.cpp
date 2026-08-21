#include "OCLandmarkStartupCoordinatorSubsystem.h"

#include "OCGameMode.h"
#include "OCR137MuseumPhotoModelSubsystem.h"
#include "OCR138MuseumInteractiveArchitectureSubsystem.h"
#include "OCR139MuseumMainDoorReplacementSubsystem.h"
#include "OCR140MuseumFacadeDetailSubsystem.h"
#include "OCR141MuseumWindowReplacementSubsystem.h"
#include "OCR142MuseumEntranceDetailSubsystem.h"
#include "OCR143MuseumSiteVegetationSubsystem.h"
#include "OCR144MuseumRearExteriorDetailSubsystem.h"
#include "OCR145MuseumTreeLayoutSubsystem.h"
#include "OCR140SilpoPhotoModelSubsystem.h"
#include "OCR141SilpoDetailSubsystem.h"
#include "OCR142SilpoInteriorDetailSubsystem.h"
#include "OCR143SilpoFacadeIdentitySubsystem.h"
#include "OCR146CultureHousePhotoModelSubsystem.h"

#include "Engine/World.h"
#include "TimerManager.h"

bool UOCLandmarkStartupCoordinatorSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCLandmarkStartupCoordinatorSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    // WorldSubsystem OnWorldBeginPlay ordering is not a contract for GameMode-created actors.
    // One next-tick handoff guarantees SpawnOsterCenterSector and every landmark subsystem have
    // completed BeginPlay and registered their legacy delayed timers before we cancel/run them.
    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    InWorld.GetTimerManager().SetTimerForNextTick(
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get())
            {
                RunAuthoritativeStartup(*World);
            }
        }));
}

void UOCLandmarkStartupCoordinatorSubsystem::RunAuthoritativeStartup(UWorld& World)
{
    FTimerManager& Timers = World.GetTimerManager();
    const bool bHasGameplayAuthority = World.GetNetMode() != NM_Client;

    // Museum. Cancel each historical delayed reveal BEFORE invoking the same existing build method,
    // so timers created intentionally by the immediate build itself are not accidentally erased.
    if (UOCR137MuseumPhotoModelSubsystem* Stage = World.GetSubsystem<UOCR137MuseumPhotoModelSubsystem>())
    {
        Timers.ClearAllTimersForObject(Stage);
        Stage->RunAuthoritativeBuildNow(World);
    }
    if (UOCR138MuseumInteractiveArchitectureSubsystem* Stage = World.GetSubsystem<UOCR138MuseumInteractiveArchitectureSubsystem>())
    {
        Timers.ClearAllTimersForObject(Stage);
        Stage->RunAuthoritativeUpgradeNow(World);
    }

    // These two stages replace replicated gameplay actors and historically never executed on NM_Client.
    if (bHasGameplayAuthority)
    {
        if (UOCR139MuseumMainDoorReplacementSubsystem* Stage = World.GetSubsystem<UOCR139MuseumMainDoorReplacementSubsystem>())
        {
            Timers.ClearAllTimersForObject(Stage);
            Stage->RunAuthoritativeDetailNow(World);
        }
    }

    if (UOCR140MuseumFacadeDetailSubsystem* Stage = World.GetSubsystem<UOCR140MuseumFacadeDetailSubsystem>())
    {
        Timers.ClearAllTimersForObject(Stage);
        Stage->RunAuthoritativeDetailNow(World);
    }

    if (bHasGameplayAuthority)
    {
        if (UOCR141MuseumWindowReplacementSubsystem* Stage = World.GetSubsystem<UOCR141MuseumWindowReplacementSubsystem>())
        {
            Timers.ClearAllTimersForObject(Stage);
            Stage->RunAuthoritativeDetailNow(World);
        }
    }

    if (UOCR142MuseumEntranceDetailSubsystem* Stage = World.GetSubsystem<UOCR142MuseumEntranceDetailSubsystem>())
    {
        Timers.ClearAllTimersForObject(Stage);
        Stage->RunAuthoritativeDetailNow(World);
    }
    if (UOCR143MuseumSiteVegetationSubsystem* Stage = World.GetSubsystem<UOCR143MuseumSiteVegetationSubsystem>())
    {
        Timers.ClearAllTimersForObject(Stage);
        Stage->RunAuthoritativeDetailNow(World);
    }
    if (UOCR144MuseumRearExteriorDetailSubsystem* Stage = World.GetSubsystem<UOCR144MuseumRearExteriorDetailSubsystem>())
    {
        Timers.ClearAllTimersForObject(Stage);
        Stage->RunAuthoritativeDetailNow(World);
    }
    if (UOCR145MuseumTreeLayoutSubsystem* Stage = World.GetSubsystem<UOCR145MuseumTreeLayoutSubsystem>())
    {
        Timers.ClearAllTimersForObject(Stage);
        Stage->RunAuthoritativeDetailNow(World);
    }

    // Silpo. R14.0 remains the shell/interior owner; R14.1-R14.3 are detail-only stages.
    if (UOCR140SilpoPhotoModelSubsystem* Stage = World.GetSubsystem<UOCR140SilpoPhotoModelSubsystem>())
    {
        Timers.ClearAllTimersForObject(Stage);
        Stage->RunAuthoritativeBuildNow(World);
    }
    if (UOCR141SilpoDetailSubsystem* Stage = World.GetSubsystem<UOCR141SilpoDetailSubsystem>())
    {
        Timers.ClearAllTimersForObject(Stage);
        Stage->RunAuthoritativeDetailNow(World);
    }
    if (UOCR142SilpoInteriorDetailSubsystem* Stage = World.GetSubsystem<UOCR142SilpoInteriorDetailSubsystem>())
    {
        Timers.ClearAllTimersForObject(Stage);
        Stage->RunAuthoritativeDetailNow(World);
    }
    if (UOCR143SilpoFacadeIdentitySubsystem* Stage = World.GetSubsystem<UOCR143SilpoFacadeIdentitySubsystem>())
    {
        Timers.ClearAllTimersForObject(Stage);
        Stage->RunAuthoritativeDetailNow(World);
    }

    // Culture House already has one canonical FOCGeoReference owner. Run it in the same startup window.
    if (UOCR146CultureHousePhotoModelSubsystem* Stage = World.GetSubsystem<UOCR146CultureHousePhotoModelSubsystem>())
    {
        Timers.ClearAllTimersForObject(Stage);
        Stage->RunAuthoritativeBuildNow(World);
    }

    UE_LOG(LogTemp, Display,
        TEXT("Landmark startup coordinator completed: Museum/Silpo/Culture authoritative stages ran in one startup pass; historical delayed reveal timers were cancelled; authority-only door/window replacements preserved."));
}