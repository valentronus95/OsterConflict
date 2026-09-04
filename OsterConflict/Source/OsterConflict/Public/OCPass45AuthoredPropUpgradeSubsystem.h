#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCPass45AuthoredPropUpgradeSubsystem.generated.h"

/**
 * PASS45 authored-prop cutover for source owners that still render Engine BasicShape placeholders.
 *
 * The first bounded cutover replaces the 14 central-park cube benches with the tracked Street Props V2
 * bench asset while preserving source placement, yaw and ground contact. The subsystem is intentionally
 * runtime-safe: it waits for the canonical Oster sector, preflights every instance, and rolls back on a
 * failed write instead of leaving a half-mutated owner.
 */
UCLASS()
class OSTERCONFLICT_API UOCPass45AuthoredPropUpgradeSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override { return !bFinished; }

private:
    float ElapsedSeconds = 0.0f;
    bool bFinished = false;
};
