#include "OCMuseumSpawnGuardSubsystem.h"

#include "OCGameMode.h"
#include "OCPlayerController.h"
#include "OCPlayerState.h"
#include "OCTeamSpawnPoint.h"
#include "OCWeaponBase.h"
#include "OCWorldSectorOster.h"

#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"

namespace
{
    const FName RuntimeBaseRackTag(TEXT("OC_RuntimeBaseWeaponRack"));
    constexpr int32 RequiredRackWeaponCount = 11;
    constexpr float PrimaryBaseRadiusCm = 6500.0f;
    constexpr float RackRadiusCm = 1800.0f;
    constexpr float MuseumNoSpawnRadiusCm = 3000.0f;
    constexpr float BaseDeploymentAcceptanceRadiusCm = 8000.0f;

    int32 CountRackWeaponsNear(UWorld* World, const FVector& BaseLocation)
    {
        if (!World) return 0;
        int32 Count = 0;
        for (TActorIterator<AOCWeaponBase> It(World); It; ++It)
        {
            const AOCWeaponBase* Weapon = *It;
            if (!Weapon || !Weapon->ActorHasTag(RuntimeBaseRackTag)) continue;
            if (FVector::DistSquared2D(Weapon->GetActorLocation(), BaseLocation) <= FMath::Square(RackRadiusCm))
            {
                ++Count;
            }
        }
        return Count;
    }

    AOCTeamSpawnPoint* FindPrimaryMuseumBase(UWorld* World, EOCTeam Team)
    {
        if (!World || Team == EOCTeam::None) return nullptr;
        const FVector Museum = AOCWorldSectorOster::MuseumAnchor();
        AOCTeamSpawnPoint* Best = nullptr;
        double BestSq = TNumericLimits<double>::Max();
        for (TActorIterator<AOCTeamSpawnPoint> It(World); It; ++It)
        {
            AOCTeamSpawnPoint* Point = *It;
            if (!Point || !Point->IsBaseSpawn() || Point->GetTeamId() != Team) continue;
            const double DistanceSq = FVector::DistSquared2D(Point->GetActorLocation(), Museum);
            if (DistanceSq < FMath::Square(MuseumNoSpawnRadiusCm)) continue;
            if (DistanceSq <= FMath::Square(PrimaryBaseRadiusCm) && DistanceSq < BestSq)
            {
                BestSq = DistanceSq;
                Best = Point;
            }
        }
        return Best;
    }
}

bool UOCMuseumSpawnGuardSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCMuseumSpawnGuardSubsystem::Tick(float DeltaTime)
{
    if (DeltaTime < 0.0f) return;

    UWorld* World = GetWorld();
    if (!World || World->GetNetMode() == NM_Client) return;

    ValidationAccumulator += DeltaTime;
    if (ValidationAccumulator < 0.25f) return;
    ValidationAccumulator = 0.0f;

    const AOCGameMode* GameMode = World->GetAuthGameMode<AOCGameMode>();
    if (!GameMode || GameMode->IsFrontendOnlySession()) return;

    if (!bFinished)
    {
        bFinished = EnsureAuthoritativeMuseumBases();
    }

    if (bFinished)
    {
        ValidateBaseDeployments();
    }
}

