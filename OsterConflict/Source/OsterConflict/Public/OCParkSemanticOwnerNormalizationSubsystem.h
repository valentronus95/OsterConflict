#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCParkSemanticOwnerNormalizationSubsystem.generated.h"

/**
 * PASS45 Gate K migration bridge for Central Park mixed semantic source families.
 *
 * Historical source authoring still places the memorial slab+monument in ParkMemorialPlaza and the skate pad+ramps
 * in ParkSkateFitness. Until those large primary-source declarations are safely split, this subsystem performs one
 * fail-closed, geometry-preserving re-home before authored detail upgrades and before Gate K observes the world.
 *
 * This is deliberately marked primary_authoring=0. It must be retired when AOCWorldSectorOster directly authors the
 * exact semantic owners. It may never hide rejected geometry or claim runtime/Gate K acceptance by itself.
 */
UCLASS()
class OSTERCONFLICT_API UOCParkSemanticOwnerNormalizationSubsystem : public UTickableWorldSubsystem
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
