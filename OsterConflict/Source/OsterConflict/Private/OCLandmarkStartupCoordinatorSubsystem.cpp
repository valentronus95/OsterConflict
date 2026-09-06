#include "OCLandmarkStartupCoordinatorSubsystem.h"

#include "OCGameMode.h"
#include "OCPlayerController.h"
#include "OCR137MuseumPhotoModelSubsystem.h"
#include "OCR138MuseumInteractiveArchitectureSubsystem.h"
#include "OCR139MuseumMainDoorReplacementSubsystem.h"
#include "OCR140MuseumFacadeDetailSubsystem.h"
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
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

namespace
{
    const FName MuseumExteriorTag(TEXT("R137_MuseumPhotoModel"));
}

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

    StartupStageIndex = 0;
    bHistoricalTimersCancelled = false;
    bStartupComplete = false;
    bDeferredLogWritten = false;

    // Wait one frame so every historical landmark subsystem has registered its old delayed timer.
    // The first coordinator step cancels those timers before any authored package work is allowed.
    ScheduleStartupStep(InWorld, 0.0f);
}

void UOCLandmarkStartupCoordinatorSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(StartupStepTimerHandle);
    }
    StartupStepTimerHandle.Invalidate();
    Super::Deinitialize();
}

void UOCLandmarkStartupCoordinatorSubsystem::ScheduleStartupStep(UWorld& World, const float DelaySeconds)
{
    TWeakObjectPtr<UWorld> WeakWorld(&World);
    FTimerDelegate Delegate = FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
    {
        if (UWorld* DeferredWorld = WeakWorld.Get())
        {
            RunAuthoritativeStartup(*DeferredWorld);
        }
    });

    World.GetTimerManager().ClearTimer(StartupStepTimerHandle);
    if (DelaySeconds <= 0.0f)
    {
        World.GetTimerManager().SetTimerForNextTick(Delegate);
        return;
    }

    World.GetTimerManager().SetTimer(StartupStepTimerHandle, Delegate, DelaySeconds, false);
}

void UOCLandmarkStartupCoordinatorSubsystem::CancelHistoricalStageTimers(UWorld& World)
{
    if (bHistoricalTimersCancelled) return;

    FTimerManager& Timers = World.GetTimerManager();
    if (UOCR137MuseumPhotoModelSubsystem* Stage = World.GetSubsystem<UOCR137MuseumPhotoModelSubsystem>())
        Timers.ClearAllTimersForObject(Stage);
    if (UOCR138MuseumInteractiveArchitectureSubsystem* Stage = World.GetSubsystem<UOCR138MuseumInteractiveArchitectureSubsystem>())
        Timers.ClearAllTimersForObject(Stage);
    if (UOCR139MuseumMainDoorReplacementSubsystem* Stage = World.GetSubsystem<UOCR139MuseumMainDoorReplacementSubsystem>())
        Timers.ClearAllTimersForObject(Stage);
    if (UOCR140MuseumFacadeDetailSubsystem* Stage = World.GetSubsystem<UOCR140MuseumFacadeDetailSubsystem>())
        Timers.ClearAllTimersForObject(Stage);
    if (UOCR142MuseumEntranceDetailSubsystem* Stage = World.GetSubsystem<UOCR142MuseumEntranceDetailSubsystem>())
        Timers.ClearAllTimersForObject(Stage);
    if (UOCR143MuseumSiteVegetationSubsystem* Stage = World.GetSubsystem<UOCR143MuseumSiteVegetationSubsystem>())
        Timers.ClearAllTimersForObject(Stage);
    if (UOCR144MuseumRearExteriorDetailSubsystem* Stage = World.GetSubsystem<UOCR144MuseumRearExteriorDetailSubsystem>())
        Timers.ClearAllTimersForObject(Stage);
    if (UOCR145MuseumTreeLayoutSubsystem* Stage = World.GetSubsystem<UOCR145MuseumTreeLayoutSubsystem>())
        Timers.ClearAllTimersForObject(Stage);
    if (UOCR140SilpoPhotoModelSubsystem* Stage = World.GetSubsystem<UOCR140SilpoPhotoModelSubsystem>())
        Timers.ClearAllTimersForObject(Stage);
    if (UOCR141SilpoDetailSubsystem* Stage = World.GetSubsystem<UOCR141SilpoDetailSubsystem>())
        Timers.ClearAllTimersForObject(Stage);
    if (UOCR142SilpoInteriorDetailSubsystem* Stage = World.GetSubsystem<UOCR142SilpoInteriorDetailSubsystem>())
        Timers.ClearAllTimersForObject(Stage);
    if (UOCR143SilpoFacadeIdentitySubsystem* Stage = World.GetSubsystem<UOCR143SilpoFacadeIdentitySubsystem>())
        Timers.ClearAllTimersForObject(Stage);
    if (UOCR146CultureHousePhotoModelSubsystem* Stage = World.GetSubsystem<UOCR146CultureHousePhotoModelSubsystem>())
        Timers.ClearAllTimersForObject(Stage);

    bHistoricalTimersCancelled = true;
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_LANDMARK_STARTUP_TIMERS_CANCELLED menu_safe=1 synchronous_next_tick_chain=0"));
}

