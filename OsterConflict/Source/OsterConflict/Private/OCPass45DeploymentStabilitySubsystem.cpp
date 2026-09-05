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
#include "Widgets/Layout/SBorder.h"

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
    const bool bGameplayReady = bLocalPlayer && !bBlockingMenuVisible && PC->GetPawn() != nullptr;

    // The actual freeze was not a Windows title-bar problem. World timers and delayed content owners
    // were still allowed to run while Slate was serving the frontend/deployment flow. Pause the world
    // while a blocking UI owns the screen, but keep this subsystem and the deployment-flow subsystem
    // tickable while paused. Slate, Alt+Tab and the native minimize/maximize/close buttons then keep
    // receiving messages while gameplay timers remain suspended.
    ApplyMenuPause(*World, bBlockingMenuVisible);

    // The museum owner used to schedule a one-shot synchronous package-loading rebuild at +0.75 s.
    // A one-time clear had an OnWorldBeginPlay ordering race, so keep suppressing that timer for
    // the whole pre-game/menu phase. This protects both Slate input and the native Windows message pump.
    if (!bMuseumBuildReleased)
    {
        if (!bGameplayReady)
        {
            SuppressSynchronousMuseumStartup(*World);
        }
        else
        {
            ReleaseMuseumBuildToGameplay(*World);
        }
    }

    // Deployment-state detection deliberately has a no-pawn fallback. The R13 flow can be visible
    // for a frame before the old controller deployment flag settles; without this fallback the world
    // could bleed through even though the deployment widget itself was already on screen.
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

    // Repeat while menus are active. This closes the startup-order race instead of assuming a
    // single ClearAllTimersForObject happened after the museum subsystem scheduled itself.
    World.GetTimerManager().ClearAllTimersForObject(MuseumSubsystem);

    if (!bMuseumSuppressionLogged)
    {
        bMuseumSuppressionLogged = true;
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_DEPLOYMENT_MUSEUM_SYNC_LOAD_SUPPRESSED menu_phase=1 repeated_guard=1 synchronous_package_loads_during_deployment=0"));
    }
}

void UOCPass45DeploymentStabilitySubsystem::ReleaseMuseumBuildToGameplay(UWorld& World)
{
    UOCR137MuseumPhotoModelSubsystem* MuseumSubsystem =
        World.GetSubsystem<UOCR137MuseumPhotoModelSubsystem>();
    if (!MuseumSubsystem) return;

    // Kill any legacy one-shot timer one final time. From here on the authoritative museum build
    // is owned by this guarded path and its packages are loaded asynchronously first.
    World.GetTimerManager().ClearAllTimersForObject(MuseumSubsystem);
    bMuseumBuildReleased = true;

    TArray<FSoftObjectPath> MuseumAssets;
    MuseumAssets.Reserve(15);
    MuseumAssets.Add(FSoftObjectPath(TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Wall_8m.Wall_8m")));
    MuseumAssets.Add(FSoftObjectPath(TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Wall_Window_4m.Wall_Window_4m")));
    MuseumAssets.Add(FSoftObjectPath(TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Wall_Door_Windows_8m.Wall_Door_Windows_8m")));
    MuseumAssets.Add(FSoftObjectPath(TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Wall_Top_4m.Wall_Top_4m")));
    MuseumAssets.Add(FSoftObjectPath(TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Roof_Both_Ends_4m.Roof_Both_Ends_4m")));
    MuseumAssets.Add(FSoftObjectPath(TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Bottom_Extender_4m.Bottom_Extender_4m")));
    MuseumAssets.Add(FSoftObjectPath(TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Porch_4x4m.Porch_4x4m")));
    MuseumAssets.Add(FSoftObjectPath(TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Window_Frame_Part.Window_Frame_Part")));
    MuseumAssets.Add(FSoftObjectPath(TEXT("/Game/Modular_Rural_Cabin/Materials/Instances/Metal_Roof.Metal_Roof")));
    MuseumAssets.Add(FSoftObjectPath(TEXT("/Game/Modular_Rural_Cabin/Materials/Instances/Wood_Planks_Painted_Blue.Wood_Planks_Painted_Blue")));
    MuseumAssets.Add(FSoftObjectPath(TEXT("/Game/KiteDemo/LevelContent/Architecture/SM_1Meter_01.SM_1Meter_01")));
    MuseumAssets.Add(FSoftObjectPath(TEXT("/Game/Mega_Street_Props_Pack/Street_Props_pack_V2/Materials/Instances/M_Concrete_2_Inst.M_Concrete_2_Inst")));
    MuseumAssets.Add(FSoftObjectPath(TEXT("/Game/KiteDemo/Environments/Trees/ScotsPineTall_01/ScotsPineTall_01.ScotsPineTall_01")));
    MuseumAssets.Add(FSoftObjectPath(TEXT("/Game/KiteDemo/Environments/Trees/HillTree_02/HillTree_02.HillTree_02")));
    MuseumAssets.Add(FSoftObjectPath(TEXT("/Game/KiteDemo/Environments/Trees/Vegetation_Debris_002/SM_Vegetation_Debris_002.SM_Vegetation_Debris_002")));

    MuseumPreloadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
        MuseumAssets,
        FStreamableDelegate::CreateUObject(this,
            &UOCPass45DeploymentStabilitySubsystem::CompleteMuseumBuildAfterAsyncLoad));

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_MUSEUM_ASYNC_PRELOAD_STARTED assets=%d menu_phase=0 gameplay_pawn=1"), MuseumAssets.Num());

    // RequestAsyncLoad may return no handle when everything is already resident. In that case
    // execute the build immediately; LoadObject calls inside it are cache hits rather than disk loads.
    if (!MuseumPreloadHandle.IsValid())
    {
        CompleteMuseumBuildAfterAsyncLoad();
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

    if (UOCR137MuseumPhotoModelSubsystem* MuseumSubsystem =
        World->GetSubsystem<UOCR137MuseumPhotoModelSubsystem>())
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_MUSEUM_ASYNC_PRELOAD_READY menu_phase=0 synchronous_startup_load=0"));
        MuseumSubsystem->RunAuthoritativeBuildNow(*World);
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
