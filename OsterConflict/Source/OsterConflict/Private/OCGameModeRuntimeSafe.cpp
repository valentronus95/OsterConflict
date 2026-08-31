#include "OCGameModeRuntimeSafe.h"

#include "OCGameInstance.h"
#include "OCGameUIRootWidget.h"
#include "OCPlayerController.h"
#include "OCPlayerState.h"
#include "OCTeamSpawnPoint.h"
#include "OCWorldSectorOster.h"

#include "Components/Widget.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "HAL/PlatformTime.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "UObject/UObjectIterator.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
    constexpr float MaxMuseumBaseDistanceCm = 4500.0f;
    constexpr float SpawnLiftCm = 80.0f;

    bool HasExplicitOption(const FString& Options, const TCHAR* Key)
    {
        FString Value = UGameplayStatics::ParseOption(Options, Key);
        Value.TrimStartAndEndInline();
        return !Value.IsEmpty();
    }

    bool IsFrontendWidgetActuallyVisible(const UOCGameUIRootWidget* Root)
    {
        if (!Root) return false;
        for (const FName Name : { FName(TEXT("R13_MenuPanel")), FName(TEXT("FrontendPanel")) })
        {
            if (const UWidget* Widget = Root->GetWidgetFromName(Name))
            {
                const ESlateVisibility Visibility = Widget->GetVisibility();
                if (Widget->GetIsEnabled() && Visibility != ESlateVisibility::Collapsed && Visibility != ESlateVisibility::Hidden)
                {
                    return true;
                }
            }
        }
        return false;
    }
}

void AOCGameModeRuntimeSafe::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);

    const bool bBotsExplicit = HasExplicitOption(Options, TEXT("Bots"));
    const bool bPopulationExplicit = HasExplicitOption(Options, TEXT("Population"));
    const bool bBotFillExplicit = HasExplicitOption(Options, TEXT("BotFill"));

    if (!bBotsExplicit && !bPopulationExplicit && !bBotFillExplicit)
    {
        TargetPopulation = 0;
        bAutoFillBots = false;
        UE_LOG(LogTemp, Display,
            TEXT("PASS44_LOCAL_BOT_AUTOFILL_DISABLED_READY implicit_population=0 explicit_bot_options=0 background_ai_load=0"));
    }
    else
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS44_EXPLICIT_BOT_OPTIONS_PRESERVED bots=%d population=%d botfill=%d target_population=%d auto_fill=%d"),
            bBotsExplicit ? 1 : 0,
            bPopulationExplicit ? 1 : 0,
            bBotFillExplicit ? 1 : 0,
            TargetPopulation,
            bAutoFillBots ? 1 : 0);
    }
}

void AOCGameModeRuntimeSafe::ShowFrontendBootstrapOverlay()
{
    if (FrontendBootstrapOverlay.IsValid()) return;
    if (!GEngine || !GEngine->GameViewport)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("PASS45_FRONTEND_BOOTSTRAP_VIEWPORT_NOT_READY retry_pending=1 black_screen_guard=armed"));
        return;
    }

    if (FrontendBootstrapStartedAtSeconds <= 0.0)
    {
        FrontendBootstrapStartedAtSeconds = FPlatformTime::Seconds();
    }
    bFrontendBootstrapDelayLogged = false;

    FrontendBootstrapOverlay =
        SNew(SBorder)
        .Padding(FMargin(64.0f))
        .BorderBackgroundColor(FLinearColor(0.006f, 0.009f, 0.012f, 1.0f))
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .FillHeight(1.0f)
            .HAlign(HAlign_Center)
            .VAlign(VAlign_Center)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Center)
                .Padding(FMargin(0.0f, 0.0f, 0.0f, 24.0f))
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(TEXT("OSTER CONFLICT")))
                    .ColorAndOpacity(FLinearColor(0.96f, 0.97f, 0.98f, 1.0f))
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Center)
                .Padding(FMargin(0.0f, 0.0f, 0.0f, 12.0f))
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(TEXT("90%")))
                    .ColorAndOpacity(FLinearColor(0.96f, 0.97f, 0.98f, 1.0f))
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Center)
                .Padding(FMargin(0.0f, 0.0f, 0.0f, 18.0f))
                [
                    SNew(SProgressBar)
                    .Percent(0.90f)
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Center)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(TEXT("ПІДГОТОВКА ГОЛОВНОГО МЕНЮ")))
                    .ColorAndOpacity(FLinearColor(0.78f, 0.81f, 0.84f, 1.0f))
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Center)
                .Padding(FMargin(0.0f, 10.0f, 0.0f, 0.0f))
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(TEXT("Екран залишається видимим, доки frontend UI фактично не готовий.")))
                    .ColorAndOpacity(FLinearColor(0.56f, 0.60f, 0.64f, 1.0f))
                ]
            ]
        ];

    GEngine->GameViewport->AddViewportWidgetContent(FrontendBootstrapOverlay.ToSharedRef(), 100000);
    UE_LOG(LogTemp, Display, TEXT("PASS45_FRONTEND_BOOTSTRAP_OVERLAY_READY percent=90 viewport_owner=1"));
}

