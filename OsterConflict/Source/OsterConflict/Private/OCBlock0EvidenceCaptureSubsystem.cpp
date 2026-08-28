#include "OCBlock0EvidenceCaptureSubsystem.h"

#include "OCWorldSectorOster.h"

#include "Camera/CameraActor.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformMisc.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "TimerManager.h"
#include "UnrealClient.h"

namespace
{
    constexpr float InitialWorldSettleSeconds = 35.0f;
    constexpr float CameraSettleSeconds = 1.5f;
    constexpr float ScreenshotFlushSeconds = 1.25f;
    constexpr float FinalFlushSeconds = 2.0f;

    FString EvidenceOutputDirectory()
    {
        return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Screenshots"), TEXT("Pass45Block0"));
    }
}

void UOCBlock0EvidenceCaptureSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    if (!FParse::Param(FCommandLine::Get(), TEXT("Pass45Block0Evidence")))
    {
        return;
    }

    if (!InWorld.IsGameWorld() || InWorld.GetNetMode() == NM_DedicatedServer ||
        !InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime")))
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_BLOCK0_EVIDENCE_CAMERA_FAIL reason=invalid_world_or_map runtime_acceptance=0"));
        return;
    }

    bEvidenceMode = true;
    Views.Reset(5);

    const FVector Museum = AOCWorldSectorOster::MuseumAnchor();
    const FVector Park = AOCWorldSectorOster::ParkAnchor();
    const FVector College = AOCWorldSectorOster::CollegeAnchor();
    const FVector Roadside = AOCWorldSectorOster::KrushelnytskaEnterableHouseAnchor();

    Views.Add({
        TEXT("01_museum_central_ground"),
        Museum + FVector(-4200.0f, -2800.0f, 240.0f),
        Museum + FVector(0.0f, 0.0f, 90.0f)
    });
    Views.Add({
        TEXT("02_central_park_ground"),
        Park + FVector(-4500.0f, -3000.0f, 300.0f),
        Park + FVector(500.0f, 450.0f, 80.0f)
    });
    Views.Add({
        TEXT("03_college_urban_lawn"),
        College + FVector(-3800.0f, 2600.0f, 250.0f),
        College + FVector(300.0f, -250.0f, 85.0f)
    });
    Views.Add({
        TEXT("04_roadside_private_sector"),
        Roadside + FVector(-3200.0f, -2200.0f, 220.0f),
        Roadside + FVector(900.0f, 300.0f, 70.0f)
    });
    Views.Add({
        TEXT("05_long_sightline_lod"),
        FVector(-70000.0f, 4000.0f, 380.0f),
        FVector(12000.0f, 76000.0f, 90.0f)
    });

    CurrentViewIndex = 0;
    IFileManager::Get().MakeDirectory(*EvidenceOutputDirectory(), true);

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_BLOCK0_EVIDENCE_CAMERA_ARMED views=5 initial_settle_seconds=35 exact_required_views=1 auto_exit=1 runtime_acceptance=0"));

    InWorld.GetTimerManager().SetTimer(
        StartupDelayHandle,
        this,
        &UOCBlock0EvidenceCaptureSubsystem::BeginCaptureSequence,
        InitialWorldSettleSeconds,
        false);
}

void UOCBlock0EvidenceCaptureSubsystem::BeginCaptureSequence()
{
    if (!bEvidenceMode)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    EvidencePlayerController = UGameplayStatics::GetPlayerController(World, 0);
    if (!EvidencePlayerController)
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_BLOCK0_EVIDENCE_CAMERA_FAIL reason=player_controller_unavailable runtime_acceptance=0"));
        FGenericPlatformMisc::RequestExit(false);
        return;
    }

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    EvidenceCamera = World->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Params);
    if (!EvidenceCamera)
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_BLOCK0_EVIDENCE_CAMERA_FAIL reason=camera_spawn_failed runtime_acceptance=0"));
        FGenericPlatformMisc::RequestExit(false);
        return;
    }

    EvidenceCamera->SetActorEnableCollision(false);
    EvidencePlayerController->SetViewTarget(EvidenceCamera);
    PositionCurrentView();
}

void UOCBlock0EvidenceCaptureSubsystem::PositionCurrentView()
{
    if (!bEvidenceMode || !EvidenceCamera || !Views.IsValidIndex(CurrentViewIndex))
    {
        FinishCaptureSequence();
        return;
    }

    const FEvidenceView& View = Views[CurrentViewIndex];
    const FVector LookVector = View.LookAtLocation - View.CameraLocation;
    EvidenceCamera->SetActorLocationAndRotation(View.CameraLocation, LookVector.Rotation(), false, nullptr, ETeleportType::TeleportPhysics);

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_BLOCK0_EVIDENCE_CAMERA_VIEW index=%d/%d name=%s location=%s target=%s runtime_acceptance=0"),
        CurrentViewIndex + 1,
        Views.Num(),
        *View.FileStem,
        *View.CameraLocation.ToCompactString(),
        *View.LookAtLocation.ToCompactString());

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            SettleDelayHandle,
            this,
            &UOCBlock0EvidenceCaptureSubsystem::CaptureCurrentView,
            CameraSettleSeconds,
            false);
    }
}

void UOCBlock0EvidenceCaptureSubsystem::CaptureCurrentView()
{
    if (!bEvidenceMode || !Views.IsValidIndex(CurrentViewIndex))
    {
        FinishCaptureSequence();
        return;
    }

    const FEvidenceView& View = Views[CurrentViewIndex];
    const FString Filename = FPaths::Combine(EvidenceOutputDirectory(), View.FileStem + TEXT(".png"));
    FScreenshotRequest::RequestScreenshot(Filename, false, false);

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_BLOCK0_EVIDENCE_CAPTURE index=%d/%d name=%s file=%s runtime_acceptance=0"),
        CurrentViewIndex + 1,
        Views.Num(),
        *View.FileStem,
        *Filename);

    ++CurrentViewIndex;
    if (UWorld* World = GetWorld())
    {
        if (Views.IsValidIndex(CurrentViewIndex))
        {
            World->GetTimerManager().SetTimer(
                SettleDelayHandle,
                this,
                &UOCBlock0EvidenceCaptureSubsystem::PositionCurrentView,
                ScreenshotFlushSeconds,
                false);
        }
        else
        {
            World->GetTimerManager().SetTimer(
                FinishDelayHandle,
                this,
                &UOCBlock0EvidenceCaptureSubsystem::FinishCaptureSequence,
                FinalFlushSeconds,
                false);
        }
    }
}

void UOCBlock0EvidenceCaptureSubsystem::FinishCaptureSequence()
{
    if (!bEvidenceMode)
    {
        return;
    }

    bEvidenceMode = false;
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_BLOCK0_EVIDENCE_CAPTURE_COMPLETE screenshots=5 exact_required_views=1 auto_exit=1 visual_review_required=1 runtime_acceptance=0"));
    FGenericPlatformMisc::RequestExit(false);
}
