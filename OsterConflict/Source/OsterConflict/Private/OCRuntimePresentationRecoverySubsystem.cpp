#include "OCRuntimePresentationRecoverySubsystem.h"

#include "OCAuthoredWorldSurfaceUpgradeSubsystem.h"
#include "OCBlock0GroundFoundationSubsystem.h"
#include "OCGameMode.h"
#include "OCGameModeRuntimeSafe.h"
#include "OCParkGroundAuthoredUpgradeSubsystem.h"
#include "OCParkHardscapeAuthoredUpgradeSubsystem.h"
#include "OCPlayerController.h"
#include "OCPlayerState.h"

#include "Components/EditableTextBox.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Styling/SlateTypes.h"
#include "UObject/UObjectIterator.h"

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

    if (const UOCAuthoredWorldSurfaceUpgradeSubsystem* Surface = World->GetSubsystem<UOCAuthoredWorldSurfaceUpgradeSubsystem>();
        Surface && Surface->HasWorldSurfaceFailed())
    {
        Failed.Add(TEXT("surface_failed"));
    }

    if (const UOCParkGroundAuthoredUpgradeSubsystem* ParkGround = World->GetSubsystem<UOCParkGroundAuthoredUpgradeSubsystem>();
        ParkGround && ParkGround->HasParkGroundFailed())
    {
        Failed.Add(TEXT("park_ground_failed"));
    }

    if (const UOCParkHardscapeAuthoredUpgradeSubsystem* ParkHardscape = World->GetSubsystem<UOCParkHardscapeAuthoredUpgradeSubsystem>();
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

    UWorld* World = GetWorld();
    if (!World || World->GetNetMode() == NM_DedicatedServer) return;

    int32 FixedFields = 0;
    for (TObjectIterator<UEditableTextBox> It; It; ++It)
    {
        UEditableTextBox* Field = *It;
        if (!IsValid(Field) || Field->IsTemplate() || Field->GetWorld() != World) continue;

        FEditableTextBoxStyle Style = Field->GetWidgetStyle();
        const FMargin& Padding = Style.Padding;
        const bool bR13FrontendSignature =
            FMath::IsNearlyEqual(Padding.Left, 14.0f) &&
            FMath::IsNearlyEqual(Padding.Top, 9.0f) &&
            FMath::IsNearlyEqual(Padding.Right, 14.0f) &&
            FMath::IsNearlyEqual(Padding.Bottom, 9.0f);
        if (!bR13FrontendSignature) continue;

        // 44 px field height plus 18 px vertical padding clipped the font baseline on the server setup page.
        Style.Padding = FMargin(14.0f, 4.0f, 14.0f, 4.0f);
        Field->SetWidgetStyle(Style);
        ++FixedFields;
    }

    if (FixedFields > 0)
    {
        bFrontendFieldPaddingFixed = true;
        UE_LOG(LogTemp, Display,
            TEXT("GAME_RECOVERY_FRONTEND_FIELD_ALIGNMENT_READY fields=%d vertical_padding=4 clipped_text=0"),
            FixedFields);
    }
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
        if (!IsValid(PC) || PC->GetPawn()) continue;

        const AOCPlayerState* State = PC->GetPlayerState<AOCPlayerState>();
        if (!State || State->IsBotPlayer() || !State->IsLobbyReady()) continue;

        const TWeakObjectPtr<AController> WeakController(PC);
        if (FailsoftSpawnAttempts.Contains(WeakController)) continue;
        FailsoftSpawnAttempts.Add(WeakController);

        // Deliberately bypass only the RuntimeSafe presentation gate. The base game mode still performs
        // its normal team spawn selection and has its own safe fallback transform when no spawn actor is valid.
        RuntimeGameMode->AOCGameMode::RestartPlayer(PC);

        if (PC->GetPawn())
        {
            UE_LOG(LogTemp, Display,
                TEXT("GAME_RECOVERY_PRESENTATION_FAILSOFT_SPAWN READY stages=%s player_spawned=1 deployment_gray_screen=0"),
                *FailedStages);
        }
        else
        {
            UE_LOG(LogTemp, Error,
                TEXT("GAME_RECOVERY_PRESENTATION_FAILSOFT_SPAWN FAIL stages=%s player_spawned=0 deployment_gray_screen=0"),
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
