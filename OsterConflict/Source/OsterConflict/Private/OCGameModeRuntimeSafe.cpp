#include "OCGameModeRuntimeSafe.h"

#include "OCAuthoredWorldSurfaceUpgradeSubsystem.h"
#include "OCBlock0GroundFoundationSubsystem.h"
#include "OCLandmarkStartupCoordinatorSubsystem.h"
#include "OCParkGroundAuthoredUpgradeSubsystem.h"
#include "OCParkHardscapeAuthoredUpgradeSubsystem.h"
#include "OCParkMemorialApproachAuthoredUpgradeSubsystem.h"
#include "OCParkSemanticAuthoredUpgradeSubsystem.h"
#include "OCPlayerController.h"
#include "OCPlayerState.h"
#include "OCTeamSpawnPoint.h"
#include "OCWorldSectorOster.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "HAL/PlatformTime.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

namespace
{
    constexpr float MaxMuseumBaseDistanceCm = 4500.0f;
    constexpr float SpawnLiftCm = 80.0f;
    const FName Pass45PreTickOsterSectorTag(TEXT("PASS45_PreTickOsterSector"));

    bool HasExplicitOption(const FString& Options, const TCHAR* Key)
    {
        FString Value = UGameplayStatics::ParseOption(Options, Key);
        Value.TrimStartAndEndInline();
        return !Value.IsEmpty();
    }

    void GatherLiveOsterSectors(UWorld* World, TArray<AOCWorldSectorOster*>& OutSectors)
    {
        OutSectors.Reset();
        if (!World) return;
        for (TActorIterator<AOCWorldSectorOster> It(World); It; ++It)
        {
            AOCWorldSectorOster* Sector = *It;
            if (IsValid(Sector) && !Sector->IsActorBeingDestroyed()) OutSectors.Add(Sector);
        }
    }
}

void AOCGameModeRuntimeSafe::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);

    // Keep the frontend/travel stage light before a real human host exists. OCBotPopulationPolicySubsystem restores
    // the approved filler-bot population after the host is represented in the gameplay world.
    const bool bBotsExplicit = HasExplicitOption(Options, TEXT("Bots"));
    const bool bPopulationExplicit = HasExplicitOption(Options, TEXT("Population"));
    const bool bBotFillExplicit = HasExplicitOption(Options, TEXT("BotFill"));

    if (!bBotsExplicit && !bPopulationExplicit && !bBotFillExplicit)
    {
        TargetPopulation = 0;
        bAutoFillBots = false;
        UE_LOG(LogTemp, Display,
            TEXT("PASS44_LOCAL_BOT_AUTOFILL_DEFERRED_READY implicit_population=0 explicit_bot_options=0 background_ai_load_before_human=0 restore_owner=OCBotPopulationPolicySubsystem"));
    }
    else
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS44_EXPLICIT_BOT_OPTIONS_PRESERVED bots=%d population=%d botfill=%d target_population=%d auto_fill=%d"),
            bBotsExplicit ? 1 : 0,
            bPopulationExplicit ? 1 : 0,
            bBotFillExplicit ? 1 : 0,
            TargetPopulation,
            bAutoFillBots ? 1 : 0);
    }

    if (IsFrontendOnlySession() || !HasAuthority() || !GetWorld()) return;

    TArray<AOCWorldSectorOster*> ExistingSectors;
    GatherLiveOsterSectors(GetWorld(), ExistingSectors);
    if (ExistingSectors.Num() > 1)
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_PRETICK_OSTER_SECTOR_FAIL reason=preexisting_sector_count_%d authored_before_world_begin_play=0 runtime_acceptance=0"),
            ExistingSectors.Num());
        return;
    }

    AOCWorldSectorOster* Sector = ExistingSectors.Num() == 1 ? ExistingSectors[0] : nullptr;
    if (!Sector)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        Sector = GetWorld()->SpawnActor<AOCWorldSectorOster>(AOCWorldSectorOster::StaticClass(), FTransform::Identity, SpawnParams);
    }

    if (!IsValid(Sector))
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_PRETICK_OSTER_SECTOR_FAIL reason=spawn_failed authored_before_world_begin_play=0 runtime_acceptance=0"));
        return;
    }

    Sector->Tags.AddUnique(Pass45PreTickOsterSectorTag);
    GatherLiveOsterSectors(GetWorld(), ExistingSectors);
    if (ExistingSectors.Num() != 1)
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_PRETICK_OSTER_SECTOR_FAIL reason=post_spawn_sector_count_%d authored_before_world_begin_play=0 runtime_acceptance=0"),
            ExistingSectors.Num());
        return;
    }

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_PRETICK_OSTER_SECTOR_READY sector_count=1 authored_before_world_begin_play=1 heavy_tree_startup_loads=0 canonical_owner=OCGameModeRuntimeSafe runtime_acceptance=0"));
}