void AOCGameModeRuntimeSafe::RemoveFrontendBootstrapOverlay(const TCHAR* Reason)
{
    if (!FrontendBootstrapOverlay.IsValid()) return;
    if (GEngine && GEngine->GameViewport)
    {
        GEngine->GameViewport->RemoveViewportWidgetContent(FrontendBootstrapOverlay.ToSharedRef());
    }
    FrontendBootstrapOverlay.Reset();
    UE_LOG(LogTemp, Display, TEXT("PASS45_FRONTEND_BOOTSTRAP_HANDOFF_READY reason=%s percent=100"),
        Reason ? Reason : TEXT("unknown"));
}

void AOCGameModeRuntimeSafe::PollFrontendBootstrapReady()
{
    UWorld* World = GetWorld();
    if (!World) return;

    bool bFrontendReady = false;
    for (TObjectIterator<UOCGameUIRootWidget> It; It; ++It)
    {
        UOCGameUIRootWidget* Root = *It;
        if (!IsValid(Root) || Root->GetWorld() != World) continue;
        if (IsFrontendWidgetActuallyVisible(Root))
        {
            bFrontendReady = true;
            break;
        }
    }

    if (bFrontendReady)
    {
        GetWorldTimerManager().ClearTimer(FrontendBootstrapPollHandle);
        RemoveFrontendBootstrapOverlay(TEXT("frontend_widget_visible"));
        return;
    }

    // GameViewport can become available a little later than GameMode BeginPlay in editor -game startup.
    // Retry creation instead of silently giving up after one early null check.
    if (!FrontendBootstrapOverlay.IsValid())
    {
        ShowFrontendBootstrapOverlay();
    }

    const double Elapsed = FrontendBootstrapStartedAtSeconds > 0.0
        ? FPlatformTime::Seconds() - FrontendBootstrapStartedAtSeconds
        : 0.0;
    if (!bFrontendBootstrapDelayLogged && Elapsed >= 5.0)
    {
        bFrontendBootstrapDelayLogged = true;
        const APlayerController* PC = World->GetFirstPlayerController();
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_FRONTEND_BOOTSTRAP_STALLED elapsed_s=%.1f player_controller=%d overlay_visible=%d black_screen_guard=armed"),
            Elapsed, PC ? 1 : 0, FrontendBootstrapOverlay.IsValid() ? 1 : 0);
    }
}

void AOCGameModeRuntimeSafe::BeginPlay()
{
    const bool bFrontendBootstrapRequired = GetNetMode() == NM_Standalone && IsFrontendOnlySession();
    if (bFrontendBootstrapRequired)
    {
        FrontendBootstrapStartedAtSeconds = FPlatformTime::Seconds();
        ShowFrontendBootstrapOverlay();
    }

    Super::BeginPlay();

    if (UOCGameInstance* GI = Cast<UOCGameInstance>(GetGameInstance()))
    {
        GI->CompleteRuntimeLoading(IsFrontendOnlySession()
            ? TEXT("frontend_beginplay_ready")
            : TEXT("runtime_beginplay_ready"));
    }

    if (bFrontendBootstrapRequired)
    {
        GetWorldTimerManager().SetTimer(
            FrontendBootstrapPollHandle,
            this,
            &AOCGameModeRuntimeSafe::PollFrontendBootstrapReady,
            0.10f,
            true,
            0.0f);
    }
}

void AOCGameModeRuntimeSafe::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (GetWorld())
    {
        GetWorldTimerManager().ClearTimer(FrontendBootstrapPollHandle);
    }
    RemoveFrontendBootstrapOverlay(TEXT("world_endplay"));
    Super::EndPlay(EndPlayReason);
}

