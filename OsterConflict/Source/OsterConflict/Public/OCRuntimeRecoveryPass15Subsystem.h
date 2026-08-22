#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCRuntimeRecoveryPass15Subsystem.generated.h"

class AOCPlayerController;
class UButton;
class UOCGameUIRootWidget;

/**
 * Runtime recovery for the 2026-08-23 laptop playtest failures.
 * Keeps server/network UI readable, provides a reliable host-travel fallback,
 * and returns failed direct-connect attempts to a clean frontend instead of
 * exposing the frontend-only gameplay shell.
 */
UCLASS()
class OSTERCONFLICT_API UOCRuntimeRecoveryPass15Subsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;

private:
    TWeakObjectPtr<UButton> BoundPrimaryButton;
    FTimerHandle HostFallbackTimer;
    bool bStylePassLogged = false;
    bool bJoinPending = false;

    void ApplyFrontendRepairs();
    void ApplyJoinPendingOverlay(UOCGameUIRootWidget* Root);
    void BindPrimaryFallback(UOCGameUIRootWidget* Root);
    void RecoverConnectionFailure(AOCPlayerController* PC);
    void RunHostTravelFallback();

    UFUNCTION()
    void OnPrimaryClickedFallback();
};
