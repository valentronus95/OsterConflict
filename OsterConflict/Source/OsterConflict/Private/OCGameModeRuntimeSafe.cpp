#include "OCGameModeRuntimeSafe.h"

#include "OCPlayerController.h"
#include "OCPlayerState.h"
#include "OCTeamSpawnPoint.h"
#include "OCWorldSectorOster.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

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

    const bool bBotsExplicit = HasExplicitOption(Options, TEXT("Bots"));
    const bool bPopulationExplicit = HasExplicitOption(Options, TEXT("Population"));
    const bool bBotFillExplicit = HasExplicitOption(Options, TEXT("BotFill"));

    if (!bBotsExplicit && !bPopulationExplicit && !bBotFillExplicit)
    {
        // Pass 44: a normal local visual/playtest launch must measure the actual map/content,
        // not a hidden 16-player AI workload that starts roughly one second after deployment.
        TargetPopulation = 0;
        bAutoFillBots = false;
        UE_LOG(LogTemp, Display,
            TEXT("PASS44_LOCAL_BOT_AUTOFILL_DISABLED_READY implicit_population=0 explicit_bot_options=0 background_ai_load=0"));
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

    if (IsFrontendOnlySession() || !HasAuthority() || !GetWorld())
    {
        return;
    }

    // PASS45 P0: UWorldSubsystem::OnWorldBeginPlay runs before AOCGameMode::BeginPlay. Block0 therefore needs
    // the lightweight sector actor before the legacy SpawnOsterCenterSector() path runs. The sector constructor
    // no longer synchronously resolves KiteDemo tree packages, so this ordering repair does not move the rejected
    // HillTree_02 / ScotsPine startup dependency back into pre-tick initialization.
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
        Sector = GetWorld()->SpawnActor<AOCWorldSectorOster>(
            AOCWorldSectorOster::StaticClass(), FTransform::Identity, SpawnParams);
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

    if (IsFrontendOnlySession() || !HasAuthority() || !GetWorld())
    {
        return;
    }

    // The base GameMode still owns all normal gameplay population and currently calls SpawnOsterCenterSector(),
    // which creates a second sector. Keep its non-sector work intact, but retire only that duplicate before the
    // first gameplay tick so every runtime subsystem sees one canonical Oster sector after BeginPlay completes.
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
            CanonicalSector ? 1 : 0,
            RemainingSectors.Num(),
            DuplicatesRetired);
        return;
    }

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_OSTER_SECTOR_SINGLE_OWNER_READY canonical_pretick_owner=1 sector_count=1 duplicates_retired=%d before_first_tick=1 heavy_tree_startup_loads=0 runtime_acceptance=0"),
        DuplicatesRetired);
}

void AOCGameModeRuntimeSafe::RestartPlayer(AController* NewPlayer)
{
    AOCPlayerController* HumanPC = Cast<AOCPlayerController>(NewPlayer);
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
        // Fail-safe only. Normal runtime should always have the canonical team BASE actor.
        // Keep this near the Museum rather than falling back to the old map center/edge logic.
        const float Side = Team == EOCTeam::TeamTwo ? 1.0f : -1.0f;
        const FVector FallbackLocation = Museum + FVector(1400.0f * Side, -2400.0f, 200.0f);
        const FRotator FallbackRotation = (Museum - FallbackLocation).Rotation();
        SpawnTransform = FTransform(FallbackRotation, FallbackLocation);
        SpawnSource = TEXT("museum_anchor_failsafe");
        UE_LOG(LogTemp, Warning,
            TEXT("PASS44_MUSEUM_BASE_ACTOR_MISSING team=%d using_anchor_failsafe=1"),
            static_cast<int32>(Team));
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
        // Collision adjustment or a stale spawn owner must never silently move the real player back
        // to the giant legacy map. Correct the live pawn and leave explicit runtime evidence.
        SpawnedPawn->SetActorLocation(
            SpawnTransform.GetLocation(),
            false,
            nullptr,
            ETeleportType::TeleportPhysics);
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
            static_cast<int32>(Team),
            ActualDistanceCm / 100.0f,
            SpawnSource,
            SpawnedPawn->GetActorLocation().X,
            SpawnedPawn->GetActorLocation().Y,
            SpawnedPawn->GetActorLocation().Z,
            Museum.X,
            Museum.Y,
            Museum.Z);
    }
    else
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS44_ACTUAL_PAWN_MUSEUM_BASE_FAIL reason=distance team=%d distance_m=%.1f max_m=45 source=%s"),
            static_cast<int32>(Team), ActualDistanceCm / 100.0f, SpawnSource);
    }
}