bool UOCMuseumSpawnGuardSubsystem::EnsureAuthoritativeMuseumBases()
{
    UWorld* World = GetWorld();
    if (!World || World->GetNetMode() == NM_Client) return false;

    const FVector Museum = AOCWorldSectorOster::MuseumAnchor();
    AOCTeamSpawnPoint* TeamOnePrimary = nullptr;
    AOCTeamSpawnPoint* TeamTwoPrimary = nullptr;
    double TeamOneBestSq = TNumericLimits<double>::Max();
    double TeamTwoBestSq = TNumericLimits<double>::Max();

    // Pass 30: AOCTeamSpawnPoint::BeginPlay already canonicalizes serialized BASE actors. Do not
    // repeatedly call ConfigureServer here based on distance: the new safe exterior bases are
    // intentionally 41+ metres from MuseumAnchor, and reconfiguring them would collapse secondary
    // bases onto the primary location. Only accept BASE actors outside the museum exclusion radius.
    for (TActorIterator<AOCTeamSpawnPoint> It(World); It; ++It)
    {
        AOCTeamSpawnPoint* Point = *It;
        if (!Point || !Point->IsBaseSpawn()) continue;

        const EOCTeam Team = Point->GetTeamId();
        if (Team != EOCTeam::TeamOne && Team != EOCTeam::TeamTwo) continue;

        const double DistanceSq = FVector::DistSquared2D(Point->GetActorLocation(), Museum);
        if (DistanceSq < FMath::Square(MuseumNoSpawnRadiusCm) ||
            DistanceSq > FMath::Square(PrimaryBaseRadiusCm))
        {
            continue;
        }

        if (Team == EOCTeam::TeamOne && DistanceSq < TeamOneBestSq)
        {
            TeamOneBestSq = DistanceSq;
            TeamOnePrimary = Point;
        }
        else if (Team == EOCTeam::TeamTwo && DistanceSq < TeamTwoBestSq)
        {
            TeamTwoBestSq = DistanceSq;
            TeamTwoPrimary = Point;
        }
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    auto EnsurePrimary = [&](const EOCTeam Team, AOCTeamSpawnPoint*& Primary)
    {
        if (Primary) return;

        AOCTeamSpawnPoint* Point = World->SpawnActor<AOCTeamSpawnPoint>(
            AOCTeamSpawnPoint::StaticClass(), Museum, FRotator::ZeroRotator, SpawnParams);
        if (!Point)
        {
            UE_LOG(LogTemp, Error,
                TEXT("PASS30_MUSEUM_EXTERIOR_BASE_CREATE_FAIL team=%s"), *OCTeamToString(Team));
            return;
        }

        Point->ConfigureServer(Team, true, NAME_None);
        const float DistanceM = FVector::Dist2D(Point->GetActorLocation(), Museum) / 100.0f;
        if (DistanceM < MuseumNoSpawnRadiusCm / 100.0f)
        {
            UE_LOG(LogTemp, Error,
                TEXT("PASS30_MUSEUM_EXTERIOR_BASE_CREATE_FAIL team=%s distance_m=%.1f location=%s"),
                *OCTeamToString(Team), DistanceM, *Point->GetActorLocation().ToCompactString());
            Point->Destroy();
            return;
        }

        Primary = Point;
        UE_LOG(LogTemp, Display,
            TEXT("PASS30_MUSEUM_EXTERIOR_BASE_CREATED team=%s location=%s distance_m=%.1f"),
            *OCTeamToString(Team), *Point->GetActorLocation().ToCompactString(), DistanceM);
    };

    EnsurePrimary(EOCTeam::TeamOne, TeamOnePrimary);
    EnsurePrimary(EOCTeam::TeamTwo, TeamTwoPrimary);
    if (!TeamOnePrimary || !TeamTwoPrimary) return false;

    int32 TeamOneRack = CountRackWeaponsNear(World, TeamOnePrimary->GetActorLocation());
    int32 TeamTwoRack = CountRackWeaponsNear(World, TeamTwoPrimary->GetActorLocation());
    if (TeamOneRack < RequiredRackWeaponCount)
    {
        TeamOnePrimary->ConfigureServer(EOCTeam::TeamOne, true, NAME_None);
        TeamOneRack = CountRackWeaponsNear(World, TeamOnePrimary->GetActorLocation());
    }
    if (TeamTwoRack < RequiredRackWeaponCount)
    {
        TeamTwoPrimary->ConfigureServer(EOCTeam::TeamTwo, true, NAME_None);
        TeamTwoRack = CountRackWeaponsNear(World, TeamTwoPrimary->GetActorLocation());
    }

    const float TeamOneDistanceM = FVector::Dist2D(TeamOnePrimary->GetActorLocation(), Museum) / 100.0f;
    const float TeamTwoDistanceM = FVector::Dist2D(TeamTwoPrimary->GetActorLocation(), Museum) / 100.0f;
    if (TeamOneRack >= RequiredRackWeaponCount && TeamTwoRack >= RequiredRackWeaponCount &&
        TeamOneDistanceM >= MuseumNoSpawnRadiusCm / 100.0f &&
        TeamTwoDistanceM >= MuseumNoSpawnRadiusCm / 100.0f)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS7_MUSEUM_BASES_READY museum=%s team1=1 team2=1"), *Museum.ToCompactString());
        UE_LOG(LogTemp, Display,
            TEXT("PASS30_MUSEUM_EXTERIOR_BASES_READY team1_rack=%d team2_rack=%d team1=%s team2=%s distance_m=%.1f/%.1f"),
            TeamOneRack, TeamTwoRack,
            *TeamOnePrimary->GetActorLocation().ToCompactString(),
            *TeamTwoPrimary->GetActorLocation().ToCompactString(),
            TeamOneDistanceM, TeamTwoDistanceM);
        return true;
    }

    UE_LOG(LogTemp, Warning,
        TEXT("PASS30_MUSEUM_EXTERIOR_BASES_NOT_READY team1_rack=%d team2_rack=%d distance_m=%.1f/%.1f"),
        TeamOneRack, TeamTwoRack, TeamOneDistanceM, TeamTwoDistanceM);
    return false;
}

