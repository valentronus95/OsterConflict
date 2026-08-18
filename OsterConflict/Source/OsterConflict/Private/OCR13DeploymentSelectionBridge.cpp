#include "OCPlayerController.h"

#include "OCGameMode.h"
#include "OCGameState.h"
#include "OCPlayerState.h"
#include "OCR13CompactOsterSubsystem.h"

#include "Engine/World.h"
#include "GameFramework/PlayerState.h"

bool AOCPlayerController::ConsumeR13DeploymentCommitAuthorization()
{
    UWorld* World = GetWorld();
    const double Now = World ? World->GetTimeSeconds() : TNumericLimits<double>::Max();
    const bool bFreshAuthorization = bR13DeploymentCommitAuthorized &&
        R13DeploymentCommitAuthorizationExpiresAt >= Now;

    bR13DeploymentCommitAuthorized = false;
    R13DeploymentCommitAuthorizationExpiresAt = -1.0;
    return bFreshAuthorization;
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
    if (HasAuthority()) ServerRequestRole_Implementation(RequestedRole);
    else ServerRequestRole(RequestedRole);
}

void AOCPlayerController::UICommitDeployment()
{
    if (!bDeploymentPanelVisible || bFrontendMenuVisible || bSettingsVisible) return;

    // Keep the deployment UI visible while the authoritative server checks that compact Oster has already
    // relocated the old source PlayerStarts and then creates a pawn. OCR13SpawnSafetySubsystem owns the
    // collision-grounding/fallback decision and closes this panel only after validating the resulting pawn.
    if (HasAuthority()) ServerCommitDeployment_Implementation();
    else ServerCommitDeployment();
}

void AOCPlayerController::ServerCommitDeployment_Implementation()
{
    UWorld* World = GetWorld();
    AOCPlayerState* State = GetPlayerState<AOCPlayerState>();
    bR13DeploymentCommitAuthorized = false;
    R13DeploymentCommitAuthorizationExpiresAt = -1.0;

    if (!World || !State || State->IsBotPlayer() || State->GetTeamId() == EOCTeam::None || State->GetSquadId() < 0)
    {
        if (State) State->SetLobbyReadyServer(false);
        ClientCompleteDeployment(false);
        return;
    }

    // The source world still authors legacy bases far outside the R13 compact crop. Never let a human become ready
    // until OCR13CompactOsterSubsystem has cropped the world, moved objectives and relocated TeamSpawn actors.
    if (World->GetMapName().Contains(TEXT("OsterConflict_Runtime")))
    {
        UOCR13CompactOsterSubsystem* Compact = World->GetSubsystem<UOCR13CompactOsterSubsystem>();
        if (!Compact || !Compact->IsCompactLayoutReady())
        {
            State->SetLobbyReadyServer(false);
            UE_LOG(LogTemp, Warning,
                TEXT("R13 deployment held: compact Oster layout is not ready for %s."), *GetName());
            ClientCompleteDeployment(false);
            return;
        }
    }

    // This token is consumed by OCR13SpawnSafetySubsystem on the first new human pawn. Legacy F4/ReadyAction may
    // still exist for compatibility, but it cannot make an accepted gameplay pawn because it never grants this token.
    // A two-second expiry prevents a failed/aborted RestartPlayer from leaving a stale authorization behind.
    bR13DeploymentCommitAuthorized = true;
    R13DeploymentCommitAuthorizationExpiresAt = World->GetTimeSeconds() + 2.0;
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
                    // A newly selected squad can conflict with the player's old/default specialist role. Force a
                    // neutral flexible role so replicated state cannot accidentally masquerade as an accepted choice.
                    State->SetRoleServer(EOCPlayerRole::Rifleman);
                    State->SetLobbyReadyServer(false);
                    return false;
                }
            }
        }
    }

    State->SetRoleServer(RequestedRole);
    State->SetLobbyReadyServer(false);
    return true;
}
