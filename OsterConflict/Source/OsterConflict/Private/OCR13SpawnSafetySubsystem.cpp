#include "OCR13SpawnSafetySubsystem.h"

#include "OCCharacter.h"
#include "OCPlayerController.h"
#include "OCPlayerState.h"
#include "OCTeamSpawnPoint.h"

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
    if (!PC || !Character) return false;

    FVector SafeLocation;
    const FVector Original = Character->GetActorLocation();
    if (!ResolveGroundAt(Original, Character, SafeLocation))
    {
        if (!ResolveSafeTeamFallback(PC, Character, SafeLocation))
        {
            UE_LOG(LogTemp, Error,
                TEXT("R13 spawn safety rejected player spawn: no walkable collision under requested or fallback locations. player=%s location=%s"),
                *PC->GetName(), *Original.ToCompactString());
            return false;
        }
    }

    const float VerticalError = FMath::Abs(Original.Z - SafeLocation.Z);
    if (VerticalError > 8.0f)
    {
        Character->SetActorLocation(SafeLocation, false, nullptr, ETeleportType::TeleportPhysics);
        UE_LOG(LogTemp, Display,
            TEXT("R13 spawn safety grounded %s: %s -> %s"),
            *PC->GetName(), *Original.ToCompactString(), *SafeLocation.ToCompactString());
    }
    return true;
}

void UOCR13SpawnSafetySubsystem::Tick(float DeltaTime)
{
    (void)DeltaTime;

    UWorld* World = GetWorld();
    if (!World || World->GetNetMode() == NM_Client) return;

    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        AOCPlayerController* PC = Cast<AOCPlayerController>(It->Get());
        if (!PC) continue;
        AOCCharacter* Character = Cast<AOCCharacter>(PC->GetPawn());
        if (!Character) continue;

        if (const TWeakObjectPtr<AOCCharacter>* Last = LastValidatedPawn.Find(PC))
        {
            if (Last->Get() == Character) continue;
        }

        if (ValidateNewPawn(PC, Character))
        {
            LastValidatedPawn.Add(PC, Character);
            PC->ClientCompleteDeployment(true);
            continue;
        }

        if (AOCPlayerState* State = PC->GetPlayerState<AOCPlayerState>())
        {
            State->SetLobbyReadyServer(false);
        }
        PC->UnPossess();
        Character->Destroy();
        LastValidatedPawn.Remove(PC);
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
