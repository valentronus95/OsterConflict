#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCParkGroundAuthoredUpgradeSubsystem.generated.h"

/**
 * PASS45 Gate K authored ground presentation for exact semantic owners created directly by AOCWorldSectorOster.
 *
 * ParkCentralGround, ParkNorthCivicGround and CollegeRecreationGround are primary actor-owned source families.
 * This subsystem upgrades only their presentation to the tracked authored plane + grass material pair. It preserves
 * XY footprint, yaw and source surface-top elevation, and rolls back the whole three-owner mutation if any
 * post-preflight write fails.
 *
 * Semantic ownership is primary_authoring=1 / normalization_bridge=0. The presentation upgrade remains a bounded
 * pre-Gate-K source step and does not constitute UE 5.8 visual acceptance.
 */
UCLASS()
class OSTERCONFLICT_API UOCParkGroundAuthoredUpgradeSubsystem : public UTickableWorldSubsystem
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
