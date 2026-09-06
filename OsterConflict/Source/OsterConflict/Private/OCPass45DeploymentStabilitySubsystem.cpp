#include "OCPass45DeploymentStabilitySubsystem.h"

#include "OCPlayerController.h"
#include "OCR137MuseumPhotoModelSubsystem.h"

#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/StreamableManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Styling/CoreStyle.h"
#include "TimerManager.h"
#include "UObject/SoftObjectPath.h"
#include "Widgets/Layout/SBorder.h"

namespace
{
    void BuildMuseumPreloadPaths(TArray<FSoftObjectPath>& OutAssets)
    {
        OutAssets.Reset();
        OutAssets.Reserve(15);
        OutAssets.Add(FSoftObjectPath(TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Wall_8m.Wall_8m")));
        OutAssets.Add(FSoftObjectPath(TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Wall_Window_4m.Wall_Window_4m")));
        OutAssets.Add(FSoftObjectPath(TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Wall_Door_Windows_8m.Wall_Door_Windows_8m")));
        OutAssets.Add(FSoftObjectPath(TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Wall_Top_4m.Wall_Top_4m")));
        OutAssets.Add(FSoftObjectPath(TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Roof_Both_Ends_4m.Roof_Both_Ends_4m")));
        OutAssets.Add(FSoftObjectPath(TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Bottom_Extender_4m.Bottom_Extender_4m")));
        OutAssets.Add(FSoftObjectPath(TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Porch_4x4m.Porch_4x4m")));
        OutAssets.Add(FSoftObjectPath(TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Window_Frame_Part.Window_Frame_Part")));
        OutAssets.Add(FSoftObjectPath(TEXT("/Game/Modular_Rural_Cabin/Materials/Instances/Metal_Roof.Metal_Roof")));
        OutAssets.Add(FSoftObjectPath(TEXT("/Game/Modular_Rural_Cabin/Materials/Instances/Wood_Planks_Painted_Blue.Wood_Planks_Painted_Blue")));
        OutAssets.Add(FSoftObjectPath(TEXT("/Game/KiteDemo/LevelContent/Architecture/SM_1Meter_01.SM_1Meter_01")));
        OutAssets.Add(FSoftObjectPath(TEXT("/Game/Mega_Street_Props_Pack/Street_Props_pack_V2/Materials/Instances/M_Concrete_2_Inst.M_Concrete_2_Inst")));
        OutAssets.Add(FSoftObjectPath(TEXT("/Game/KiteDemo/Environments/Trees/ScotsPineTall_01/ScotsPineTall_01.ScotsPineTall_01")));
        OutAssets.Add(FSoftObjectPath(TEXT("/Game/KiteDemo/Environments/Trees/HillTree_02/HillTree_02.HillTree_02")));
        OutAssets.Add(FSoftObjectPath(TEXT("/Game/KiteDemo/Environments/Trees/Vegetation_Debris_002/SM_Vegetation_Debris_002.SM_Vegetation_Debris_002")));
    }

    bool AreMuseumPreloadAssetsResolved(FString& OutMissingAsset)
    {
        TArray<FSoftObjectPath> MuseumAssets;
        BuildMuseumPreloadPaths(MuseumAssets);
        for (const FSoftObjectPath& Path : MuseumAssets)
        {
            if (!Path.ResolveObject())
            {
                OutMissingAsset = Path.ToString();
                return false;
            }
        }
        OutMissingAsset.Reset();
        return true;
    }
}

bool UOCPass45DeploymentStabilitySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && World->GetNetMode() != NM_DedicatedServer &&
        (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCPass45DeploymentStabilitySubsystem::Tick(float DeltaTime)
{
    (void)DeltaTime;

    UWorld* World = GetWorld();
    if (!World || !World->IsGameWorld()) return;
    if (!World->GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    AOCPlayerController* PC = Cast<AOCPlayerController>(World->GetFirstPlayerController());
    const bool bLocalPlayer = PC && PC->IsLocalController();
    const bool bFrontendVisible = bLocalPlayer && PC->IsFrontendMenuVisible();
    const bool bSettingsVisible = bLocalPlayer && PC->IsSettingsVisible();
    const bool bDeploymentFlagVisible = bLocalPlayer && PC->IsDeploymentPanelVisible();
    const bool bDeploymentVisible = bLocalPlayer && !bFrontendVisible && !bSettingsVisible &&
        (bDeploymentFlagVisible || PC->GetPawn() == nullptr);
    const bool bBlockingMenuVisible = bLocalPlayer &&
        (bFrontendVisible || bSettingsVisible || bDeploymentFlagVisible || PC->GetPawn() == nullptr);
    const bool bWorldPreparationWindow = bDeploymentVisible && !bFrontendVisible && !bSettingsVisible;

    // Keep gameplay timers paused while a blocking UI owns the screen. Both this subsystem and the
    // landmark coordinator tick while paused, so preparation can advance without allowing old delayed
    // world owners to race the menu or block the native Windows message pump.
    ApplyMenuPause(*World, bBlockingMenuVisible);

    if (!bMuseumPreparationStarted)
    {
        SuppressSynchronousMuseumStartup(*World);
        if (bWorldPreparationWindow)
        {
            BeginMuseumBuildPreparation(*World);
        }
    }

    if (bDeploymentVisible)
    {
        EnsureDeploymentBackdrop();
    }
    else
    {
        RemoveDeploymentBackdrop();
    }
}

void UOCPass45DeploymentStabilitySubsystem::ApplyMenuPause(UWorld& World, const bool bShouldPause)
{
    const bool bPaused = UGameplayStatics::IsGamePaused(&World);

    if (bShouldPause)
    {
        if (!bPaused)
        {
            if (UGameplayStatics::SetGamePaused(&World, true))
            {
                bMenuPauseOwned = true;
                UE_LOG(LogTemp, Display,
                    TEXT("PASS45_MENU_WORLD_PAUSED blocking_ui=1 world_timers=0 slate_responsive=1 native_window_responsive=1"));
            }
        }
        return;
    }

    if (bMenuPauseOwned && bPaused)
    {
        if (UGameplayStatics::SetGamePaused(&World, false))
        {
            bMenuPauseOwned = false;
            UE_LOG(LogTemp, Display,
                TEXT("PASS45_MENU_WORLD_RESUMED blocking_ui=0 gameplay_ready=1"));
        }
    }
    else if (!bPaused)
    {
        bMenuPauseOwned = false;
    }
}

void UOCPass45DeploymentStabilitySubsystem::SuppressSynchronousMuseumStartup(UWorld& World)
{
    UOCR137MuseumPhotoModelSubsystem* MuseumSubsystem =
        World.GetSubsystem<UOCR137MuseumPhotoModelSubsystem>();
    if (!MuseumSubsystem) return;

    World.GetTimerManager().ClearAllTimersForObject(MuseumSubsystem);

    if (!bMuseumSuppressionLogged)
    {
        bMuseumSuppressionLogged = true;
        UE_LOG(LogTemp, Display,
            TEXT("GAME_RECOVERY_MUSEUM_SYNC_STARTUP_SUPPRESSED pre_spawn=1 synchronous_package_loads=0"));
    }
}

void UOCPass45DeploymentStabilitySubsystem::BeginMuseumBuildPreparation(UWorld& World)
{
    UOCR137MuseumPhotoModelSubsystem* MuseumSubsystem =
        World.GetSubsystem<UOCR137MuseumPhotoModelSubsystem>();
    if (!MuseumSubsystem) return;

    World.GetTimerManager().ClearAllTimersForObject(MuseumSubsystem);
    bMuseumPreparationStarted = true;

    TArray<FSoftObjectPath> MuseumAssets;
    BuildMuseumPreloadPaths(MuseumAssets);

    MuseumPreloadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
        MuseumAssets,
        FStreamableDelegate::CreateUObject(this,
            &UOCPass45DeploymentStabilitySubsystem::CompleteMuseumBuildAfterAsyncLoad));

    UE_LOG(LogTemp, Display,
        TEXT("GAME_RECOVERY_MUSEUM_ASYNC_PRELOAD_STARTED assets=%d pre_spawn=1 deployment_visible=1"), MuseumAssets.Num());

    if (!MuseumPreloadHandle.IsValid())
    {
        UE_LOG(LogTemp, Error,
            TEXT("GAME_RECOVERY_MUSEUM_ASYNC_PRELOAD_FAIL reason=invalid_handle sync_fallback=0 build_started=0 runtime_acceptance=0"));
    }
}

void UOCPass45DeploymentStabilitySubsystem::CompleteMuseumBuildAfterAsyncLoad()
{
    UWorld* World = GetWorld();
    if (!World || !World->IsGameWorld())
    {
        MuseumPreloadHandle.Reset();
        return;
    }

    if (!MuseumPreloadHandle.IsValid() || !MuseumPreloadHandle->HasLoadCompleted())
    {
        UE_LOG(LogTemp, Error,
            TEXT("GAME_RECOVERY_MUSEUM_ASYNC_PRELOAD_FAIL reason=handle_not_complete sync_fallback=0 build_started=0 runtime_acceptance=0"));
        MuseumPreloadHandle.Reset();
        return;
    }

    FString MissingAsset;
    if (!AreMuseumPreloadAssetsResolved(MissingAsset))
    {
        UE_LOG(LogTemp, Error,
            TEXT("GAME_RECOVERY_MUSEUM_ASYNC_PRELOAD_FAIL reason=asset_not_resolved asset=%s sync_fallback=0 build_started=0 runtime_acceptance=0"),
            *MissingAsset);
        MuseumPreloadHandle.Reset();
        return;
    }

    if (UOCR137MuseumPhotoModelSubsystem* MuseumSubsystem =
        World->GetSubsystem<UOCR137MuseumPhotoModelSubsystem>())
    {
        UE_LOG(LogTemp, Display,
            TEXT("GAME_RECOVERY_MUSEUM_ASYNC_PRELOAD_READY pre_spawn=1 synchronous_disk_load=0 resolved_assets=15"));
        MuseumSubsystem->RunAuthoritativeBuildNow(*World);
        bMuseumBuildComplete = true;
        UE_LOG(LogTemp, Display,
            TEXT("GAME_RECOVERY_MUSEUM_BUILD_READY pre_spawn=1"));
    }

    MuseumPreloadHandle.Reset();
}

void UOCPass45DeploymentStabilitySubsystem::EnsureDeploymentBackdrop()
{
    if (DeploymentBackdrop.IsValid()) return;
    if (!GEngine || !GEngine->GameViewport) return;

    DeploymentBackdrop =
        SNew(SBorder)
        .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
        .BorderBackgroundColor(FLinearColor(0.018f, 0.024f, 0.030f, 1.0f))
        .Padding(0.0f)
        .Visibility(EVisibility::HitTestInvisible);

    GEngine->GameViewport->AddViewportWidgetContent(DeploymentBackdrop.ToSharedRef(), 490);
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_DEPLOYMENT_OPAQUE_BACKDROP_READY alpha=1 viewport_z=490 world_bleedthrough=0 hit_test_blocking=0 no_pawn_fallback=1"));
}

void UOCPass45DeploymentStabilitySubsystem::RemoveDeploymentBackdrop()
{
    if (!DeploymentBackdrop.IsValid()) return;

    if (GEngine && GEngine->GameViewport)
    {
        GEngine->GameViewport->RemoveViewportWidgetContent(DeploymentBackdrop.ToSharedRef());
    }
    DeploymentBackdrop.Reset();
}

void UOCPass45DeploymentStabilitySubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        if (bMenuPauseOwned && UGameplayStatics::IsGamePaused(World))
        {
            UGameplayStatics::SetGamePaused(World, false);
        }
    }
    bMenuPauseOwned = false;
    bMuseumPreparationStarted = false;
    bMuseumBuildComplete = false;

    if (MuseumPreloadHandle.IsValid())
    {
        MuseumPreloadHandle->CancelHandle();
        MuseumPreloadHandle.Reset();
    }
    RemoveDeploymentBackdrop();
    Super::Deinitialize();
}

TStatId UOCPass45DeploymentStabilitySubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCPass45DeploymentStabilitySubsystem, STATGROUP_Tickables);
}
