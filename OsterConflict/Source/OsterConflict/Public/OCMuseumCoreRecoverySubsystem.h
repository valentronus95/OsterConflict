#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "OCMuseumCoreRecoverySubsystem.generated.h"

/**
 * Pass 35 runtime recovery for the museum core.
 *
 * The photo shell (R13.7) historically hard-depended on optional presentation assets. If one of
 * those assets failed to load, R13.7 returned before creating its owner actor and R13.8 then refused
 * to build the enterable architecture. The result was a valid BASE/rack looking into an empty field.
 *
 * This subsystem never creates a competing visible museum. It only guarantees the lightweight owner
 * carrier required by the already-authoritative R13.8/R14.x stages, then replays those stages once.
 */
UCLASS()
class OSTERCONFLICT_API UOCMuseumCoreRecoverySubsystem final : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

private:
    FTimerHandle RecoveryTimer;

    void EnsureMuseumCore();
};
