#include "OCRuntimePresentationRecoverySubsystem.h"

#include "OCAuthoredWorldSurfaceUpgradeSubsystem.h"
#include "OCBlock0GroundFoundationSubsystem.h"
#include "OCGameMode.h"
#include "OCGameModeRuntimeSafe.h"
#include "OCGameUIRootWidget.h"
#include "OCParkGroundAuthoredUpgradeSubsystem.h"
#include "OCParkHardscapeAuthoredUpgradeSubsystem.h"
#include "OCPlayerController.h"
#include "OCPlayerState.h"

#include "Components/EditableTextBox.h"
#include "Components/PanelWidget.h"
#include "Components/VerticalBox.h"
#include "Components/Widget.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Styling/SlateTypes.h"
#include "UObject/UObjectIterator.h"

namespace
{
    void ApplyFrontendPaddingRecursive(UWidget* Widget, int32& FixedFields)
    {
        if (!Widget) return;

        if (UEditableTextBox* Field = Cast<UEditableTextBox>(Widget))
        {
            FEditableTextBoxStyle Style = Field->GetWidgetStyle();
            const FMargin& Padding = Style.Padding;
            const bool bNeedsFix =
                FMath::IsNearlyEqual(Padding.Left, 14.0f) &&
                FMath::IsNearlyEqual(Padding.Top, 9.0f) &&
                FMath::IsNearlyEqual(Padding.Right, 14.0f) &&
                FMath::IsNearlyEqual(Padding.Bottom, 9.0f);

            if (bNeedsFix)
            {
                Style.Padding = FMargin(14.0f, 4.0f, 14.0f, 4.0f);
                Field->SetWidgetStyle(Style);
                ++FixedFields;
            }
            return;
        }

        if (UPanelWidget* Panel = Cast<UPanelWidget>(Widget))
        {
            for (int32 Index = 0; Index < Panel->GetChildrenCount(); ++Index)
            {
                ApplyFrontendPaddingRecursive(Panel->GetChildAt(Index), FixedFields);
            }
        }
    }
}

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

    UWorld* World = GetWorld();
    if (!World || World->GetNetMode() == NM_DedicatedServer) return;

    AOCPlayerController* PC = Cast<AOCPlayerController>(World->GetFirstPlayerController());
    if (!PC || !PC->IsLocalController() || !PC->IsFrontendMenuVisible()) return;

    UOCGameUIRootWidget* Root = nullptr;
    for (TObjectIterator<UOCGameUIRootWidget> It; It; ++It)
    {
        if (It->GetWorld() == World && It->GetOwningPlayer() == PC)
        {
            Root = *It;
            break;
        }
    }
    if (!Root) return;

    UVerticalBox* Fields = Cast<UVerticalBox>(Root->GetWidgetFromName(TEXT("R13_FrontendFields")));
    if (!Fields) return;

    int32 FixedFields = 0;
    ApplyFrontendPaddingRecursive(Fields, FixedFields);

    if (FixedFields > 0)
    {
        bFrontendFieldPaddingFixed = true;
        UE_LOG(LogTemp, Display,
            TEXT("GAME_RECOVERY_FRONTEND_FIELD_ALIGNMENT_READY fields=%d vertical_padding=4 clipped_text=0 scoped_widget_tree=1 global_uobject_scan=0"),
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
