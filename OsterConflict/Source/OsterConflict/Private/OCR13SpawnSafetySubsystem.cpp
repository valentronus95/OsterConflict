#include "OCR13SpawnSafetySubsystem.h"

#include "OCCharacter.h"
#include "OCPlayerController.h"
#include "OCPlayerState.h"
#include "OCTeamSpawnPoint.h"

#include "Engine/EngineBaseTypes.h"
#include "Engine/World.h"
#include "EngineUtils.h"

bool UOCR13SpawnSafetySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

bool UOCR13SpawnSafetySubsystem::ResolveGroundAt(
    const FVector& XYLocation, const AActor* IgnoreActor, FVector& OutCharacterLocation) const
{
    UWorld* World = GetWorld();
    if (!World) return false;

    // The current Oster runtime sector is intentionally near Z=0. Trace from well above all normal street geometry
    // so a pawn that accidentally started below the ground can still be recovered deterministically.
    const FVector Start(XYLocation.X, XYLocation.Y, 2200.0f);
    const FVector End(XYLocation.X, XYLocation.Y, -3500.0f);
    FCollisionQueryParams Params(TEXT("R13SpawnGround"), false);
    if (IgnoreActor) Params.AddIgnoredActor(IgnoreActor);

    FHitResult Hit;
    if (!World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params)) return false;
    if (!Hit.bBlockingHit || Hit.ImpactNormal.Z < 0.55f) return false;

    // OCCharacter uses a 92 cm capsule half-height. Keep a small safety margin above the contact surface.
    constexpr float SpawnCapsuleCenterOffsetCm = 104.0f;
    OutCharacterLocation = FVector(XYLocation.X, XYLocation.Y, Hit.ImpactPoint.Z + SpawnCapsuleCenterOffsetCm);
    return true;
}

bool UOCR13SpawnSafetySubsystem::ResolveSafeTeamFallback(
    const AOCPlayerController* PC, const AActor* IgnoreActor, FVector& OutCharacterLocation) const
{
    if (!PC || !GetWorld()) return false;
    const AOCPlayerState* State = PC->GetPlayerState<AOCPlayerState>();
    const EOCTeam Team = State ? State->GetTeamId() : EOCTeam::None;
    if (Team == EOCTeam::None) return false;

    const FName Requested = PC->GetRequestedDeploymentSpawn();
    for (int32 Pass = 0; Pass < 3; ++Pass)
    {
        for (TActorIterator<AOCTeamSpawnPoint> It(GetWorld()); It; ++It)
        {
            const AOCTeamSpawnPoint* Point = *It;
            if (!Point || Point->GetTeamId() != Team || !Point->IsAvailableForTeam(Team)) continue;

            bool bMatches = false;
            if (Pass == 0)
            {
                bMatches = Requested == TEXT("BASE") ? Point->IsBaseSpawn()
                    : (!Point->IsBaseSpawn() && Point->GetLinkedCapturePointId() == Requested);
            }
            else if (Pass == 1)
            {
                bMatches = Point->IsBaseSpawn();
            }
            else
            {
                bMatches = true;
            }
            if (!bMatches) continue;

            if (ResolveGroundAt(Point->GetActorLocation(), IgnoreActor, OutCharacterLocation)) return true;
        }
    }

    // Last-resort compact-layout bases. These are inside the R13 Oster ground bounds and are only accepted if
    // collision is actually found there. No collision means no spawn, rather than inventing another void coordinate.
    const FVector CompactBase = Team == EOCTeam::TeamTwo
        ? FVector(20000.0f, -19000.0f, 0.0f)
        : FVector(-64000.0f, 44000.0f, 0.0f);
    if (ResolveGroundAt(CompactBase, IgnoreActor, OutCharacterLocation)) return true;

    return ResolveGroundAt(FVector(-22500.0f, 12500.0f, 0.0f), IgnoreActor, OutCharacterLocation);
}