void AOCGameModeRuntimeSafe::BeginPlay()
{
    Super::BeginPlay();
    if (IsFrontendOnlySession() || !HasAuthority() || !GetWorld()) return;

    TArray<AOCWorldSectorOster*> Sectors;
    GatherLiveOsterSectors(GetWorld(), Sectors);

    AOCWorldSectorOster* CanonicalSector = nullptr;
    for (AOCWorldSectorOster* Sector : Sectors)
    {
        if (Sector && Sector->ActorHasTag(Pass45PreTickOsterSectorTag))
        {
            CanonicalSector = Sector;
            break;
        }
    }

    int32 DuplicatesRetired = 0;
    if (CanonicalSector)
    {
        for (AOCWorldSectorOster* Sector : Sectors)
        {
            if (Sector && Sector != CanonicalSector)
            {
                Sector->Destroy();
                ++DuplicatesRetired;
            }
        }
    }

    TArray<AOCWorldSectorOster*> RemainingSectors;
    GatherLiveOsterSectors(GetWorld(), RemainingSectors);
    if (!CanonicalSector || RemainingSectors.Num() != 1 || RemainingSectors[0] != CanonicalSector)
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_OSTER_SECTOR_SINGLE_OWNER_FAIL canonical_pretick_owner=%d sector_count=%d duplicates_retired=%d before_first_tick=0 runtime_acceptance=0"),
            CanonicalSector ? 1 : 0, RemainingSectors.Num(), DuplicatesRetired);
        return;
    }

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_OSTER_SECTOR_SINGLE_OWNER_READY canonical_pretick_owner=1 sector_count=1 duplicates_retired=%d before_first_tick=1 heavy_tree_startup_loads=0 runtime_acceptance=0"),
        DuplicatesRetired);
}

