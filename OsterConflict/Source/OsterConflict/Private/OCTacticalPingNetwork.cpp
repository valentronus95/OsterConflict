#include "OCPlayerController.h"

#include "OCPlayerState.h"

#include "Engine/World.h"
#include "GameFramework/Pawn.h"

namespace
{
    constexpr double TacticalPingCooldownSeconds = 0.25;
    constexpr float TacticalPingMaxDistanceCm = 500000.0f;
    constexpr int32 MaxRecentTacticalPings = 8;
}

void AOCPlayerController::SubmitTacticalPing(const FVector& WorldLocation)
{
    if (HasAuthority()) ServerSubmitTacticalPing_Implementation(WorldLocation);
    else ServerSubmitTacticalPing(WorldLocation);
}

void AOCPlayerController::ServerSubmitTacticalPing_Implementation(FVector Location)
{
    UWorld* World = GetWorld();
    AOCPlayerState* State = GetPlayerState<AOCPlayerState>();
    APawn* Pawn = GetPawn();
    if (!World || !State || !Pawn) return;
    if (State->GetTeamId() == EOCTeam::None || State->GetSquadId() < 0) return;
    if (Location.ContainsNaN()) return;

    const double Now = World->GetTimeSeconds();
    if (Now - LastTacticalPingServerTime < TacticalPingCooldownSeconds) return;
    if (FVector::DistSquared2D(Pawn->GetActorLocation(), Location) > FMath::Square(TacticalPingMaxDistanceCm)) return;
    LastTacticalPingServerTime = Now;

    FOCTacticalPing Ping;
    Ping.WorldLocation = Location;
    Ping.IssuerName = State->GetPlayerName();
    Ping.Team = State->GetTeamId();
    Ping.SquadId = State->GetSquadId();
    Ping.ServerTime = static_cast<float>(Now);

    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        AOCPlayerController* Recipient = Cast<AOCPlayerController>(It->Get());
        if (!Recipient) continue;
        const AOCPlayerState* RecipientState = Recipient->GetPlayerState<AOCPlayerState>();
        if (!RecipientState) continue;
        if (RecipientState->GetTeamId() == Ping.Team && RecipientState->GetSquadId() == Ping.SquadId)
            Recipient->ClientReceiveTacticalPing(Ping);
    }
}

void AOCPlayerController::ClientReceiveTacticalPing_Implementation(const FOCTacticalPing& Ping)
{
    const AOCPlayerState* LocalState = GetPlayerState<AOCPlayerState>();
    if (!LocalState) return;
    if (Ping.Team != LocalState->GetTeamId() || Ping.SquadId != LocalState->GetSquadId()) return;

    RecentTacticalPings.Add(Ping);
    while (RecentTacticalPings.Num() > MaxRecentTacticalPings) RecentTacticalPings.RemoveAt(0);
    ++TacticalPingRevision;
}
