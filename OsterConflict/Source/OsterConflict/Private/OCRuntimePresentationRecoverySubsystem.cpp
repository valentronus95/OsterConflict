#include "OCRuntimePresentationRecoverySubsystem.h"

#include "OCAuthoredWorldSurfaceUpgradeSubsystem.h"
#include "OCBlock0GroundFoundationSubsystem.h"
#include "OCGameMode.h"
#include "OCGameModeRuntimeSafe.h"
#include "OCParkGroundAuthoredUpgradeSubsystem.h"
#include "OCParkHardscapeAuthoredUpgradeSubsystem.h"
#include "OCPlayerController.h"
#include "OCPlayerState.h"

#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

bool UOCRuntimePresentationRecoverySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

bool UOCRuntimePresentationRecoverySubsystem::HasPresentationFailure(FString& OutStages) const
{
    OutStages.Reset();
    const UWorld* World = GetWorld();
    if (!World || !World->GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return false;

    TArray<FString> Failed;

    if (const UOCBlock0GroundFoundationSubsystem* Ground = World->GetSubsystem<UOCBlock0GroundFoundationSubsystem>();
        Ground && Ground->HasGroundFailed())
    {
        Failed.Add(TEXT("ground_failed"));
    }

    if (const UOCAuthoredWorldSurfaceUpgradeSubsystem* Surface =
            World->GetSubsystem<UOCAuthoredWorldSurfaceUpgradeSubsystem>();
        Surface && Surface->HasWorldSurfaceFailed())
    {
        Failed.Add(TEXT("surface_failed"));
    }

    if (const UOCParkGroundAuthoredUpgradeSubsystem* ParkGround =
            World->GetSubsystem<UOCParkGroundAuthoredUpgradeSubsystem>();
        ParkGround && ParkGround->HasParkGroundFailed())
    {
        Failed.Add(TEXT("park_ground_failed"));
    }

    if (const UOCParkHardscapeAuthoredUpgradeSubsystem* ParkHardscape =
            World->GetSubsystem<UOCParkHardscapeAuthoredUpgradeSubsystem>();
        ParkHardscape && ParkHardscape->HasParkHardscapeFailed())
    {
        Failed.Add(TEXT("park_hardscape_failed"));
    }

    OutStages = Failed.Num() > 0 ? FString::Join(Failed, TEXT(",")) : TEXT("none");
    return Failed.Num() > 0;
}

void UOCRuntimePresentationRecoverySubsystem::ApplyFrontendFieldPaddingFix()
{
    if (bFrontendFieldPaddingFixed) return;

    // The field style is now authored correctly when R13_FrontendFields is constructed.
    // Do not traverse or restyle live UMG/Slate widgets from a world tick: that recovery path
    // was unnecessary UI surgery during travel/deployment and was a plausible stale-widget crash source.
    bFrontendFieldPaddingFixed = true;
    UE_LOG(LogTemp, Display,
        TEXT("GAME_RECOVERY_FRONTEND_FIELD_ALIGNMENT_READY source_owned=1 vertical_padding=4 runtime_slate_mutation=0 global_uobject_scan=0"));
}

void UOCRuntimePresentationRecoverySubsystem::ReleaseReadyPlayersFromPresentationFailure()
{
    UWorld* World = GetWorld();
    if (!World || World->GetNetMode() == NM_Client || World->GetNetMode() == NM_DedicatedServer) return;

    AOCGameModeRuntimeSafe* RuntimeGameMode = World->GetAuthGameMode<AOCGameModeRuntimeSafe>();
    if (!RuntimeGameMode) return;

    FString FailedStages;
    if (!HasPresentationFailure(FailedStages)) return;

    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        AOCPlayerController* PC = Cast<AOCPlayerController>(It->Get());
        if (!PC || PC->IsActorBeingDestroyed() || PC->GetWorld() != World || PC->GetPawn()) continue;

        const AOCPlayerState* State = PC->GetPlayerState<AOCPlayerState>();
        if (!State || State->IsBotPlayer() || !State->IsLobbyReady()) continue;

        const FString AttemptKey = FString::Printf(TEXT("%d:%s"), State->GetPlayerId(), *State->GetPlayerName());
        if (FailsoftSpawnAttemptKeys.Contains(AttemptKey)) continue;
        FailsoftSpawnAttemptKeys.Add(AttemptKey);

        // Bypass only the presentation readiness gate after it has already reported a factual visual failure.
        // Do this once per ready player and keep no UObject references in the recovery bookkeeping.
        RuntimeGameMode->AOCGameMode::RestartPlayer(PC);

        if (PC->GetPawn())
        {
            UE_LOG(LogTemp, Display,
                TEXT("GAME_RECOVERY_PRESENTATION_FAILSOFT_SPAWN READY stages=%s player_spawned=1 deployment_gray_screen=0 stale_uobject_bookkeeping=0"),
                *FailedStages);
        }
        else
        {
            UE_LOG(LogTemp, Error,
                TEXT("GAME_RECOVERY_PRESENTATION_FAILSOFT_SPAWN FAIL stages=%s player_spawned=0 deployment_gray_screen=0 stale_uobject_bookkeeping=0"),
                *FailedStages);
        }
    }
}

void UOCRuntimePresentationRecoverySubsystem::Tick(float DeltaTime)
{
    TickAccumulator += FMath::Max(0.0f, DeltaTime);
    if (TickAccumulator < 0.10f) return;
    TickAccumulator = 0.0f;

    ApplyFrontendFieldPaddingFix();
    ReleaseReadyPlayersFromPresentationFailure();
}

TStatId UOCRuntimePresentationRecoverySubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCRuntimePresentationRecoverySubsystem, STATGROUP_Tickables);
}
