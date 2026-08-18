#include "OCGameMode.h"

#include "OCGameState.h"
#include "OCPlayerState.h"

#include "GameFramework/PlayerState.h"

bool AOCGameMode::RequestRoleChange(AOCPlayerState* State, EOCPlayerRole RequestedRole)
{
    if (!HasAuthority() || !State || State->GetTeamId() == EOCTeam::None || State->GetSquadId() < 0)
    {
        return false;
    }
    if (State->IsLobbyReady())
    {
        return false;
    }
    if (State->GetPlayerRole() == RequestedRole)
    {
        return true;
    }

    // Rifleman is the flexible role. Specialist roles remain one-per-squad and are enforced here on the server,
    // not merely greyed out by the local deployment UI.
    if (RequestedRole != EOCPlayerRole::Rifleman)
    {
        const AOCGameState* GameState = GetGameState<AOCGameState>();
        if (!GameState) return false;

        for (APlayerState* RawState : GameState->PlayerArray)
        {
            const AOCPlayerState* Other = Cast<AOCPlayerState>(RawState);
            if (!Other || Other == State) continue;
            if (Other->GetTeamId() == State->GetTeamId() &&
                Other->GetSquadId() == State->GetSquadId() &&
                Other->GetPlayerRole() == RequestedRole)
            {
                return false;
            }
        }
    }

    State->SetRoleServer(RequestedRole);
    State->SetLobbyReadyServer(false);
    return true;
}
