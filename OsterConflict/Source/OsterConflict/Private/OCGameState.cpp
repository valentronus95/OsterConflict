#include "OCGameState.h"

#include "Net/UnrealNetwork.h"

AOCGameState::AOCGameState()
{
    bReplicates = true;
}

void AOCGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AOCGameState, TeamOneTickets);
    DOREPLIFETIME(AOCGameState, TeamTwoTickets);
    DOREPLIFETIME(AOCGameState, StartingTickets);
    DOREPLIFETIME(AOCGameState, MatchPhase);
    DOREPLIFETIME(AOCGameState, WinningTeam);
    DOREPLIFETIME(AOCGameState, GameplayMode);
    DOREPLIFETIME(AOCGameState, HumanPlayerCount);
    DOREPLIFETIME(AOCGameState, BotPlayerCount);
    DOREPLIFETIME(AOCGameState, MaxPlayerSlots);
    DOREPLIFETIME(AOCGameState, TargetPopulation);
}

void AOCGameState::SetGameplayModeServer(EOCGameplayMode NewMode)
{
    if (!HasAuthority()) return;
    GameplayMode = NewMode;
    ForceNetUpdate();
}

void AOCGameState::ConfigurePopulationServer(int32 InMaxSlots, int32 InTargetPopulation)
{
    if (!HasAuthority()) return;
    MaxPlayerSlots = FMath::Clamp(InMaxSlots, 2, 64);
    TargetPopulation = FMath::Clamp(InTargetPopulation, 0, MaxPlayerSlots);
    ForceNetUpdate();
}

void AOCGameState::SetPopulationCountsServer(int32 Humans, int32 Bots)
{
    if (!HasAuthority()) return;
    HumanPlayerCount = FMath::Max(0, Humans);
    BotPlayerCount = FMath::Max(0, Bots);
    ForceNetUpdate();
}

int32 AOCGameState::GetTickets(EOCTeam Team) const
{
    if (Team == EOCTeam::TeamOne) return TeamOneTickets;
    if (Team == EOCTeam::TeamTwo) return TeamTwoTickets;
    return 0;
}

void AOCGameState::InitializeRoundServer(int32 InStartingTickets)
{
    if (!HasAuthority()) return;
    StartingTickets = FMath::Max(1, InStartingTickets);
    TeamOneTickets = StartingTickets;
    TeamTwoTickets = StartingTickets;
    WinningTeam = EOCTeam::None;
    MatchPhase = EOCMatchPhase::InProgress;
    ForceNetUpdate();
}

int32 AOCGameState::RemoveTicketsServer(EOCTeam Team, int32 Amount)
{
    if (!HasAuthority() || MatchPhase != EOCMatchPhase::InProgress || Amount <= 0) return GetTickets(Team);
    if (Team == EOCTeam::TeamOne)
    {
        TeamOneTickets = FMath::Max(0, TeamOneTickets - Amount);
        ForceNetUpdate();
        return TeamOneTickets;
    }
    if (Team == EOCTeam::TeamTwo)
    {
        TeamTwoTickets = FMath::Max(0, TeamTwoTickets - Amount);
        ForceNetUpdate();
        return TeamTwoTickets;
    }
    return 0;
}

void AOCGameState::FinishRoundServer(EOCTeam Winner)
{
    if (!HasAuthority() || MatchPhase == EOCMatchPhase::Ended) return;
    MatchPhase = EOCMatchPhase::Ended;
    WinningTeam = Winner;
    ForceNetUpdate();
}
