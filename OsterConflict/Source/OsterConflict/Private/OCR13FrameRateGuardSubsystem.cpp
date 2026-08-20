#include "OCR13FrameRateGuardSubsystem.h"

#include "OCPlayerController.h"

#include "Engine/Engine.h"
#include "Engine/World.h"

namespace
{
    constexpr float FrontendCap = 45.0f;
    constexpr float MaximumGameplayCap = 60.0f;
}

bool UOCR13FrameRateGuardSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13FrameRateGuardSubsystem::Tick(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World || DeltaTime <= 0.0f || !GEngine) return;

    AOCPlayerController* PC = Cast<AOCPlayerController>(World->GetFirstPlayerController());
    if (!PC || !PC->IsLocalController()) return;

    if (!bCapturedPreviousCap)
    {
        PreviousEngineCap = GEngine->GetMaxFPS();
        // Respect a deliberate lower cap if one already existed. Unlimited/very high caps are bounded to 60 for
        // this laptop-oriented playtest branch; no GameUserSettings config is written or saved here.
        GameplayCap = PreviousEngineCap > 0.0f
            ? FMath::Clamp(PreviousEngineCap, 30.0f, MaximumGameplayCap)
            : MaximumGameplayCap;
        bCapturedPreviousCap = true;
    }

    const bool bPregameFrontend = PC->IsFrontendMenuVisible() && PC->GetPawn() == nullptr;
    const float TargetCap = bPregameFrontend ? FMath::Min(FrontendCap, GameplayCap) : GameplayCap;
    ApplyCap(TargetCap);
}

void UOCR13FrameRateGuardSubsystem::ApplyCap(const float NewCap)
{
    if (!GEngine || NewCap <= 0.0f || FMath::IsNearlyEqual(ActiveCap, NewCap, 0.1f)) return;
    GEngine->SetMaxFPS(NewCap);
    ActiveCap = NewCap;
    UE_LOG(LogTemp, Display, TEXT("R13.6 frame-rate guard: runtime cap %.0f FPS."), NewCap);
}

void UOCR13FrameRateGuardSubsystem::Deinitialize()
{
    if (GEngine && bCapturedPreviousCap)
    {
        GEngine->SetMaxFPS(PreviousEngineCap);
    }
    ActiveCap = -1.0f;
    Super::Deinitialize();
}

TStatId UOCR13FrameRateGuardSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCR13FrameRateGuardSubsystem, STATGROUP_Tickables);
}
