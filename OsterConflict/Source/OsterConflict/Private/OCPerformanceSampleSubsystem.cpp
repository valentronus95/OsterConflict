#include "OCPerformanceSampleSubsystem.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "HAL/PlatformMemory.h"
#include "NavigationSystem.h"

namespace
{
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

void UOCPerformanceSampleSubsystem::Tick(float DeltaTime)
{
    if (bFinished || DeltaTime <= 0.0f || DeltaTime > 1.0f) return;

    UWorld* World = GetWorld();
    if (!World || World->GetNetMode() == NM_DedicatedServer) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC || !PC->IsLocalController() || PC->GetPawn() == nullptr) return;

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

    // Pass 18 is diagnostics-only. Capture world density/memory evidence once, beside the final FPS sample,
    // so the next optimization decision is based on the same settled gameplay window.
    LogPass18WorldDiagnostics(World, PC);

    UE_LOG(LogTemp, Display,
        TEXT("PASS15_PERF_SAMPLE avg_fps=%.1f worst_frame_fps=%.1f frames=%d window_seconds=%.2f quality_mutation=0"),
        AverageFps, WorstFrameFps, SampleFrames, SampleSeconds);

    // Preserve the Pass 14 marker for existing acceptance tooling while recording the stricter Pass 15 sample above.
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
    }
    else
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS14_PERF_30FPS_READY avg_fps=%.1f"), AverageFps);
        UE_LOG(LogTemp, Display,
            TEXT("PASS15_PERF_30FPS_READY avg_fps=%.1f quality_mutation=0"), AverageFps);
    }

    UE_LOG(LogTemp, Display,
        TEXT("PASS39_PERF_SAMPLER_IDLE_READY finished=1 further_tick=0"));
}

void UOCPerformanceSampleSubsystem::ReportLowFpsProbe(float ProbeFps) const
{
    // Pass 15 used to react to a low startup probe by dropping resolution to 65%, zeroing several
    // graphics features and changing LOD distances for the rest of the session. That made screenshots
    // visibly worse while hiding the real runtime bottleneck. Keep the probe as evidence only.
    UE_LOG(LogTemp, Warning,
        TEXT("PASS39_LOW_FPS_PROBE_DIAGNOSTIC avg_fps=%.1f quality_mutation=0 preserve_user_graphics=1"),
        ProbeFps);
}

TStatId UOCPerformanceSampleSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCPerformanceSampleSubsystem, STATGROUP_Tickables);
}
