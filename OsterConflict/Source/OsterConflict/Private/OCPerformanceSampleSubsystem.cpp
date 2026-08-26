#include "OCPerformanceSampleSubsystem.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformMemory.h"
#include "Misc/CommandLine.h"
#include "NavigationSystem.h"

namespace
{
    const TCHAR* Pass45RhiMode()
    {
        const FString CommandLine(FCommandLine::Get());
        return CommandLine.Contains(TEXT("-norhithread"), ESearchCase::IgnoreCase)
            ? TEXT("dx11_sm5_no_rhi_thread_compat")
            : TEXT("dx11_sm5_rhi_thread");
    }

    void LogPass18WorldDiagnostics(UWorld* World, APlayerController* PC)
    {
        if (!World || !PC || !PC->GetPawn()) return;

        int32 ActorCount = 0;
        int32 ISMComponentCount = 0;
        int32 ISMInstanceCount = 0;
        int32 VisibleISMInstanceCount = 0;
        int32 CollidingISMInstanceCount = 0;

        for (TActorIterator<AActor> It(World); It; ++It)
        {
            AActor* Actor = *It;
            if (!Actor) continue;
            ++ActorCount;

            TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
            Actor->GetComponents(Components);
            for (UInstancedStaticMeshComponent* Component : Components)
            {
                if (!Component) continue;

                const int32 InstanceCount = Component->GetInstanceCount();
                ++ISMComponentCount;
                ISMInstanceCount += InstanceCount;
                if (Component->IsVisible())
                {
                    VisibleISMInstanceCount += InstanceCount;
                }
                if (Component->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
                {
                    CollidingISMInstanceCount += InstanceCount;
                }
            }
        }

        const FPlatformMemoryStats Memory = FPlatformMemory::GetStats();
        constexpr double BytesPerMiB = 1024.0 * 1024.0;
        const double UsedPhysicalMiB = static_cast<double>(Memory.UsedPhysical) / BytesPerMiB;
        const double AvailablePhysicalMiB = static_cast<double>(Memory.AvailablePhysical) / BytesPerMiB;
        const bool bNavigationSystemPresent = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World) != nullptr;
        const FVector PlayerLocation = PC->GetPawn()->GetActorLocation();

        UE_LOG(LogTemp, Display,
            TEXT("PASS18_WORLD_PERF_DIAGNOSTICS actors=%d ism_components=%d ism_instances=%d visible_ism_instances=%d colliding_ism_instances=%d nav_system=%d ram_used_mib=%.0f ram_available_mib=%.0f player_x=%.0f player_y=%.0f player_z=%.0f"),
            ActorCount,
            ISMComponentCount,
            ISMInstanceCount,
            VisibleISMInstanceCount,
            CollidingISMInstanceCount,
            bNavigationSystemPresent ? 1 : 0,
            UsedPhysicalMiB,
            AvailablePhysicalMiB,
            PlayerLocation.X,
            PlayerLocation.Y,
            PlayerLocation.Z);
    }
}

bool UOCPerformanceSampleSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCPerformanceSampleSubsystem::ValidatePass45RecoveryRuntimeContract()
{
    if (bRecoveryRuntimeContractLogged) return;
    bRecoveryRuntimeContractLogged = true;

    const FString CommandLine(FCommandLine::Get());
    const bool bFullscreenRequested = CommandLine.Contains(TEXT("-fullscreen"), ESearchCase::IgnoreCase);

    IConsoleVariable* MaxFpsVariable = IConsoleManager::Get().FindConsoleVariable(TEXT("t.MaxFPS"));
    const float RuntimeMaxFps = MaxFpsVariable ? MaxFpsVariable->GetFloat() : -1.0f;
    const bool bThermalCapReady = MaxFpsVariable && FMath::IsNearlyEqual(RuntimeMaxFps, 60.0f, 0.5f);

    const UGameViewportClient* ViewportClient = GEngine ? GEngine->GameViewport : nullptr;
    const bool bViewportReady = ViewportClient != nullptr;
    const bool bFullscreenRuntime = ViewportClient && ViewportClient->IsFullScreenViewport();
    const bool bDisplayReady = bFullscreenRequested && bViewportReady && bFullscreenRuntime;

    if (bThermalCapReady)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_THERMAL_CAP_RUNTIME_READY requested_fps=60 actual_tmaxfps=%.1f tolerance=0.5 quality_mutation=0 render_scale_mutation=0"),
            RuntimeMaxFps);
    }
    else
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_THERMAL_CAP_RUNTIME_FAIL requested_fps=60 actual_tmaxfps=%.1f cvar_present=%d quality_mutation=0 render_scale_mutation=0"),
            RuntimeMaxFps, MaxFpsVariable ? 1 : 0);
    }

    if (bDisplayReady)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_FULLSCREEN_RUNTIME_READY requested_fullscreen=1 viewport_present=1 viewport_fullscreen=1"));
    }
    else
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_FULLSCREEN_RUNTIME_FAIL requested_fullscreen=%d viewport_present=%d viewport_fullscreen=%d"),
            bFullscreenRequested ? 1 : 0,
            bViewportReady ? 1 : 0,
            bFullscreenRuntime ? 1 : 0);
    }
}

