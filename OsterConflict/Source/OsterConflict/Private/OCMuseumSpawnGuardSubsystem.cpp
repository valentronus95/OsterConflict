#include "OCMuseumSpawnGuardSubsystem.h"

#include "OCTeamSpawnPoint.h"
#include "OCWorldSectorOster.h"

#include "EngineUtils.h"
#include "Engine/World.h"

bool UOCMuseumSpawnGuardSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCMuseumSpawnGuardSubsystem::Tick(float DeltaTime)
{
    if (bFinished || DeltaTime < 0.0f) return;

    UWorld* World = GetWorld();
    if (!World || World->GetNetMode() == NM_Client) return;

    // The frontend-only shell deliberately has no Oster gameplay sector. Waiting for this actor keeps the guard
    // out of the main menu while still making it active as soon as the real gameplay world has been initialized.
    bool bGameplaySectorPresent = false;
    for (TActorIterator<AOCWorldSectorOster> It(World); It; ++It)
    {
        if (IsValid(*It))
        {
            bGameplaySectorPresent = true;
            break;
        }
    }
    if (!bGameplaySectorPresent) return;

    bFinished = EnsureAuthoritativeMuseumBases();
}

bool UOCMuseumSpawnGuardSubsystem::EnsureAuthoritativeMuseumBases()
{
    UWorld* World = GetWorld();
    if (!World || World->GetNetMode() == NM_Client) return false;

    const FVector Museum = AOCWorldSectorOster::MuseumAnchor();
    bool bHasTeamOneBase = false;
    bool bHasTeamTwoBase = false;

    // Repair any stale map-edge BASE actors before considering the set complete. ConfigureServer() owns the
    // canonical Museum-relative transform, ground snap and primary weapon-rack creation contract.
    for (TActorIterator<AOCTeamSpawnPoint> It(World); It; ++It)
    {
        AOCTeamSpawnPoint* Point = *It;
        if (!Point || !Point->IsBaseSpawn()) continue;

        const EOCTeam Team = Point->GetTeamId();
        if (Team != EOCTeam::TeamOne && Team != EOCTeam::TeamTwo) continue;

        if (FVector::DistSquared2D(Point->GetActorLocation(), Museum) > FMath::Square(3500.0f))
        {
            UE_LOG(LogTemp, Warning,
                TEXT("Pass7 spawn guard repairing stale BASE before deployment: team=%s old=%s museum=%s"),
                *OCTeamToString(Team), *Point->GetActorLocation().ToCompactString(), *Museum.ToCompactString());
            Point->ConfigureServer(Team, true, NAME_None);
        }

        if (Team == EOCTeam::TeamOne) bHasTeamOneBase = true;
        if (Team == EOCTeam::TeamTwo) bHasTeamTwoBase = true;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    auto EnsureTeamBase = [&](const EOCTeam Team, bool& bHasBase)
    {
        if (bHasBase) return;

        AOCTeamSpawnPoint* Point = World->SpawnActor<AOCTeamSpawnPoint>(
            AOCTeamSpawnPoint::StaticClass(), Museum, FRotator::ZeroRotator, SpawnParams);
        if (!Point)
        {
            UE_LOG(LogTemp, Error,
                TEXT("Pass7 spawn guard FAILED to create Museum BASE for team=%s"), *OCTeamToString(Team));
            return;
        }

        // Spawning at MuseumAnchor intentionally classifies this as the primary BASE. ConfigureServer then applies
        // the canonical team offset, walkable-surface snap and 11-weapon rack instead of leaving it at the anchor.
        Point->ConfigureServer(Team, true, NAME_None);
        bHasBase = true;
        UE_LOG(LogTemp, Warning,
            TEXT("Pass7 spawn guard created missing authoritative Museum BASE: team=%s location=%s"),
            *OCTeamToString(Team), *Point->GetActorLocation().ToCompactString());
    };

    EnsureTeamBase(EOCTeam::TeamOne, bHasTeamOneBase);
    EnsureTeamBase(EOCTeam::TeamTwo, bHasTeamTwoBase);

    if (bHasTeamOneBase && bHasTeamTwoBase)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS7_MUSEUM_BASES_READY museum=%s team1=1 team2=1"), *Museum.ToCompactString());
        return true;
    }
    return false;
}

TStatId UOCMuseumSpawnGuardSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCMuseumSpawnGuardSubsystem, STATGROUP_Tickables);
}
