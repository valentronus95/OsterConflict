#include "OCPlayerController.h"

#include "OCGameMode.h"
#include "OCPlayerState.h"
#include "OCR13CompactOsterSubsystem.h"
#include "OCTeamSpawnPoint.h"

#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"

namespace
{
    bool FindGroundForPawn(UWorld* World, APawn* Pawn, const FVector& ProbeLocation, FVector& OutLocation)
    {
        if (!World || !Pawn) return false;

        FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(R13DeploymentGround), false, Pawn);
        QueryParams.bReturnPhysicalMaterial = false;

        FHitResult Hit;
        const FVector TraceStart(ProbeLocation.X, ProbeLocation.Y, ProbeLocation.Z + 2600.0f);
        const FVector TraceEnd(ProbeLocation.X, ProbeLocation.Y, ProbeLocation.Z - 7000.0f);
        if (!World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams) ||
            !Hit.bBlockingHit || Hit.ImpactNormal.Z < 0.55f)
        {
            return false;
        }

        float HalfHeight = 92.0f;
        if (const ACharacter* Character = Cast<ACharacter>(Pawn))
        {
            if (const UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
            {
                HalfHeight = FMath::Max(20.0f, Capsule->GetScaledCapsuleHalfHeight());
            }
        }

        OutLocation = Hit.ImpactPoint + FVector(0.0f, 0.0f, HalfHeight + 5.0f);
        return true;
    }

    bool GroundCurrentPawn(AOCPlayerController* PC)
    {
        if (!PC || !PC->GetWorld()) return false;
        APawn* Pawn = PC->GetPawn();
        if (!Pawn) return false;

        FVector SafeLocation;
        if (FindGroundForPawn(PC->GetWorld(), Pawn, Pawn->GetActorLocation(), SafeLocation))
        {
            FHitResult SweepHit;
            if (Pawn->SetActorLocation(SafeLocation, true, &SweepHit, ETeleportType::TeleportPhysics))
            {
                return true;
            }
        }

        const AOCPlayerState* State = PC->GetPlayerState<AOCPlayerState>();
        const EOCTeam Team = State ? State->GetTeamId() : EOCTeam::None;
        if (Team == EOCTeam::None) return false;

        // Requested forward spawn can disappear between UI refresh and commit. Use only a valid team base as the
        // final recovery point; if even that has no blocking ground, keep the player in Deployment instead of void-fall.
        for (TActorIterator<AOCTeamSpawnPoint> It(PC->GetWorld()); It; ++It)
        {
            const AOCTeamSpawnPoint* Spawn = *It;
            if (!Spawn || !Spawn->IsBaseSpawn() || !Spawn->IsAvailableForTeam(Team)) continue;

            if (!FindGroundForPawn(PC->GetWorld(), Pawn, Spawn->GetActorLocation(), SafeLocation)) continue;
            const FRotator Rotation = Spawn->GetActorRotation();
            if (Pawn->SetActorLocationAndRotation(SafeLocation, Rotation, true, nullptr, ETeleportType::TeleportPhysics))
            {
                return true;
            }
        }

        return false;
    }
}

void AOCPlayerController::UIRequestSquad(int32 SquadId)
{
    if (!IsLocalController() || !bDeploymentPanelVisible) return;
    if (HasAuthority()) ServerRequestSquad_Implementation(SquadId); else ServerRequestSquad(SquadId);
}

void AOCPlayerController::UIRequestRole(EOCPlayerRole Role)
{
    if (!IsLocalController() || !bDeploymentPanelVisible) return;
    if (HasAuthority()) ServerRequestRole_Implementation(Role); else ServerRequestRole(Role);
}

void AOCPlayerController::UICommitDeployment()
{
    if (!IsLocalController() || !bDeploymentPanelVisible) return;

    // Keep the panel and UI input lock visible until ClientCompleteDeployment confirms an authoritative grounded pawn.
    if (HasAuthority()) ServerCommitDeployment_Implementation(); else ServerCommitDeployment();
    ApplyUIInputMode();
}

void AOCPlayerController::ServerRequestRole_Implementation(EOCPlayerRole RequestedRole)
{
    AOCGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AOCGameMode>() : nullptr;
    AOCPlayerState* State = GetPlayerState<AOCPlayerState>();
    if (!GameMode || !State) return;
    GameMode->RequestRoleChange(State, RequestedRole);
}

void AOCPlayerController::ServerCommitDeployment_Implementation()
{
    UWorld* World = GetWorld();
    AOCGameMode* GameMode = World ? World->GetAuthGameMode<AOCGameMode>() : nullptr;
    AOCPlayerState* State = GetPlayerState<AOCPlayerState>();
    if (!World || !GameMode || !State || State->IsBotPlayer())
    {
        ClientCompleteDeployment(false);
        return;
    }

    if (State->GetTeamId() == EOCTeam::None || State->GetSquadId() < 0)
    {
        ClientCompleteDeployment(false);
        return;
    }

    UOCR13CompactOsterSubsystem* Compact = World->GetSubsystem<UOCR13CompactOsterSubsystem>();
    if (!Compact || !Compact->IsCompactLayoutReady())
    {
        State->SetLobbyReadyServer(false);
        ClientCompleteDeployment(false);
        return;
    }

    if (!GetPawn())
    {
        State->SetLobbyReadyServer(true);
        GameMode->RestartPlayer(this);
    }

    APawn* SpawnedPawn = GetPawn();
    if (!SpawnedPawn || !GroundCurrentPawn(this))
    {
        if (SpawnedPawn)
        {
            UnPossess();
            SpawnedPawn->Destroy();
        }
        State->SetLobbyReadyServer(false);
        ClientCompleteDeployment(false);
        return;
    }

    State->SetLobbyReadyServer(true);
    ClientCompleteDeployment(true);
}

void AOCPlayerController::ClientCompleteDeployment_Implementation(bool bSuccess)
{
    if (!IsLocalController()) return;

    bFrontendMenuVisible = false;
    bDeploymentPanelVisible = !bSuccess;
    bAdminPanelVisible = false;
    bChatInputActive = false;
    ApplyUIInputMode();
}
