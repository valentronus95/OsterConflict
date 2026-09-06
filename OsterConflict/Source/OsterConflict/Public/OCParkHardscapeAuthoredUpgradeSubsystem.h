#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCParkHardscapeAuthoredUpgradeSubsystem.generated.h"

/**
 * PASS45 Gate K authored hardscape presentation for exact flat semantic owners created directly by AOCWorldSectorOster.
 *
 * Only ParkMemorialSurface and ParkSkateSurface are upgraded to a tracked authored plane + concrete material.
 * ParkMemorialMonument and ParkSkateRamps remain separate content gaps and are deliberately not mutated here.
 * XY footprint, yaw and source top-surface elevation are preserved, with transaction rollback if either write fails.
 *
 * Semantic ownership is primary_authoring=1 / normalization_bridge=0. This presentation upgrade remains source-only
 * evidence until direct UE 5.8 visual acceptance.
 */
UCLASS()
class OSTERCONFLICT_API UOCParkHardscapeAuthoredUpgradeSubsystem : public UTickableWorldSubsystem
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
