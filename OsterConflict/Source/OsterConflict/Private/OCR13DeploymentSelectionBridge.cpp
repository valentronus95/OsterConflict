#include "OCPlayerController.h"

#include "OCGameMode.h"
#include "OCGameState.h"
#include "OCPlayerState.h"

#include "Engine/World.h"
#include "GameFramework/PlayerState.h"

void AOCPlayerController::UIRequestSquad(const int32 SquadId)
{
    if (!bDeploymentPanelVisible || SquadId < 0 || SquadId >= 8) return;
    if (HasAuthority()) ServerRequestSquad_Implementation(SquadId);
    else ServerRequestSquad(SquadId);
}

void AOCPlayerController::UIRequestRole(const EOCPlayerRole Role)
{
    if (!bDeploymentPanelVisible) return;
    if (HasAuthority()) ServerRequestRole_Implementation(Role);
    else ServerRequestRole(Role);
}

void AOCPlayerController::UICommitDeployment()
{
    if (!bDeploymentPanelVisible || bFrontendMenuVisible || bSettingsVisible) return;

    // Unlike the legacy UIReadyDeploy path, keep the deployment UI visible while the authoritative server
    // creates and ground-validates the pawn. ClientCompleteDeployment is the only thing allowed to close it.
    if (HasAuthority()) ServerSetLobbyReady_Implementation(true);
    else ServerSetLobbyReady(true);
}

void AOCPlayerController::ServerRequestRole_Implementation(const EOCPlayerRole RequestedRole)
{
    AOCGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AOCGameMode>() : nullptr;
    AOCPlayerState* State = GetPlayerState<AOCPlayerState>();
    if (GameMode && State)
    {
        GameMode->RequestRoleChange(State, RequestedRole);
    }
}

void AOCPlayerController::ClientCompleteDeployment_Implementation(const bool bSuccess)
{
    if (!IsLocalController()) return;

    bDeploymentPanelVisible = !bSuccess;
    if (!bSuccess)
    {
        bFrontendMenuVisible = false;
    }
    ApplyUIInputMode();
}

bool AOCGameMode::RequestRoleChange(AOCPlayerState* State, const EOCPlayerRole RequestedRole)
{
    if (!HasAuthority() || !State || State->IsLobbyReady()) return false;
    if (State->GetTeamId() == EOCTeam::None || State->GetSquadId() < 0) return false;
    if (State->GetPlayerRole() == RequestedRole) return true;

    // Riflemen are the flexible/default slot. Specialist roles are unique inside a four-person squad.
    if (RequestedRole != EOCPlayerRole::Rifleman)
    {
        if (const AOCGameState* GameState = GetGameState<AOCGameState>())
        {
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
    }

    State->SetRoleServer(RequestedRole);
    State->SetLobbyReadyServer(false);
    return true;
}
