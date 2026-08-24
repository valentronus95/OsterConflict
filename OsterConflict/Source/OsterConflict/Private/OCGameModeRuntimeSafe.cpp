#include "OCGameModeRuntimeSafe.h"

#include "OCPlayerController.h"
#include "OCPlayerState.h"
#include "OCTeamSpawnPoint.h"
#include "OCWorldSectorOster.h"

#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

namespace
{
    constexpr float MaxMuseumBaseDistanceCm = 4500.0f;
    constexpr float SpawnLiftCm = 80.0f;

    bool HasExplicitOption(const FString& Options, const TCHAR* Key)
    {
        FString Value = UGameplayStatics::ParseOption(Options, Key);
        Value.TrimStartAndEndInline();
        return !Value.IsEmpty();
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