bool UOCLandmarkStartupCoordinatorSubsystem::IsBlockingPreGameUI(UWorld& World) const
{
    if (World.GetNetMode() == NM_DedicatedServer) return false;

    const AOCPlayerController* PC = Cast<AOCPlayerController>(World.GetFirstPlayerController());
    if (!PC || !PC->IsLocalController()) return true;

    return PC->IsFrontendMenuVisible() ||
        PC->IsSettingsVisible() ||
        PC->IsDeploymentPanelVisible() ||
        PC->GetPawn() == nullptr;
}

bool UOCLandmarkStartupCoordinatorSubsystem::IsMuseumExteriorReady(UWorld& World) const
{
    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        const AActor* Actor = *It;
        if (Actor && Actor->ActorHasTag(MuseumExteriorTag)) return true;
    }
    return false;
}

void UOCLandmarkStartupCoordinatorSubsystem::RunAuthoritativeStartup(UWorld& World)
{
    if (bStartupComplete || !World.IsGameWorld()) return;

    // This runs after every landmark subsystem's OnWorldBeginPlay, so one cancellation is enough and
    // does not erase timers intentionally created by stages after their authoritative build runs.
    CancelHistoricalStageTimers(World);

    if (IsBlockingPreGameUI(World))
    {
        if (!bDeferredLogWritten)
        {
            bDeferredLogWritten = true;
            UE_LOG(LogTemp, Display,
                TEXT("PASS45_LANDMARK_STARTUP_DEFERRED blocking_ui=1 game_thread_materialization=0 team_input_free=1"));
        }
        ScheduleStartupStep(World, DeferredStartupRetrySeconds);
        return;
    }

    // On playable worlds the deployment-stability subsystem asynchronously preloads the R13.7 museum
    // exterior before calling its existing build method. Never defeat that by calling the same package-
    // loading build synchronously from this coordinator. Dedicated servers have no UI/message-pump risk
    // and still need the collision-bearing exterior, so they retain the authoritative direct build.
    if (StartupStageIndex == 0)
    {
        if (World.GetNetMode() == NM_DedicatedServer)
        {
            if (UOCR137MuseumPhotoModelSubsystem* Stage = World.GetSubsystem<UOCR137MuseumPhotoModelSubsystem>())
            {
                Stage->RunAuthoritativeBuildNow(World);
            }
        }
        else if (!IsMuseumExteriorReady(World))
        {
            ScheduleStartupStep(World, DeferredStartupRetrySeconds);
            return;
        }

        ++StartupStageIndex;
        ScheduleStartupStep(World, 0.0f);
        return;
    }

    if (RunNextStartupStage(World)) return;
    ScheduleStartupStep(World, 0.0f);
}

