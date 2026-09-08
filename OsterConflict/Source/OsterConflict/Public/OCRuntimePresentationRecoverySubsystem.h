#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCRuntimePresentationRecoverySubsystem.generated.h"

/**
 * Runtime fail-soft recovery for presentation-only world failures plus the R13 frontend field padding fix.
 *
 * Presentation stages are allowed to fail visibly without trapping a ready human controller with no pawn.
 * The normal AOCGameModeRuntimeSafe spawn gate remains authoritative while those stages are merely loading.
 */
UCLASS()
class OSTERCONFLICT_API UOCRuntimePresentationRecoverySubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickableWhenPaused() const override { return true; }

private:
    bool HasPresentationFailure(FString& OutStages) const;
    void ApplyFrontendFieldPaddingFix();
    void ReleaseReadyPlayersFromPresentationFailure();

    float TickAccumulator = 0.0f;
    bool bFrontendFieldPaddingFixed = false;
    TSet<TWeakObjectPtr<AController>> FailsoftSpawnAttempts;
};
