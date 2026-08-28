#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCBlock0FoliageCoverageValidationSubsystem.generated.h"

/**
 * PASS45 Block 0 validation-only spatial coverage gate.
 *
 * PopulationComplete proves the generator traversed its cursor, not that grass actually exists across Oster.
 * This subsystem observes the final DenseGrass HISM distribution and fails closed when accepted instances remain
 * concentrated in a small crop. It never spawns, moves, hides, destroys or repairs world content.
 */
UCLASS()
class OSTERCONFLICT_API UOCBlock0FoliageCoverageValidationSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override { return !bFinished; }

private:
    void FailValidation(const FString& Reason);

    float ElapsedSeconds = 0.0f;
    float ValidationAccumulator = 0.0f;
    bool bFinished = false;
};