bool UOCLandmarkStartupCoordinatorSubsystem::RunNextStartupStage(UWorld& World)
{
    const bool bHasGameplayAuthority = World.GetNetMode() != NM_Client;

    switch (StartupStageIndex)
    {
    case 1:
        if (UOCR138MuseumInteractiveArchitectureSubsystem* Stage = World.GetSubsystem<UOCR138MuseumInteractiveArchitectureSubsystem>())
            Stage->RunAuthoritativeUpgradeNow(World);
        break;
    case 2:
        if (bHasGameplayAuthority)
        {
            if (UOCR139MuseumMainDoorReplacementSubsystem* Stage = World.GetSubsystem<UOCR139MuseumMainDoorReplacementSubsystem>())
                Stage->RunAuthoritativeDetailNow(World);
        }
        break;
    case 3:
        if (UOCR140MuseumFacadeDetailSubsystem* Stage = World.GetSubsystem<UOCR140MuseumFacadeDetailSubsystem>())
            Stage->RunAuthoritativeDetailNow(World);
        break;
    case 4:
        if (UOCR142MuseumEntranceDetailSubsystem* Stage = World.GetSubsystem<UOCR142MuseumEntranceDetailSubsystem>())
            Stage->RunAuthoritativeDetailNow(World);
        break;
    case 5:
        if (UOCR143MuseumSiteVegetationSubsystem* Stage = World.GetSubsystem<UOCR143MuseumSiteVegetationSubsystem>())
            Stage->RunAuthoritativeDetailNow(World);
        break;
    case 6:
        if (UOCR144MuseumRearExteriorDetailSubsystem* Stage = World.GetSubsystem<UOCR144MuseumRearExteriorDetailSubsystem>())
            Stage->RunAuthoritativeDetailNow(World);
        break;
    case 7:
        if (UOCR145MuseumTreeLayoutSubsystem* Stage = World.GetSubsystem<UOCR145MuseumTreeLayoutSubsystem>())
            Stage->RunAuthoritativeDetailNow(World);
        break;
    case 8:
        if (UOCR140SilpoPhotoModelSubsystem* Stage = World.GetSubsystem<UOCR140SilpoPhotoModelSubsystem>())
            Stage->RunAuthoritativeBuildNow(World);
        break;
    case 9:
        if (UOCR141SilpoDetailSubsystem* Stage = World.GetSubsystem<UOCR141SilpoDetailSubsystem>())
            Stage->RunAuthoritativeDetailNow(World);
        break;
    case 10:
        if (UOCR142SilpoInteriorDetailSubsystem* Stage = World.GetSubsystem<UOCR142SilpoInteriorDetailSubsystem>())
            Stage->RunAuthoritativeDetailNow(World);
        break;
    case 11:
        if (UOCR143SilpoFacadeIdentitySubsystem* Stage = World.GetSubsystem<UOCR143SilpoFacadeIdentitySubsystem>())
            Stage->RunAuthoritativeDetailNow(World);
        break;
    case 12:
        if (UOCR146CultureHousePhotoModelSubsystem* Stage = World.GetSubsystem<UOCR146CultureHousePhotoModelSubsystem>())
            Stage->RunAuthoritativeBuildNow(World);
        break;
    default:
        bStartupComplete = true;
        break;
    }

    if (!bStartupComplete)
    {
        ++StartupStageIndex;
        if (StartupStageIndex > 12) bStartupComplete = true;
    }

    if (!bStartupComplete) return false;

    StartupStepTimerHandle.Invalidate();
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_LANDMARK_STARTUP_COORDINATED_READY staged_frames=1 deployment_blocking_work=0 museum_async_owner=1 delayed_stage_timers_cancelled=1 legacy_core_recovery=0 destructive_visibility_rebuild=0"));
    UE_LOG(LogTemp, Display,
        TEXT("Landmark startup coordinator completed: Museum/Silpo/Culture authoritative stages were deferred until gameplay and released one stage per frame."));
    return true;
}