void UOCPerformanceSampleSubsystem::Tick(float DeltaTime)
{
    if (bFinished || DeltaTime <= 0.0f || DeltaTime > 1.0f) return;

    UWorld* World = GetWorld();
    if (!World || World->GetNetMode() == NM_DedicatedServer) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC || !PC->IsLocalController()) return;

    // Pass 45: the latest user screenshot shows ~8 FPS in the main menu. Measure the pawn-less frontend
    // before gameplay-only AI/foliage can be blamed. Keep this lightweight and one-shot.
    if (!bFrontendSampleLogged && PC->GetPawn() == nullptr)
    {
        if (FrontendWarmupSeconds < 1.0f)
        {
            FrontendWarmupSeconds += DeltaTime;
            return;
        }

        FrontendSampleSeconds += DeltaTime;
        FrontendAccumulatedFrameSeconds += DeltaTime;
        FrontendWorstFrameSeconds = FMath::Max(FrontendWorstFrameSeconds, DeltaTime);
        ++FrontendFrames;

        if (FrontendSampleSeconds >= 2.0f)
        {
            const float AverageFps = FrontendAccumulatedFrameSeconds > KINDA_SMALL_NUMBER
                ? static_cast<float>(FrontendFrames) / FrontendAccumulatedFrameSeconds : 0.0f;
            const float WorstFrameFps = FrontendWorstFrameSeconds > KINDA_SMALL_NUMBER
                ? 1.0f / FrontendWorstFrameSeconds : 0.0f;

            UE_LOG(LogTemp, Display,
                TEXT("PASS45_RHI_MODE mode=%s norhithread=%d"),
                Pass45RhiMode(),
                FString(FCommandLine::Get()).Contains(TEXT("-norhithread"), ESearchCase::IgnoreCase) ? 1 : 0);
            UE_LOG(LogTemp, Display,
                TEXT("PASS45_FRONTEND_PERF_BASELINE avg_fps=%.1f worst_frame_fps=%.1f frames=%d window_seconds=%.2f quality_mutation=0"),
                AverageFps, WorstFrameFps, FrontendFrames, FrontendSampleSeconds);

            if (AverageFps < 30.0f)
            {
                UE_LOG(LogTemp, Warning,
                    TEXT("PASS45_FRONTEND_PERF_BELOW_TARGET avg_fps=%.1f target_fps=30.0"), AverageFps);
            }
            else
            {
                UE_LOG(LogTemp, Display,
                    TEXT("PASS45_FRONTEND_PERF_30FPS_READY avg_fps=%.1f"), AverageFps);
            }

            bFrontendSampleLogged = true;
        }
        return;
    }

    if (PC->GetPawn() == nullptr) return;

    // The strict normal route requests fullscreen + t.MaxFPS 60. Verify the actual UE runtime state after
    // gameplay possession so Gate C/H cannot be satisfied by command-line intent alone.
    ValidatePass45RecoveryRuntimeContract();

    if (!bFrontendSampleLogged)
    {
        // The user entered gameplay before the 2 s frontend sample completed. Record that fact once rather
        // than pretending a frontend baseline was measured.
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_FRONTEND_PERF_BASELINE_SKIPPED reason=pawn_possessed_before_sample_complete mode=%s"),
            Pass45RhiMode());
        bFrontendSampleLogged = true;
    }

    if (!bProbeComplete)
    {
        if (WarmupSeconds < 2.0f)
        {
            WarmupSeconds += DeltaTime;
            return;
        }

        ProbeSeconds += DeltaTime;
        ProbeFrameSeconds += DeltaTime;
        ++ProbeFrames;
        if (ProbeSeconds < 2.0f) return;

        const float ProbeFps = ProbeFrameSeconds > KINDA_SMALL_NUMBER
            ? static_cast<float>(ProbeFrames) / ProbeFrameSeconds : 0.0f;
        UE_LOG(LogTemp, Display,
            TEXT("PASS15_PERF_PROBE avg_fps=%.1f frames=%d window_seconds=%.2f"),
            ProbeFps, ProbeFrames, ProbeSeconds);

        if (ProbeFps < 20.0f)
        {
            ReportLowFpsProbe(ProbeFps);
        }

        bProbeComplete = true;
        WarmupSeconds = 0.0f;
        return;
    }

    // Let startup activity settle after the probe before final evidence.
    if (WarmupSeconds < 2.0f)
    {
        WarmupSeconds += DeltaTime;
        return;
    }

    SampleSeconds += DeltaTime;
    AccumulatedFrameSeconds += DeltaTime;
    WorstFrameSeconds = FMath::Max(WorstFrameSeconds, DeltaTime);
    ++SampleFrames;

    if (SampleSeconds < 8.0f) return;

    bFinished = true;
    const float AverageFps = AccumulatedFrameSeconds > KINDA_SMALL_NUMBER
        ? static_cast<float>(SampleFrames) / AccumulatedFrameSeconds : 0.0f;
    const float WorstFrameFps = WorstFrameSeconds > KINDA_SMALL_NUMBER
        ? 1.0f / WorstFrameSeconds : 0.0f;

    // Capture world density/memory evidence once beside the settled gameplay sample.
    LogPass18WorldDiagnostics(World, PC);

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_GAMEPLAY_PERF_BASELINE avg_fps=%.1f worst_frame_fps=%.1f frames=%d window_seconds=%.2f rhi_mode=%s quality_mutation=0"),
        AverageFps, WorstFrameFps, SampleFrames, SampleSeconds, Pass45RhiMode());

    UE_LOG(LogTemp, Display,
        TEXT("PASS15_PERF_SAMPLE avg_fps=%.1f worst_frame_fps=%.1f frames=%d window_seconds=%.2f quality_mutation=0"),
        AverageFps, WorstFrameFps, SampleFrames, SampleSeconds);

    // Preserve the Pass 14 marker for existing acceptance tooling while recording the stricter Pass 45 sample above.
    UE_LOG(LogTemp, Display,
        TEXT("PASS14_PERF_SAMPLE avg_fps=%.1f worst_frame_fps=%.1f frames=%d window_seconds=%.2f"),
        AverageFps, WorstFrameFps, SampleFrames, SampleSeconds);

    if (AverageFps < 30.0f)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("PASS14_PERF_BELOW_TARGET avg_fps=%.1f target_fps=30.0"), AverageFps);
        UE_LOG(LogTemp, Warning,
            TEXT("PASS15_PERF_BELOW_TARGET avg_fps=%.1f target_fps=30.0 quality_mutation=0"),
            AverageFps);
        UE_LOG(LogTemp, Warning,
            TEXT("PASS45_GAMEPLAY_PERF_BELOW_TARGET avg_fps=%.1f target_fps=30.0"), AverageFps);
    }
    else
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS14_PERF_30FPS_READY avg_fps=%.1f"), AverageFps);
        UE_LOG(LogTemp, Display,
            TEXT("PASS15_PERF_30FPS_READY avg_fps=%.1f quality_mutation=0"), AverageFps);
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_GAMEPLAY_PERF_30FPS_READY avg_fps=%.1f"), AverageFps);
    }

    UE_LOG(LogTemp, Display,
        TEXT("PASS39_PERF_SAMPLER_IDLE_READY finished=1 further_tick=0"));
}

void UOCPerformanceSampleSubsystem::ReportLowFpsProbe(float ProbeFps) const
{
    // Pass 15 used to react to a low startup probe by dropping resolution to 65%, zeroing several
    // graphics features and changing LOD distances for the rest of the session. Keep the probe evidence-only.
    UE_LOG(LogTemp, Warning,
        TEXT("PASS39_LOW_FPS_PROBE_DIAGNOSTIC avg_fps=%.1f quality_mutation=0 preserve_user_graphics=1"),
        ProbeFps);
}

TStatId UOCPerformanceSampleSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCPerformanceSampleSubsystem, STATGROUP_Tickables);
}
