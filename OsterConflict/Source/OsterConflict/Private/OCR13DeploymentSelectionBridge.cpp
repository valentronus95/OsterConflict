#include "OCPlayerController.h"

#include "OCGameMode.h"
#include "OCGameState.h"
#include "OCPlayerState.h"
#include "OCR13CompactOsterSubsystem.h"

#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "HAL/PlatformTime.h"
#include "TimerManager.h"

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

    if (!World || !State || State->IsBotPlayer() || State->GetTeamId() == EOCTeam::None || State->GetSquadId() < 0)
    {
        bR13DeploymentCommitAuthorized = false;
        R13DeploymentCommitAuthorizationExpiresAt = -1.0;
        if (State) State->SetLobbyReadyServer(false);
        ClientCompleteDeployment(false);
        return;
    }

    // Ignore a second click while the one-shot spawn request is already queued. The old synchronous path could
    // re-enter RestartPlayer from the UI click stack and made a slow spawn look exactly like a frozen game.
    if (bR13DeploymentCommitAuthorized && R13DeploymentCommitAuthorizationExpiresAt >= World->GetTimeSeconds())
    {
        UE_LOG(LogTemp, Verbose, TEXT("R13 deployment ignored duplicate commit while spawn is pending: %s."), *GetName());
        return;
    }
    bR13DeploymentCommitAuthorized = false;
    R13DeploymentCommitAuthorizationExpiresAt = -1.0;

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

    // Grant the one-shot token now, but perform RestartPlayer on the next game tick instead of inside the button/RPC
    // call stack. That lets Slate finish the click and gives us a persistent timing marker around the expensive spawn.
    bR13DeploymentCommitAuthorized = true;
    R13DeploymentCommitAuthorizationExpiresAt = World->GetTimeSeconds() + 2.0;

    TWeakObjectPtr<AOCPlayerController> WeakController(this);
    World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [WeakController]()
    {
        AOCPlayerController* PC = WeakController.Get();
        UWorld* SpawnWorld = PC ? PC->GetWorld() : nullptr;
        AOCPlayerState* SpawnState = PC ? PC->GetPlayerState<AOCPlayerState>() : nullptr;
        if (!PC || !SpawnWorld || !SpawnState)
        {
            return;
        }

        UE_LOG(LogTemp, Display,
            TEXT("R13 deployment spawn BEGIN player=%s worldTime=%.3f requested=%s."),
            *PC->GetName(), SpawnWorld->GetTimeSeconds(), *PC->GetRequestedDeploymentSpawn().ToString());

        const double SpawnBeginSeconds = FPlatformTime::Seconds();
        PC->ServerSetLobbyReady_Implementation(true);
        const double SpawnDurationMs = (FPlatformTime::Seconds() - SpawnBeginSeconds) * 1000.0;

        UE_LOG(LogTemp, Display,
            TEXT("R13 deployment spawn END player=%s durationMs=%.1f pawn=%s."),
            *PC->GetName(), SpawnDurationMs, PC->GetPawn() ? *PC->GetPawn()->GetName() : TEXT("none"));

        // RestartPlayer is synchronous. If it returned without a pawn there is nothing for SpawnSafety to validate,
        // so fail back to deployment instead of leaving the user on a dead, apparently frozen screen forever.
        if (!PC->GetPawn())
        {
            PC->bR13DeploymentCommitAuthorized = false;
            PC->R13DeploymentCommitAuthorizationExpiresAt = -1.0;
            SpawnState->SetLobbyReadyServer(false);
            UE_LOG(LogTemp, Error,
                TEXT("R13 deployment spawn FAILED: RestartPlayer returned without a pawn for %s."), *PC->GetName());
            PC->ClientCompleteDeployment(false);
        }
    }));
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
        if (const AOCGameState* CurrentGameState = GetGameState<AOCGameState>())
        {
            for (APlayerState* RawState : CurrentGameState->PlayerArray)
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