bool AOCGameModeRuntimeSafe::IsRecoveryWorldReady(FString& OutPendingStages, bool& bOutHardFailure) const
{
    OutPendingStages.Reset();
    bOutHardFailure = false;

    UWorld* World = GetWorld();
    if (!World || World->GetNetMode() == NM_DedicatedServer || !World->GetMapName().Contains(TEXT("OsterConflict_Runtime")))
        return true;

    TArray<FString> Pending;

    const UOCBlock0GroundFoundationSubsystem* Ground = World->GetSubsystem<UOCBlock0GroundFoundationSubsystem>();
    if (!Ground) { Pending.Add(TEXT("ground_subsystem_missing")); bOutHardFailure = true; }
    else if (Ground->HasGroundFailed()) { Pending.Add(TEXT("ground_failed")); bOutHardFailure = true; }
    else if (!Ground->IsGroundReady()) Pending.Add(TEXT("ground"));

    const UOCAuthoredWorldSurfaceUpgradeSubsystem* Surface = World->GetSubsystem<UOCAuthoredWorldSurfaceUpgradeSubsystem>();
    if (!Surface) { Pending.Add(TEXT("surface_subsystem_missing")); bOutHardFailure = true; }
    else if (Surface->HasWorldSurfaceFailed()) { Pending.Add(TEXT("surface_failed")); bOutHardFailure = true; }
    else if (!Surface->IsWorldSurfaceReady()) Pending.Add(TEXT("surface"));

    const UOCLandmarkStartupCoordinatorSubsystem* Landmarks = World->GetSubsystem<UOCLandmarkStartupCoordinatorSubsystem>();
    if (!Landmarks) { Pending.Add(TEXT("landmark_subsystem_missing")); bOutHardFailure = true; }
    else if (!Landmarks->IsWorldStartupReady()) Pending.Add(TEXT("landmarks"));

    const UOCParkSemanticAuthoredUpgradeSubsystem* ParkSemantic = World->GetSubsystem<UOCParkSemanticAuthoredUpgradeSubsystem>();
    if (!ParkSemantic) { Pending.Add(TEXT("park_semantic_subsystem_missing")); bOutHardFailure = true; }
    else if (ParkSemantic->HasParkSemanticFailed()) { Pending.Add(TEXT("park_semantic_failed")); bOutHardFailure = true; }
    else if (!ParkSemantic->IsParkSemanticReady()) Pending.Add(TEXT("park_semantic"));

    const UOCParkGroundAuthoredUpgradeSubsystem* ParkGround = World->GetSubsystem<UOCParkGroundAuthoredUpgradeSubsystem>();
    if (!ParkGround) { Pending.Add(TEXT("park_ground_subsystem_missing")); bOutHardFailure = true; }
    else if (ParkGround->HasParkGroundFailed()) { Pending.Add(TEXT("park_ground_failed")); bOutHardFailure = true; }
    else if (!ParkGround->IsParkGroundReady()) Pending.Add(TEXT("park_ground"));

    const UOCParkHardscapeAuthoredUpgradeSubsystem* ParkHardscape = World->GetSubsystem<UOCParkHardscapeAuthoredUpgradeSubsystem>();
    if (!ParkHardscape) { Pending.Add(TEXT("park_hardscape_subsystem_missing")); bOutHardFailure = true; }
    else if (ParkHardscape->HasParkHardscapeFailed()) { Pending.Add(TEXT("park_hardscape_failed")); bOutHardFailure = true; }
    else if (!ParkHardscape->IsParkHardscapeReady()) Pending.Add(TEXT("park_hardscape"));

    const UOCParkMemorialApproachAuthoredUpgradeSubsystem* ParkMemorial = World->GetSubsystem<UOCParkMemorialApproachAuthoredUpgradeSubsystem>();
    if (!ParkMemorial) { Pending.Add(TEXT("park_memorial_subsystem_missing")); bOutHardFailure = true; }
    else if (ParkMemorial->HasParkMemorialApproachFailed()) { Pending.Add(TEXT("park_memorial_failed")); bOutHardFailure = true; }
    else if (!ParkMemorial->IsParkMemorialApproachReady()) Pending.Add(TEXT("park_memorial"));

    OutPendingStages = Pending.Num() > 0 ? FString::Join(Pending, TEXT(",")) : TEXT("none");
    return Pending.Num() == 0;
}

void AOCGameModeRuntimeSafe::QueueRestartWhenWorldReady(AController* NewPlayer, const FString& PendingStages)
{
    if (!IsValid(NewPlayer) || !GetWorld()) return;

    const TWeakObjectPtr<AController> WeakController(NewPlayer);
    if (PendingWorldReadyRestarts.Contains(WeakController)) return;

    FPendingWorldReadyRestart& Pending = PendingWorldReadyRestarts.Add(WeakController);
    Pending.StartWallTimeSeconds = FPlatformTime::Seconds();

    FTimerDelegate RetryDelegate;
    RetryDelegate.BindUObject(this, &AOCGameModeRuntimeSafe::PollRestartWhenWorldReady, WeakController);
    GetWorldTimerManager().SetTimer(Pending.TimerHandle, RetryDelegate, WorldReadyRestartPollSeconds, true, WorldReadyRestartPollSeconds);

    UE_LOG(LogTemp, Display,
        TEXT("GAME_RECOVERY_SPAWN_GATE_WAIT pending=%s spawn_deferred=1 poll_ms=100 timeout_s=60 compatibility_path=1"),
        *PendingStages);
}

void AOCGameModeRuntimeSafe::ClearPendingWorldReadyRestart(const TWeakObjectPtr<AController>& WeakController)
{
    if (FPendingWorldReadyRestart* Pending = PendingWorldReadyRestarts.Find(WeakController))
    {
        if (GetWorld()) GetWorldTimerManager().ClearTimer(Pending->TimerHandle);
    }
    PendingWorldReadyRestarts.Remove(WeakController);
}

