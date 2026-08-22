#include "OCPerformanceSampleSubsystem.h"

#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

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

    if (WarmupSeconds < 5.0f)
    {
        WarmupSeconds += DeltaTime;
        return;
    }

    SampleSeconds += DeltaTime;
    AccumulatedFrameSeconds += DeltaTime;
    WorstFrameSeconds = FMath::Max(WorstFrameSeconds, DeltaTime);
    ++SampleFrames;

    if (SampleSeconds < 10.0f) return;

    bFinished = true;
    const float AverageFps = AccumulatedFrameSeconds > KINDA_SMALL_NUMBER
        ? static_cast<float>(SampleFrames) / AccumulatedFrameSeconds : 0.0f;
    const float WorstFrameFps = WorstFrameSeconds > KINDA_SMALL_NUMBER
        ? 1.0f / WorstFrameSeconds : 0.0f;

    UE_LOG(LogTemp, Display,
        TEXT("PASS14_PERF_SAMPLE avg_fps=%.1f worst_frame_fps=%.1f frames=%d window_seconds=%.2f"),
        AverageFps, WorstFrameFps, SampleFrames, SampleSeconds);

    if (AverageFps < 30.0f)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("PASS14_PERF_BELOW_TARGET avg_fps=%.1f target_fps=30.0"), AverageFps);
    }
    else
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS14_PERF_30FPS_READY avg_fps=%.1f"), AverageFps);
    }
}

TStatId UOCPerformanceSampleSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCPerformanceSampleSubsystem, STATGROUP_Tickables);
}
