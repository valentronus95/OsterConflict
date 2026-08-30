#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCParkHardscapeAuthoredUpgradeSubsystem.generated.h"

/**
 * PASS45 Gate K authored hardscape presentation for the exact flat semantic owners produced by the temporary
 * Central Park semantic-normalization bridge.
 *
 * Only ParkMemorialSurface and ParkSkateSurface are upgraded to a tracked authored plane + concrete material.
 * ParkMemorialMonument and ParkSkateRamps remain separate content gaps and are deliberately not mutated here.
 * XY footprint, yaw and source top-surface elevation are preserved, with transaction rollback if either write fails.
 *
 * This remains primary_authoring=0 while the normalization bridge exists. Direct AOCWorldSectorOster semantic
 * authoring must eventually replace the bridge; source verification is not UE 5.8 visual acceptance.
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
