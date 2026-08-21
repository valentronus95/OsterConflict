#include "OCPlayerController.h"

#include "OCDeploymentLoadingSubsystem.h"
#include "OCPlayerState.h"
#include "Engine/World.h"

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

    if (UWorld* World = GetWorld())
    {
        if (UOCDeploymentLoadingSubsystem* Loading = World->GetSubsystem<UOCDeploymentLoadingSubsystem>())
        {
            Loading->BeginDeployment(this);
            return;
        }
    }

    // Fallback if the transition subsystem is unavailable for an unusual world type.
    UIReadyDeploy();
}
