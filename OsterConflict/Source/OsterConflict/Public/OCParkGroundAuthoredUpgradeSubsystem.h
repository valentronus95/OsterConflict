#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCParkGroundAuthoredUpgradeSubsystem.generated.h"

/**
 * PASS45 Gate K authored ground presentation for the exact semantic owners produced by the temporary
 * ParkGeometry normalization bridge.
 *
 * The legacy source tint is explicitly green park ground. This subsystem therefore upgrades exactly
 * ParkCentralGround, ParkNorthCivicGround and CollegeRecreationGround to the tracked authored plane + grass
 * material pair. It preserves XY footprint, yaw and source surface-top elevation, and rolls back the whole
 * three-owner mutation if any post-preflight write fails.
 *
 * This remains primary_authoring=0 while the normalization bridge exists. Direct AOCWorldSectorOster semantic
 * authoring must eventually replace both bridges; source verification is not UE 5.8 visual acceptance.
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
