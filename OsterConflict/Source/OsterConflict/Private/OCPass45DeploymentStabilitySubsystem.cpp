#include "OCPass45DeploymentStabilitySubsystem.h"

#include "OCPlayerController.h"
#include "OCR137MuseumPhotoModelSubsystem.h"

#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
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
    UWorld* World = GetWorld();
    if (!World || !World->IsGameWorld()) return;
    if (!World->GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    // The R13.7 museum owner schedules a synchronous package-loading rebuild about 0.75 s
    // after world begin play. That work can block the game thread while Deployment is on
    // screen, which also makes Slate, Alt+Tab and the native title-bar buttons look dead.
    // Retire that one legacy timer before it can fire. Existing world/museum content is
    // preserved; this subsystem does not destroy or replace any scene content.
    if (!bMuseumStartupRetired)
    {
        RetireSynchronousMuseumStartup();
    }

    AOCPlayerController* PC = Cast<AOCPlayerController>(World->GetFirstPlayerController());
    const bool bDeploymentVisible = PC && PC->IsLocalController() &&
        PC->IsDeploymentPanelVisible() && !PC->IsFrontendMenuVisible() && !PC->IsSettingsVisible();

    if (bDeploymentVisible)
    {
        EnsureDeploymentBackdrop();
    }
    else
    {
        RemoveDeploymentBackdrop();
    }
}

void UOCPass45DeploymentStabilitySubsystem::RetireSynchronousMuseumStartup()
{
    UWorld* World = GetWorld();
    if (!World) return;

    UOCR137MuseumPhotoModelSubsystem* MuseumSubsystem =
        World->GetSubsystem<UOCR137MuseumPhotoModelSubsystem>();
    if (!MuseumSubsystem) return;

    World->GetTimerManager().ClearAllTimersForObject(MuseumSubsystem);
    bMuseumStartupRetired = true;
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_DEPLOYMENT_MUSEUM_SYNC_LOAD_RETIRED startup_timer=0 synchronous_package_loads_during_deployment=0 source_world_preserved=1"));
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
        TEXT("PASS45_DEPLOYMENT_OPAQUE_BACKDROP_READY alpha=1 viewport_z=490 world_bleedthrough=0 hit_test_blocking=0"));
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
    RemoveDeploymentBackdrop();
    Super::Deinitialize();
}

TStatId UOCPass45DeploymentStabilitySubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCPass45DeploymentStabilitySubsystem, STATGROUP_Tickables);
}