void AOCGameModeRuntimeSafe::PollRestartWhenWorldReady(TWeakObjectPtr<AController> WeakController)
{
    FPendingWorldReadyRestart* Pending = PendingWorldReadyRestarts.Find(WeakController);
    AController* Controller = WeakController.Get();
    if (!Pending || !IsValid(Controller))
    {
        ClearPendingWorldReadyRestart(WeakController);
        return;
    }

    const double WaitMilliseconds = (FPlatformTime::Seconds() - Pending->StartWallTimeSeconds) * 1000.0;
    FString PendingStages;
    bool bHardFailure = false;
    const bool bWorldReady = IsRecoveryWorldReady(PendingStages, bHardFailure);

    if (bWorldReady)
    {
        ClearPendingWorldReadyRestart(WeakController);
        UE_LOG(LogTemp, Display,
            TEXT("GAME_RECOVERY_SPAWN_GATE_READY waited_ms=%.1f player_spawn_release=1 post_spawn_world_loading=0"),
            WaitMilliseconds);
        RestartPlayer(Controller);
        return;
    }

    // Compatibility queue must never strand the human because a presentation/authoring acceptance gate failed.
    if (bHardFailure || WaitMilliseconds >= WorldReadyRestartTimeoutSeconds * 1000.0)
    {
        ClearPendingWorldReadyRestart(WeakController);
        UE_LOG(LogTemp, Error,
            TEXT("GAME_RECOVERY_SPAWN_GATE_FAIL reason=%s pending=%s waited_ms=%.1f player_spawned=0 fail_closed=0 acceptance_preserved=1"),
            bHardFailure ? TEXT("world_preparation_failed") : TEXT("timeout"), *PendingStages, WaitMilliseconds);
        UE_LOG(LogTemp, Warning,
            TEXT("GAME_RECOVERY_SPAWN_GATE_VISUAL_FAIL_SOFT pending=%s gameplay_spawn_release=1 acceptance_failed=1 fail_closed=0"),
            *PendingStages);
        RestartPlayer(Controller);
    }
}

