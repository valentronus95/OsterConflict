#include "OCPass45TrenchSetpieceSubsystem.h"

#include "OCGameMode.h"
#include "Engine/World.h"

bool UOCPass45TrenchSetpieceSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && World->GetNetMode() != NM_DedicatedServer &&
        (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

TStatId UOCPass45TrenchSetpieceSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCPass45TrenchSetpieceSubsystem, STATGROUP_Tickables);
}

void UOCPass45TrenchSetpieceSubsystem::Tick(float DeltaTime)
{
    if (bFinished) return;

    UWorld* World = GetWorld();
    if (!World || !World->IsGameWorld()) return;
    if (!World->GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    if (const AOCGameMode* GameMode = World->GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    // GAME_RECOVERY: the old automatic sandbag/rubble ring is removed from runtime completely.
    // It was a temporary visual-evidence setpiece and must not reappear through PIE URL options,
    // command-line leftovers, or normal gameplay. Keeping the subsystem as a one-shot no-op
    // avoids changing subsystem registration while guaranteeing authored_instances=0.
    bFinished = true;
    UE_LOG(LogTemp, Display,
        TEXT("GAME_RECOVERY_TRENCH_SETPIECE_RETIRED authored_instances=0 sandbags=0 rubble=0 runtime_spawn_path=disabled"));
}
