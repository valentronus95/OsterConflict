#include "OCBotPopulationPolicySubsystem.h"

#include "OCGameMode.h"
#include "Engine/World.h"

bool UOCBotPopulationPolicySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCBotPopulationPolicySubsystem::Tick(float DeltaTime)
{
    if (bPolicyApplied) return;

    UWorld* World = GetWorld();
    if (!World) return;

    AOCGameMode* GameMode = World->GetAuthGameMode<AOCGameMode>();
    if (!GameMode) return;

    if (GameMode->IsFrontendOnlySession() || GameMode->IsSandboxMode() || World->GetNetMode() == NM_DedicatedServer)
    {
        bPolicyApplied = true;
        return;
    }

    // Wait until the local/listen host is actually represented in the world so an explicit
    // "N bots" request remains N bots beside the human host instead of becoming N-1 later.
    if (GameMode->GetHumanPlayerCount() <= 0) return;

    bPolicyApplied = GameMode->RestoreExpectedLocalBotFill();
    if (bPolicyApplied)
    {
        UE_LOG(LogTemp, Display,
            TEXT("GAME_RECOVERY_BOT_FILL_RESTORED humans=%d bots=%d target_population=%d max_players=%d"),
            GameMode->GetHumanPlayerCount(), GameMode->GetBotPlayerCount(),
            GameMode->GetTargetPopulation(), GameMode->GetMaxPlayerSlots());
    }
}

TStatId UOCBotPopulationPolicySubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCBotPopulationPolicySubsystem, STATGROUP_Tickables);
}