void UOCMuseumSpawnGuardSubsystem::ValidateBaseDeployments()
{
    UWorld* World = GetWorld();
    if (!World || World->GetNetMode() == NM_Client) return;

    const FVector Museum = AOCWorldSectorOster::MuseumAnchor();

    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        AOCPlayerController* PC = Cast<AOCPlayerController>(It->Get());
        if (!PC || PC->IsActorBeingDestroyed()) continue;

        APawn* Pawn = PC->GetPawn();
        if (!Pawn) continue;

        if (LastValidatedPawnByController.FindRef(PC).Get() == Pawn) continue;
        LastValidatedPawnByController.Add(PC, Pawn);

        if (!PC->GetRequestedDeploymentSpawn().ToString().Equals(TEXT("BASE"), ESearchCase::IgnoreCase))
        {
            continue;
        }

        const AOCPlayerState* State = PC->GetPlayerState<AOCPlayerState>();
        const EOCTeam Team = State ? State->GetTeamId() : EOCTeam::None;
        if (Team == EOCTeam::None) continue;

        const double DistanceSq = FVector::DistSquared2D(Pawn->GetActorLocation(), Museum);
        const bool bOutsideMuseum = DistanceSq >= FMath::Square(MuseumNoSpawnRadiusCm);
        const bool bNearMuseum = DistanceSq <= FMath::Square(BaseDeploymentAcceptanceRadiusCm);
        if (bOutsideMuseum && bNearMuseum)
        {
            UE_LOG(LogTemp, Display,
                TEXT("PASS30_BASE_DEPLOYMENT_OUTSIDE_MUSEUM team=%s pawn=%s location=%s distance_m=%.1f"),
                *OCTeamToString(Team), *Pawn->GetName(), *Pawn->GetActorLocation().ToCompactString(),
                FVector::Dist2D(Pawn->GetActorLocation(), Museum) / 100.0f);
            continue;
        }

        AOCTeamSpawnPoint* Base = FindPrimaryMuseumBase(World, Team);
        if (!Base)
        {
            UE_LOG(LogTemp, Error,
                TEXT("PASS30_BASE_DEPLOYMENT_RECOVERY_FAIL team=%s pawn=%s no_exterior_base=1 location=%s"),
                *OCTeamToString(Team), *Pawn->GetName(), *Pawn->GetActorLocation().ToCompactString());
            continue;
        }

        const FVector OldLocation = Pawn->GetActorLocation();
        FTransform Corrected = Base->GetActorTransform();
        Corrected.AddToTranslation(FVector(0.0f, 0.0f, 80.0f));
        Pawn->SetActorLocationAndRotation(
            Corrected.GetLocation(), Corrected.Rotator(), false, nullptr, ETeleportType::TeleportPhysics);

        UE_LOG(LogTemp, Warning,
            TEXT("PASS30_BASE_DEPLOYMENT_RECOVERED_OUTSIDE_MUSEUM team=%s pawn=%s old=%s new=%s museum=%s distance_m=%.1f"),
            *OCTeamToString(Team), *Pawn->GetName(), *OldLocation.ToCompactString(),
            *Pawn->GetActorLocation().ToCompactString(), *Museum.ToCompactString(),
            FVector::Dist2D(Pawn->GetActorLocation(), Museum) / 100.0f);
    }
}

TStatId UOCMuseumSpawnGuardSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCMuseumSpawnGuardSubsystem, STATGROUP_Tickables);
}
