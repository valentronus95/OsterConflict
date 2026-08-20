#include "OCR13MuseumRespawnSubsystem.h"

#include "OCCharacter.h"
#include "OCGameMode.h"
#include "OCPlayerController.h"
#include "OCWorldSectorOster.h"

#include "Engine/EngineTypes.h"
#include "Engine/World.h"

bool UOCR13MuseumRespawnSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

bool UOCR13MuseumRespawnSubsystem::ResolveGroundedCandidate(
    const FVector& XYLocation, const AActor* IgnoreActor, FVector& OutCharacterLocation) const
{
    UWorld* World = GetWorld();
    if (!World) return false;

    const FVector Start(XYLocation.X, XYLocation.Y, 2400.0f);
    const FVector End(XYLocation.X, XYLocation.Y, -3500.0f);
    FCollisionQueryParams Params(TEXT("R13MuseumRespawnGround"), false);
    if (IgnoreActor) Params.AddIgnoredActor(IgnoreActor);

    FHitResult Hit;
    if (!World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params)) return false;
    if (!Hit.bBlockingHit || Hit.ImpactNormal.Z < 0.55f) return false;

    constexpr float SpawnCapsuleCenterOffsetCm = 104.0f;
    OutCharacterLocation = FVector(XYLocation.X, XYLocation.Y,
        Hit.ImpactPoint.Z + SpawnCapsuleCenterOffsetCm);
    return true;
}

bool UOCR13MuseumRespawnSubsystem::PlaceHumanNearMuseum(
    AOCPlayerController* PC, AOCCharacter* Character) const
{
    UWorld* World = GetWorld();
    if (!World || !PC || !Character) return false;

    const FVector Museum = AOCWorldSectorOster::MuseumAnchor();

    // Gameplay-only staging ring beside the photographed concrete-slab approach. The first point is just beyond
    // the current approach/tree line so inspection starts with the museum in front of the player, not inside it.
    static const FVector CandidateOffsets[] =
    {
        FVector(0.0f, -5700.0f, 0.0f),
        FVector(1700.0f, -5600.0f, 0.0f),
        FVector(-1700.0f, -5600.0f, 0.0f),
        FVector(2500.0f, -4700.0f, 0.0f),
        FVector(-2500.0f, -4700.0f, 0.0f),
        FVector(0.0f, -6500.0f, 0.0f)
    };

    for (const FVector& Offset : CandidateOffsets)
    {
        FVector GroundedLocation;
        if (!ResolveGroundedCandidate(Museum + Offset, Character, GroundedLocation)) continue;

        const FVector ToMuseum = Museum - GroundedLocation;
        FRotator Facing = ToMuseum.Rotation();
        Facing.Pitch = 0.0f;
        Facing.Roll = 0.0f;

        FVector CollisionSafeLocation = GroundedLocation;
        if (!World->FindTeleportSpot(Character, CollisionSafeLocation, Facing)) continue;

        Character->SetActorLocationAndRotation(
            CollisionSafeLocation, Facing, false, nullptr, ETeleportType::TeleportPhysics);
        PC->SetControlRotation(Facing);

        UE_LOG(LogTemp, Display,
            TEXT("R13 museum respawn: human %s placed near MuseumAnchor at %s (offset=%s)."),
            *PC->GetName(), *CollisionSafeLocation.ToCompactString(), *Offset.ToCompactString());
        return true;
    }

    UE_LOG(LogTemp, Warning,
        TEXT("R13 museum respawn: no collision-clear museum staging point found for %s; keeping validated game spawn."),
        *PC->GetName());
    return false;
}

void UOCR13MuseumRespawnSubsystem::Tick(float DeltaTime)
{
    (void)DeltaTime;

    UWorld* World = GetWorld();
    if (!World || World->GetNetMode() == NM_Client) return;
    if (!World->GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    const AOCGameMode* GameMode = World->GetAuthGameMode<AOCGameMode>();
    if (!GameMode || GameMode->IsFrontendOnlySession() || !GameMode->IsLocationTestMode()) return;

    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        AOCPlayerController* PC = Cast<AOCPlayerController>(It->Get());
        if (!PC || !PC->HasCompletedR13InitialDeployment()) continue;

        AOCCharacter* Character = Cast<AOCCharacter>(PC->GetPawn());
        if (!Character) continue;

        const TWeakObjectPtr<AOCPlayerController> PCKey(PC);
        if (const TWeakObjectPtr<AOCCharacter>* LastPawn = LastMuseumPlacedPawn.Find(PCKey))
        {
            if (LastPawn->Get() == Character) continue;
        }

        // SpawnSafety owns deployment authorization and basic grounding. Once that pass has accepted the pawn,
        // move each newly possessed HUMAN pawn to the museum staging ring. A new pawn pointer means a respawn.
        PlaceHumanNearMuseum(PC, Character);
        LastMuseumPlacedPawn.Add(PCKey, TWeakObjectPtr<AOCCharacter>(Character));
    }

    for (auto It = LastMuseumPlacedPawn.CreateIterator(); It; ++It)
    {
        if (!It.Key().IsValid() || !It.Value().IsValid()) It.RemoveCurrent();
    }
}

TStatId UOCR13MuseumRespawnSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCR13MuseumRespawnSubsystem, STATGROUP_Tickables);
}
