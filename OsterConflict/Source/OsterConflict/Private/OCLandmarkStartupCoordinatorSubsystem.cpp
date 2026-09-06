#include "OCLandmarkStartupCoordinatorSubsystem.h"

#include "OCGameMode.h"
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
#include "HAL/PlatformTime.h"
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

    bInitialized = false;
    StartupStageIndex = 0;
    bHistoricalTimersCancelled = false;
    bStartupComplete = false;
    NextStageWallTimeSeconds = 0.0;

    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    bInitialized = true;
    UE_LOG(LogTemp, Display,
        TEXT("GAME_RECOVERY_WORLD_PREP_BEGIN pre_spawn=1 tick_when_paused=1 staged_materialization=1"));
}

void UOCLandmarkStartupCoordinatorSubsystem::Tick(float DeltaTime)
{
    (void)DeltaTime;
    if (!bInitialized || bStartupComplete) return;

    UWorld* World = GetWorld();
    if (!World || !World->IsGameWorld()) return;

    const double Now = FPlatformTime::Seconds();
    if (Now < NextStageWallTimeSeconds) return;
    NextStageWallTimeSeconds = Now + StageIntervalSeconds;

    RunAuthoritativeStartup(*World);
}

TStatId UOCLandmarkStartupCoordinatorSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCLandmarkStartupCoordinatorSubsystem, STATGROUP_Tickables);
}

float UOCLandmarkStartupCoordinatorSubsystem::GetStartupProgress() const
{
    if (!bInitialized) return 0.0f;
    if (bStartupComplete) return 1.0f;
    return FMath::Clamp(static_cast<float>(StartupStageIndex) / static_cast<float>(TotalStartupStages), 0.0f, 0.99f);
}

void UOCLandmarkStartupCoordinatorSubsystem::Deinitialize()
{
    bInitialized = false;
    NextStageWallTimeSeconds = 0.0;
    Super::Deinitialize();
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
        TEXT("GAME_RECOVERY_WORLD_PREP_TIMERS_CANCELLED duplicate_startup_timers=0"));
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

    // Cancel every historical delayed owner once, then own the whole landmark startup sequence here.
    // Unlike the old implementation this coordinator keeps running while the world is paused, so the
    // deployment screen absorbs preparation and the player is not used as a loading screen.
    CancelHistoricalStageTimers(World);

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
            // The deployment-stability subsystem owns asynchronous preload + R13.7 build on playable worlds.
            return;
        }

        ++StartupStageIndex;
        UE_LOG(LogTemp, Display,
            TEXT("GAME_RECOVERY_WORLD_PREP_STAGE stage=1/%d museum_exterior_ready=1"), TotalStartupStages);
        return;
    }

    RunNextStartupStage(World);
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
        if (StartupStageIndex >= TotalStartupStages) bStartupComplete = true;
    }

    if (!bStartupComplete)
    {
        UE_LOG(LogTemp, Verbose,
            TEXT("GAME_RECOVERY_WORLD_PREP_STAGE stage=%d/%d"), StartupStageIndex, TotalStartupStages);
        return false;
    }

    UE_LOG(LogTemp, Display,
        TEXT("GAME_RECOVERY_WORLD_READY stages=%d pre_spawn=1 post_spawn_landmark_materialization=0"), TotalStartupStages);
    return true;
}
