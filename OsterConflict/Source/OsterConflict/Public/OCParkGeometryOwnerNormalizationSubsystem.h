#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCParkGeometryOwnerNormalizationSubsystem.generated.h"

/**
 * PASS45 Gate K ParkGeometry semantic-owner normalization.
 *
 * Temporary, fail-closed, geometry-preserving migration bridge. The current primary world author stores two
 * Central Park ground footprints and one college recreation footprint in one ParkGeometry ISM. This subsystem
 * re-homes those three exact source instances into exact semantic owners without changing mesh, material,
 * transform, collision or gameplay authority. It is deliberately primary_authoring=0 and must be retired when
 * AOCWorldSectorOster directly authors the three semantic families.
 *
 * This bridge may never hide rejected geometry or invent an authored-content pass. The resulting Cube-backed
 * semantic owners remain visible Gate K CONTENT GAP until exact authored ground presentation is supplied.
 */
UCLASS()
class OSTERCONFLICT_API UOCParkGeometryOwnerNormalizationSubsystem : public UTickableWorldSubsystem
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
