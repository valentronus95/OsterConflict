#include "OCPlayerController.h"

#include "OCGameMode.h"
#include "OCGameState.h"
#include "OCPlayerState.h"
#include "OCR13CompactOsterSubsystem.h"

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

    // Keep the deployment UI visible while the authoritative server checks that compact Oster has already
    // relocated the old source PlayerStarts and then creates/ground-validates the pawn.
    if (HasAuthority()) ServerCommitDeployment_Implementation();
    else ServerCommitDeployment();
}

void AOCPlayerController::ServerCommitDeployment_Implementation()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // The source world still authors legacy bases far outside the R13 compact crop. Never let a human become ready
    // until OCR13CompactOsterSubsystem has cropped the world, moved objectives and relocated TeamSpawn actors.
    if (World->GetMapName().Contains(TEXT("OsterConflict_Runtime")))
    {
        UOCR13CompactOsterSubsystem* Compact = World->GetSubsystem<UOCR13CompactOsterSubsystem>();
        if (!Compact || !Compact->IsCompactLayoutReady())
        {
            UE_LOG(LogTemp, Warning,
                TEXT("R13 deployment held: compact Oster layout is not ready for %s."), *GetName());
            ClientCompleteDeployment(false);
            return;
        }
    }

    ServerSetLobbyReady_Implementation(true);
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

    // Riflemen are the flexible/default slot. Specialist roles are unique inside a four-person squad.
    // Do not short-circuit just because a legacy pre-deployment default already equals RequestedRole:
    // the newly selected squad may already contain that specialist.
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
