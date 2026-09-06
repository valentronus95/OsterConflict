#include "OCDeathRecoverySubsystem.h"

#include "OCGameMode.h"
#include "OCGameState.h"
#include "OCPlayerController.h"

#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpectatorPawn.h"
#include "HAL/PlatformTime.h"

namespace
{
    constexpr double RespawnDeadlineSeconds = 10.0;
    constexpr double RecoveryGraceSeconds = 0.35;
    constexpr double RetryIntervalSeconds = 0.75;
    constexpr int32 MaxFallbackAttempts = 4;
}

bool UOCDeathRecoverySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && World->GetNetMode() != NM_DedicatedServer &&
        (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

TStatId UOCDeathRecoverySubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCDeathRecoverySubsystem, STATGROUP_Tickables);
}

void UOCDeathRecoverySubsystem::Tick(float DeltaTime)
{
    (void)DeltaTime;
    UWorld* World = GetWorld();
    if (!World || !World->IsGameWorld()) return;
    if (!World->GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    AOCPlayerController* PC = Cast<AOCPlayerController>(World->GetFirstPlayerController());
    if (!PC || !PC->IsLocalController()) return;
    TrackedController = PC;

    APawn* CurrentPawn = PC->GetPawn();
    const bool bSpectatorPawn = CurrentPawn && CurrentPawn->IsA<ASpectatorPawn>();
    const bool bGameplayPossession = CurrentPawn != nullptr && !bSpectatorPawn;

    if (bGameplayPossession)
    {
        if (bDeathWindowActive)
        {
            RecoverRespawn(*PC);
        }
        bHadGameplayCharacter = true;
        return;
    }

    // First-launch deployment intentionally has no pawn. Only a controller that already owned real gameplay
    // may enter the death recovery window, so this cannot bypass TEAM/SQUAD/ROLE/SPAWN.
    if (!bHadGameplayCharacter) return;

    if (!bDeathWindowActive)
    {
        BeginDeathWindow(*PC);
        return;
    }

    const double Now = FPlatformTime::Seconds();
    const double Elapsed = FMath::Max(0.0, Now - DeathStartWallTimeSeconds);
    if (Elapsed < RespawnDeadlineSeconds + RecoveryGraceSeconds) return;

    const AOCGameState* MatchState = World->GetGameState<AOCGameState>();
    if (MatchState && MatchState->GetOCMatchPhase() == EOCMatchPhase::Ended) return;

    // The authoritative GameMode already tried at exactly 10 s. If we are still spectator/no-pawn,
    // retry a few times instead of leaving the player in a permanent grey/debug state.
    if (PC->HasAuthority() && FallbackAttempts < MaxFallbackAttempts &&
        (Now - LastFallbackAttemptWallTimeSeconds) >= RetryIntervalSeconds)
    {
        LastFallbackAttemptWallTimeSeconds = Now;
        ++FallbackAttempts;
        if (AOCGameMode* GameMode = World->GetAuthGameMode<AOCGameMode>())
        {
            UE_LOG(LogTemp, Warning,
                TEXT("GAME_RECOVERY_RESPAWN_FALLBACK attempt=%d elapsed=%.2f spectator=%d"),
                FallbackAttempts, Elapsed, bSpectatorPawn ? 1 : 0);
            GameMode->RestartPlayer(PC);
        }
    }

    if (Elapsed >= RespawnDeadlineSeconds + 4.0 && FallbackAttempts >= MaxFallbackAttempts)
    {
        UE_LOG(LogTemp, Error,
            TEXT("GAME_RECOVERY_RESPAWN_FAIL elapsed=%.2f attempts=%d pawn=%s authority=%d"),
            Elapsed,
            FallbackAttempts,
            CurrentPawn ? *CurrentPawn->GetClass()->GetName() : TEXT("none"),
            PC->HasAuthority() ? 1 : 0);
    }
}

void UOCDeathRecoverySubsystem::BeginDeathWindow(AOCPlayerController& Controller)
{
    bDeathWindowActive = true;
    DeathStartWallTimeSeconds = FPlatformTime::Seconds();
    LastFallbackAttemptWallTimeSeconds = DeathStartWallTimeSeconds;
    FallbackAttempts = 0;

    // Spectator is a camera bridge only. It must not become a free-fly debug mode while the respawn timer runs.
    Controller.SetIgnoreMoveInput(true);
    Controller.SetIgnoreLookInput(false);
    Controller.bShowMouseCursor = false;

    UE_LOG(LogTemp, Display,
        TEXT("GAME_RECOVERY_DEATH_WINDOW_BEGIN respawn_target_s=10 spectator_flight=0"));
}

void UOCDeathRecoverySubsystem::RecoverRespawn(AOCPlayerController& Controller)
{
    const double Elapsed = DeathStartWallTimeSeconds > 0.0
        ? FMath::Max(0.0, FPlatformTime::Seconds() - DeathStartWallTimeSeconds)
        : 0.0;

    bDeathWindowActive = false;
    DeathStartWallTimeSeconds = 0.0;
    LastFallbackAttemptWallTimeSeconds = 0.0;
    FallbackAttempts = 0;

    RestoreGameplayInput(Controller);

    UE_LOG(LogTemp, Display,
        TEXT("GAME_RECOVERY_RESPAWN_READY elapsed=%.2f input_restored=1 spectator_released=1"), Elapsed);
}

void UOCDeathRecoverySubsystem::RestoreGameplayInput(AOCPlayerController& Controller)
{
    // Close any stale gameplay overlays that survived pawn destruction. Each public UI close path is idempotent.
    if (Controller.IsSettingsVisible()) Controller.UICloseSettings();
    if (Controller.IsAdminPanelVisible()) Controller.UICloseAdmin();
    if (Controller.IsChatInputActive()) Controller.UIEndChatInput();
    if (Controller.IsDeploymentPanelVisible()) Controller.UICloseDeployment();
    if (Controller.IsFrontendMenuVisible()) Controller.UIToggleFrontend();

    Controller.ResetIgnoreMoveInput();
    Controller.ResetIgnoreLookInput();
    Controller.bShowMouseCursor = false;
    Controller.SetInputMode(FInputModeGameOnly());
    if (APawn* Pawn = Controller.GetPawn()) Controller.SetViewTarget(Pawn);
}
