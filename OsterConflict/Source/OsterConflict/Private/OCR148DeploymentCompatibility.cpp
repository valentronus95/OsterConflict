#include "OCPlayerController.h"

#include "OCPlayerState.h"

namespace
{
    int32 RoleIndex(const EOCPlayerRole Role)
    {
        switch (Role)
        {
        case EOCPlayerRole::Medic: return 1;
        case EOCPlayerRole::Engineer: return 2;
        case EOCPlayerRole::Support: return 3;
        default: return 0;
        }
    }
}

void AOCPlayerController::UIRequestSquad(const int32 SquadId)
{
    if (!bDeploymentPanelVisible || SquadId < 0 || SquadId >= 8) return;
    if (HasAuthority()) ServerRequestSquad_Implementation(SquadId);
    else ServerRequestSquad(SquadId);
}

void AOCPlayerController::UIRequestRole(const EOCPlayerRole RequestedRole)
{
    if (!bDeploymentPanelVisible) return;

    const AOCPlayerState* State = GetPlayerState<AOCPlayerState>();
    const EOCPlayerRole CurrentRole = State ? State->GetPlayerRole() : EOCPlayerRole::Rifleman;
    const int32 Steps = (RoleIndex(RequestedRole) - RoleIndex(CurrentRole) + 4) % 4;

    for (int32 Step = 0; Step < Steps; ++Step)
    {
        if (HasAuthority()) ServerCycleRole_Implementation();
        else ServerCycleRole();
    }
}

void AOCPlayerController::UICommitDeployment()
{
    if (!bDeploymentPanelVisible || bFrontendMenuVisible || bSettingsVisible) return;

    // Current main already owns authoritative RestartPlayer through UIReadyDeploy/ServerSetLobbyReady.
    // The recovered staged UI calls this compatibility entry point instead of restoring the obsolete
    // compact-map deployment bridge that used to couple UI state to old R13 geography code.
    UIReadyDeploy();
}
