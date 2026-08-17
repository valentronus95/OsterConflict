#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13VehicleArtSubsystem.generated.h"

class AOCVehicleBase;

/**
 * R13 vehicle art bridge.
 *
 * The gameplay vehicles deliberately keep their simple authoritative collision/physics bodies, but their visible
 * cube/cylinder prototypes are replaced at runtime by real meshes already present in VehicleVarietyPack. This keeps
 * network/vehicle code stable while removing the most obvious source-only geometry from the player-facing build.
 */
UCLASS()
class OSTERCONFLICT_API UOCR13VehicleArtSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;

private:
    float ScanAccumulator = 0.0f;
    TSet<TWeakObjectPtr<AOCVehicleBase>> ProcessedVehicles;

    void TryApplyVehicleArt(AOCVehicleBase* Vehicle);
};
