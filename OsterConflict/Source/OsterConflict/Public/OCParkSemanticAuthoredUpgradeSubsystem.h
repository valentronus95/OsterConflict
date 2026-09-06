#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCParkSemanticAuthoredUpgradeSubsystem.generated.h"

/**
 * PASS45 Gate K semantic Central Park authored-content upgrade.
 *
 * The world source keeps deterministic semantic proxy transforms for park details. This subsystem only upgrades
 * homogeneous semantic families when an exact tracked authored family is available. It must not blanket-remap
 * ParkDetails, memorial, skate/fitness or other unrelated proxy groups.
 */
UCLASS()
class OSTERCONFLICT_API UOCParkSemanticAuthoredUpgradeSubsystem : public UTickableWorldSubsystem
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