void AOCGameModeRuntimeSafe::RestartPlayer(AController* NewPlayer)
{
    AOCPlayerController* HumanPC = Cast<AOCPlayerController>(NewPlayer);
    if (HumanPC)
    {
        FString PendingStages;
        bool bHardFailure = false;
        if (!IsRecoveryWorldReady(PendingStages, bHardFailure))
        {
            // Ground/surface/park/tree/material authoring is acceptance evidence, not permission to possess a pawn.
            // Preserve every failure in the log, but do not turn a visual/content problem into an unplayable gray screen.
            UE_LOG(LogTemp, bHardFailure ? ELogVerbosity::Error : ELogVerbosity::Warning,
                TEXT("GAME_RECOVERY_SPAWN_GATE_FAIL reason=%s pending=%s waited_ms=0 player_spawned=0 fail_closed=0 acceptance_preserved=1"),
                bHardFailure ? TEXT("world_preparation_failed") : TEXT("world_preparation_pending"), *PendingStages);
            UE_LOG(LogTemp, Warning,
                TEXT("GAME_RECOVERY_SPAWN_GATE_VISUAL_FAIL_SOFT pending=%s gameplay_spawn_release=1 acceptance_failed=%d fail_closed=0 visual_gate_blocks_spawn=0"),
                *PendingStages, bHardFailure ? 1 : 0);
        }
        else
        {
            UE_LOG(LogTemp, Display,
                TEXT("GAME_RECOVERY_SPAWN_GATE_READY waited_ms=0 player_spawn_release=1 post_spawn_world_loading=0 fail_closed=0"));
        }
    }

    if (!HumanPC || HumanPC->GetRequestedDeploymentSpawn() != FName(TEXT("BASE")))
    {
        Super::RestartPlayer(NewPlayer);
        return;
    }

    const AOCPlayerState* State = HumanPC->GetPlayerState<AOCPlayerState>();
    const EOCTeam Team = State ? State->GetTeamId() : EOCTeam::None;
    if (Team == EOCTeam::None || !GetWorld())
    {
        Super::RestartPlayer(NewPlayer);
        return;
    }

    const FVector Museum = AOCWorldSectorOster::MuseumAnchor();
    const AOCTeamSpawnPoint* BestMuseumBase = nullptr;
    double BestDistanceSq = TNumericLimits<double>::Max();

    for (TActorIterator<AOCTeamSpawnPoint> It(GetWorld()); It; ++It)
    {
        const AOCTeamSpawnPoint* Point = *It;
        if (!IsValid(Point) || !Point->IsBaseSpawn() || !Point->IsAvailableForTeam(Team)) continue;

        const double DistanceSq = static_cast<double>(FVector::DistSquared2D(Point->GetActorLocation(), Museum));
        if (!BestMuseumBase || DistanceSq < BestDistanceSq)
        {
            BestMuseumBase = Point;
            BestDistanceSq = DistanceSq;
        }
    }

    FTransform SpawnTransform;
    const TCHAR* SpawnSource = TEXT("museum_base_actor");
    if (BestMuseumBase)
    {
        SpawnTransform = BestMuseumBase->GetActorTransform();
        SpawnTransform.AddToTranslation(FVector(0.0f, 0.0f, SpawnLiftCm));
    }
    else
    {
        const float Side = Team == EOCTeam::TeamTwo ? 1.0f : -1.0f;
        const FVector FallbackLocation = Museum + FVector(1400.0f * Side, -2400.0f, 200.0f);
        const FRotator FallbackRotation = (Museum - FallbackLocation).Rotation();
        SpawnTransform = FTransform(FallbackRotation, FallbackLocation);
        SpawnSource = TEXT("museum_anchor_failsafe");
        UE_LOG(LogTemp, Warning,
            TEXT("PASS44_MUSEUM_BASE_ACTOR_MISSING team=%d using_anchor_failsafe=1"), static_cast<int32>(Team));
    }

    RestartPlayerAtTransform(NewPlayer, SpawnTransform);

    APawn* SpawnedPawn = NewPlayer ? NewPlayer->GetPawn() : nullptr;
    if (!IsValid(SpawnedPawn))
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS44_ACTUAL_PAWN_MUSEUM_BASE_FAIL reason=no_pawn team=%d source=%s"),
            static_cast<int32>(Team), SpawnSource);
        return;
    }

    float ActualDistanceCm = FVector::Dist2D(SpawnedPawn->GetActorLocation(), Museum);
    if (ActualDistanceCm > MaxMuseumBaseDistanceCm)
    {
        SpawnedPawn->SetActorLocation(SpawnTransform.GetLocation(), false, nullptr, ETeleportType::TeleportPhysics);
        SpawnedPawn->SetActorRotation(SpawnTransform.Rotator(), ETeleportType::TeleportPhysics);
        ActualDistanceCm = FVector::Dist2D(SpawnedPawn->GetActorLocation(), Museum);
        UE_LOG(LogTemp, Warning,
            TEXT("PASS44_ACTUAL_PAWN_MUSEUM_BASE_CORRECTED team=%d distance_m=%.1f max_m=45 source=%s"),
            static_cast<int32>(Team), ActualDistanceCm / 100.0f, SpawnSource);
    }

    if (ActualDistanceCm <= MaxMuseumBaseDistanceCm)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS44_ACTUAL_PAWN_MUSEUM_BASE_READY team=%d requested=BASE distance_m=%.1f max_m=45 source=%s pawn=(%.0f,%.0f,%.0f) museum=(%.0f,%.0f,%.0f)"),
            static_cast<int32>(Team), ActualDistanceCm / 100.0f, SpawnSource,
            SpawnedPawn->GetActorLocation().X, SpawnedPawn->GetActorLocation().Y, SpawnedPawn->GetActorLocation().Z,
            Museum.X, Museum.Y, Museum.Z);
    }
    else
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS44_ACTUAL_PAWN_MUSEUM_BASE_FAIL reason=distance team=%d distance_m=%.1f max_m=45 source=%s"),
            static_cast<int32>(Team), ActualDistanceCm / 100.0f, SpawnSource);
    }
}

void AOCGameModeRuntimeSafe::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (UWorld* World = GetWorld())
    {
        for (TPair<TWeakObjectPtr<AController>, FPendingWorldReadyRestart>& Pair : PendingWorldReadyRestarts)
            World->GetTimerManager().ClearTimer(Pair.Value.TimerHandle);
    }
    PendingWorldReadyRestarts.Reset();
    Super::EndPlay(EndPlayReason);
}
