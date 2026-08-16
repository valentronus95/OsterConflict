#include "OCPlayerState.h"

#include "Net/UnrealNetwork.h"

AOCPlayerState::AOCPlayerState()
{
    SetScore(0.0f);
    SetShouldUpdateReplicatedPing(true);
}

void AOCPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AOCPlayerState, Kills);
    DOREPLIFETIME(AOCPlayerState, Deaths);
    DOREPLIFETIME(AOCPlayerState, Revives);
    DOREPLIFETIME(AOCPlayerState, TeamId);
    DOREPLIFETIME(AOCPlayerState, PlayerRole);
    DOREPLIFETIME(AOCPlayerState, bBotPlayer);
    DOREPLIFETIME(AOCPlayerState, SquadId);
    DOREPLIFETIME(AOCPlayerState, bSquadLeader);
    DOREPLIFETIME(AOCPlayerState, bLobbyReady);
    DOREPLIFETIME(AOCPlayerState, FactionArchetype);
    DOREPLIFETIME(AOCPlayerState, AppearanceSeed);
}

int32 AOCPlayerState::GetPingMs() const
{
    return FMath::Max(0, FMath::RoundToInt(GetPingInMilliseconds()));
}

void AOCPlayerState::RegisterKill(int32 ScoreAward)
{
    if (!HasAuthority()) return;
    ++Kills;
    SetScore(GetScore() + static_cast<float>(FMath::Max(0, ScoreAward)));
}

void AOCPlayerState::RegisterDeath()
{
    if (!HasAuthority()) return;
    ++Deaths;
}

void AOCPlayerState::RegisterRevive(int32 ScoreAward)
{
    if (!HasAuthority()) return;
    ++Revives;
    SetScore(GetScore() + static_cast<float>(FMath::Max(0, ScoreAward)));
}

void AOCPlayerState::ResetRoundStatsServer()
{
    if (!HasAuthority()) return;
    Kills = 0;
    Deaths = 0;
    Revives = 0;
    SetScore(0.0f);
    ForceNetUpdate();
}

void AOCPlayerState::SetTeamServer(EOCTeam NewTeam)
{
    if (!HasAuthority()) return;
    TeamId = NewTeam;
    ForceNetUpdate();
}

void AOCPlayerState::SetRoleServer(EOCPlayerRole NewRole)
{
    if (!HasAuthority()) return;
    PlayerRole = NewRole;
    ForceNetUpdate();
}

void AOCPlayerState::SetBotPlayerServer(bool bNewBot)
{
    if (!HasAuthority()) return;
    bBotPlayer = bNewBot;
    if (bBotPlayer) bLobbyReady = true;
    ForceNetUpdate();
}

void AOCPlayerState::SetSquadServer(int32 NewSquadId, bool bLeader)
{
    if (!HasAuthority()) return;
    SquadId = NewSquadId;
    bSquadLeader = bLeader;
    ForceNetUpdate();
}

void AOCPlayerState::SetSquadLeaderServer(bool bLeader)
{
    if (!HasAuthority()) return;
    bSquadLeader = bLeader;
    ForceNetUpdate();
}

void AOCPlayerState::SetLobbyReadyServer(bool bReady)
{
    if (!HasAuthority()) return;
    bLobbyReady = bReady;
    ForceNetUpdate();
}


void AOCPlayerState::SetFactionServer(EOCFactionArchetype NewFaction, int32 NewAppearanceSeed)
{
    if (!HasAuthority()) return;
    FactionArchetype = NewFaction;
    AppearanceSeed = FMath::Max(1, NewAppearanceSeed);
    ForceNetUpdate();
}