void AOCGameModeRuntimeSafe::RestartPlayer(AController* NewPlayer)
{
    AOCPlayerController* HumanPC = Cast<AOCPlayerController>(NewPlayer);
    if (!HumanPC || HumanPC->GetRequestedDeploymentSpawn() != FName(TEXT("BASE")))
    {
        Super::RestartPlayer(NewPlayer);
        return;
    }

    const AOCPlayerState* State = HumanPC->GetPlayerState<AOCPlayerState>();
    const EOCTeam Team = State ? State->GetTeamId() : EOCTeam::None;
    if (Team == EOCTeam::None || !GetWorld())
    {
        Super::RestartPlayer(NewPlayer);
        return;
    }

    const FVector Museum = AOCWorldSectorOster::MuseumAnchor();
    const AOCTeamSpawnPoint* BestMuseumBase = nullptr;
    double BestDistanceSq = TNumericLimits<double>::Max();

    for (TActorIterator<AOCTeamSpawnPoint> It(GetWorld()); It; ++It)
    {
        const AOCTeamSpawnPoint* Point = *It;
        if (!IsValid(Point) || !Point->IsBaseSpawn() || !Point->IsAvailableForTeam(Team)) continue;

        const double DistanceSq = static_cast<double>(FVector::DistSquared2D(Point->GetActorLocation(), Museum));
        if (!BestMuseumBase || DistanceSq < BestDistanceSq)
        {
            BestMuseumBase = Point;
            BestDistanceSq = DistanceSq;
        }
    }

    FTransform SpawnTransform;
    const TCHAR* SpawnSource = TEXT("museum_base_actor");
    if (BestMuseumBase)
    {
        SpawnTransform = BestMuseumBase->GetActorTransform();
        SpawnTransform.AddToTranslation(FVector(0.0f, 0.0f, SpawnLiftCm));
    }
    else
    {
        const float Side = Team == EOCTeam::TeamTwo ? 1.0f : -1.0f;
        const FVector FallbackLocation = Museum + FVector(1400.0f * Side, -2400.0f, 200.0f);
        const FRotator FallbackRotation = (Museum - FallbackLocation).Rotation();
        SpawnTransform = FTransform(FallbackRotation, FallbackLocation);
        SpawnSource = TEXT("museum_anchor_failsafe");
        UE_LOG(LogTemp, Warning,
            TEXT("PASS44_MUSEUM_BASE_ACTOR_MISSING team=%d using_anchor_failsafe=1"),
            static_cast<int32>(Team));
    }

    RestartPlayerAtTransform(NewPlayer, SpawnTransform);

    APawn* SpawnedPawn = NewPlayer ? NewPlayer->GetPawn() : nullptr;
    if (!IsValid(SpawnedPawn))
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS44_ACTUAL_PAWN_MUSEUM_BASE_FAIL reason=no_pawn team=%d source=%s"),
            static_cast<int32>(Team), SpawnSource);
        return;
    }

    float ActualDistanceCm = FVector::Dist2D(SpawnedPawn->GetActorLocation(), Museum);
    if (ActualDistanceCm > MaxMuseumBaseDistanceCm)
    {
        SpawnedPawn->SetActorLocation(
            SpawnTransform.GetLocation(),
            false,
            nullptr,
            ETeleportType::TeleportPhysics);
        SpawnedPawn->SetActorRotation(SpawnTransform.Rotator(), ETeleportType::TeleportPhysics);
        ActualDistanceCm = FVector::Dist2D(SpawnedPawn->GetActorLocation(), Museum);
        UE_LOG(LogTemp, Warning,
            TEXT("PASS44_ACTUAL_PAWN_MUSEUM_BASE_CORRECTED team=%d distance_m=%.1f max_m=45 source=%s"),
            static_cast<int32>(Team), ActualDistanceCm / 100.0f, SpawnSource);
    }

    if (ActualDistanceCm <= MaxMuseumBaseDistanceCm)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS44_ACTUAL_PAWN_MUSEUM_BASE_READY team=%d requested=BASE distance_m=%.1f max_m=45 source=%s pawn=(%.0f,%.0f,%.0f) museum=(%.0f,%.0f,%.0f)"),
            static_cast<int32>(Team),
            ActualDistanceCm / 100.0f,
            SpawnSource,
            SpawnedPawn->GetActorLocation().X,
            SpawnedPawn->GetActorLocation().Y,
            SpawnedPawn->GetActorLocation().Z,
            Museum.X,
            Museum.Y,
            Museum.Z);
    }
    else
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS44_ACTUAL_PAWN_MUSEUM_BASE_FAIL reason=distance team=%d distance_m=%.1f max_m=45 source=%s"),
            static_cast<int32>(Team), ActualDistanceCm / 100.0f, SpawnSource);
    }
}