bool UOCR13SpawnSafetySubsystem::ValidateNewPawn(AOCPlayerController* PC, AOCCharacter* Character)
{
    UWorld* World = GetWorld();
    if (!PC || !Character || !World) return false;

    const FVector Original = Character->GetActorLocation();
    FVector GroundCandidate;
    bool bUsedTeamFallback = false;

    if (!ResolveGroundAt(Original, Character, GroundCandidate))
    {
        if (!ResolveSafeTeamFallback(PC, Character, GroundCandidate))
        {
            UE_LOG(LogTemp, Error,
                TEXT("R13 spawn safety rejected player spawn: no walkable collision under requested or fallback locations. player=%s location=%s"),
                *PC->GetName(), *Original.ToCompactString());
            return false;
        }
        bUsedTeamFallback = true;
    }

    // A downward trace proves there is ground, not that the full character capsule is clear. Ask UWorld for the
    // nearest non-overlapping placement before accepting the candidate so a spawn beside fences/props/buildings does
    // not exchange the old void-fall bug for a pawn born inside blocking geometry.
    FVector CollisionSafeLocation = GroundCandidate;
    if (!World->FindTeleportSpot(Character, CollisionSafeLocation, Character->GetActorRotation()))
    {
        FVector FallbackGround;
        if (bUsedTeamFallback || !ResolveSafeTeamFallback(PC, Character, FallbackGround))
        {
            UE_LOG(LogTemp, Error,
                TEXT("R13 spawn safety rejected player spawn: ground exists but no collision-clear capsule placement was found. player=%s candidate=%s"),
                *PC->GetName(), *GroundCandidate.ToCompactString());
            return false;
        }

        CollisionSafeLocation = FallbackGround;
        if (!World->FindTeleportSpot(Character, CollisionSafeLocation, Character->GetActorRotation()))
        {
            UE_LOG(LogTemp, Error,
                TEXT("R13 spawn safety rejected fallback spawn: team fallback also overlaps blocking geometry. player=%s fallback=%s"),
                *PC->GetName(), *FallbackGround.ToCompactString());
            return false;
        }
        bUsedTeamFallback = true;
    }

    const FVector PlacementDelta = Original - CollisionSafeLocation;
    if (PlacementDelta.SizeSquared() > FMath::Square(8.0f))
    {
        Character->SetActorLocation(CollisionSafeLocation, false, nullptr, ETeleportType::TeleportPhysics);
        UE_LOG(LogTemp, Display,
            TEXT("R13 spawn safety grounded/cleared %s: %s -> %s fallback=%s"),
            *PC->GetName(), *Original.ToCompactString(), *CollisionSafeLocation.ToCompactString(),
            bUsedTeamFallback ? TEXT("yes") : TEXT("no"));
    }
    return true;
}

void UOCR13SpawnSafetySubsystem::Tick(float DeltaTime)
{
    (void)DeltaTime;

    UWorld* World = GetWorld();
    if (!World || World->GetNetMode() == NM_Client) return;

    const bool bAutomationAutoDeploy = World->URL.HasOption(TEXT("AutoDeploy=1"));

    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        AOCPlayerController* PC = Cast<AOCPlayerController>(It->Get());
        if (!PC) continue;
        AOCCharacter* Character = Cast<AOCCharacter>(PC->GetPawn());
        if (!Character) continue;

        const TWeakObjectPtr<AOCPlayerController> PCKey(PC);
        if (const TWeakObjectPtr<AOCCharacter>* Last = LastValidatedPawn.Find(PCKey))
        {
            if (Last->Get() == Character) continue;
        }

        // Only the initial human deployment is gated. Once a player has legitimately entered gameplay, normal death/
        // round respawns keep using the existing game flow. AutoDeploy=1 remains an explicit automation/smoke exception.
        if (!PC->HasCompletedR13InitialDeployment())
        {
            const bool bStagedCommitAuthorized = PC->ConsumeR13DeploymentCommitAuthorization();
            if (!bStagedCommitAuthorized && !bAutomationAutoDeploy)
            {
                if (AOCPlayerState* State = PC->GetPlayerState<AOCPlayerState>())
                {
                    State->SetLobbyReadyServer(false);
                }
                PC->UnPossess();
                Character->Destroy();
                LastValidatedPawn.Remove(PCKey);
                PC->ClientCompleteDeployment(false);
                UE_LOG(LogTemp, Warning,
                    TEXT("R13 spawn safety rejected initial pawn without staged deployment commit: player=%s"),
                    *PC->GetName());
                continue;
            }
        }

        if (ValidateNewPawn(PC, Character))
        {
            PC->MarkR13InitialDeploymentCompleted();
            LastValidatedPawn.Add(PCKey, TWeakObjectPtr<AOCCharacter>(Character));
            PC->ClientCompleteDeployment(true);
            continue;
        }

        if (AOCPlayerState* State = PC->GetPlayerState<AOCPlayerState>())
        {
            State->SetLobbyReadyServer(false);
        }
        PC->UnPossess();
        Character->Destroy();
        LastValidatedPawn.Remove(PCKey);
        PC->ClientCompleteDeployment(false);
    }

    for (auto It = LastValidatedPawn.CreateIterator(); It; ++It)
    {
        if (!It.Key().IsValid() || !It.Value().IsValid()) It.RemoveCurrent();
    }
}

TStatId UOCR13SpawnSafetySubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCR13SpawnSafetySubsystem, STATGROUP_Tickables);
}
