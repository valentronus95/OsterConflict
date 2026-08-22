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
    constexpr float PrimaryBaseRadiusCm = 2200.0f;
    constexpr float RackRadiusCm = 1800.0f;
    constexpr float BaseDeploymentAcceptanceRadiusCm = 3500.0f;

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

    // Pass 15: do not wait on AOCWorldSectorOster actor existence. The failed laptop playtest proved
    // that this guard can otherwise remain inert even though gameplay/deployment is already visible.
    // GameMode is the authoritative distinction between the startup frontend shell and a real match.
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

    // Repair stale map-edge BASE actors first, then explicitly identify a PRIMARY Museum BASE.
    // A secondary base alone is no longer enough to claim the deployment route is ready.
    for (TActorIterator<AOCTeamSpawnPoint> It(World); It; ++It)
    {
        AOCTeamSpawnPoint* Point = *It;
        if (!Point || !Point->IsBaseSpawn()) continue;

        const EOCTeam Team = Point->GetTeamId();
        if (Team != EOCTeam::TeamOne && Team != EOCTeam::TeamTwo) continue;

        if (FVector::DistSquared2D(Point->GetActorLocation(), Museum) > FMath::Square(3500.0f))
        {
            UE_LOG(LogTemp, Warning,
                TEXT("Pass15 spawn guard repairing stale BASE: team=%s old=%s museum=%s"),
                *OCTeamToString(Team), *Point->GetActorLocation().ToCompactString(), *Museum.ToCompactString());
            Point->ConfigureServer(Team, true, NAME_None);
        }

        const double DistanceSq = FVector::DistSquared2D(Point->GetActorLocation(), Museum);
        if (DistanceSq > FMath::Square(PrimaryBaseRadiusCm)) continue;

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
                TEXT("Pass15 spawn guard FAILED to create primary Museum BASE for team=%s"), *OCTeamToString(Team));
            return;
        }

        Point->ConfigureServer(Team, true, NAME_None);
        Primary = Point;
        UE_LOG(LogTemp, Warning,
            TEXT("Pass15 spawn guard created missing primary Museum BASE: team=%s location=%s"),
            *OCTeamToString(Team), *Point->GetActorLocation().ToCompactString());
    };

    EnsurePrimary(EOCTeam::TeamOne, TeamOnePrimary);
    EnsurePrimary(EOCTeam::TeamTwo, TeamTwoPrimary);
    if (!TeamOnePrimary || !TeamTwoPrimary) return false;

    // A spawn-point tag is not rack evidence. If fewer than 11 physical tagged weapons exist near
    // the primary BASE, reconfigure that BASE so OCTeamSpawnPoint rebuilds the rack from scratch.
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

    if (TeamOneRack >= RequiredRackWeaponCount && TeamTwoRack >= RequiredRackWeaponCount)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS7_MUSEUM_BASES_READY museum=%s team1=1 team2=1"), *Museum.ToCompactString());
        UE_LOG(LogTemp, Display,
            TEXT("PASS15_MUSEUM_BASES_WEAPONS_READY team1_rack=%d team2_rack=%d team1=%s team2=%s"),
            TeamOneRack, TeamTwoRack,
            *TeamOnePrimary->GetActorLocation().ToCompactString(),
            *TeamTwoPrimary->GetActorLocation().ToCompactString());
        return true;
    }

    UE_LOG(LogTemp, Warning,
        TEXT("PASS15_MUSEUM_BASES_WEAPONS_NOT_READY team1_rack=%d team2_rack=%d"), TeamOneRack, TeamTwoRack);
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
        if (DistanceSq <= FMath::Square(BaseDeploymentAcceptanceRadiusCm))
        {
            UE_LOG(LogTemp, Display,
                TEXT("PASS15_BASE_DEPLOYMENT_NEAR_MUSEUM team=%s pawn=%s location=%s"),
                *OCTeamToString(Team), *Pawn->GetName(), *Pawn->GetActorLocation().ToCompactString());
            continue;
        }

        AOCTeamSpawnPoint* Base = FindPrimaryMuseumBase(World, Team);
        if (!Base)
        {
            UE_LOG(LogTemp, Error,
                TEXT("PASS15_BASE_DEPLOYMENT_RECOVERY_FAIL team=%s pawn=%s no_primary_base=1 location=%s"),
                *OCTeamToString(Team), *Pawn->GetName(), *Pawn->GetActorLocation().ToCompactString());
            continue;
        }

        const FVector OldLocation = Pawn->GetActorLocation();
        FTransform Corrected = Base->GetActorTransform();
        Corrected.AddToTranslation(FVector(0.0f, 0.0f, 80.0f));
        Pawn->SetActorLocationAndRotation(
            Corrected.GetLocation(), Corrected.Rotator(), false, nullptr, ETeleportType::TeleportPhysics);

        UE_LOG(LogTemp, Warning,
            TEXT("PASS15_BASE_DEPLOYMENT_RECOVERED team=%s pawn=%s old=%s new=%s museum=%s"),
            *OCTeamToString(Team), *Pawn->GetName(), *OldLocation.ToCompactString(),
            *Pawn->GetActorLocation().ToCompactString(), *Museum.ToCompactString());
    }
}

TStatId UOCMuseumSpawnGuardSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCMuseumSpawnGuardSubsystem, STATGROUP_Tickables);
}
