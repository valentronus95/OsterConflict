#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Tickable.h"
#include "OCParkMemorialApproachAuthoredUpgradeSubsystem.generated.h"

/**
 * PASS45 Gate K: replaces only the homogeneous ParkMemorialApproach source family with a tracked authored mesh.
 * ParkMemorialPlaza, ParkSkateFitness, ParkBenches and legacy ParkDetails remain separate ownership domains.
 */
UCLASS()
class OSTERCONFLICT_API UOCParkMemorialApproachAuthoredUpgradeSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override { return !bFinished; }

private:
    bool bFinished = false;
    float ElapsedSeconds = 0.0f;
};
