#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13VehicleGameplayRepairSubsystem.generated.h"

class AOCCharacter;
class AOCVehicleBase;

/** R13.6 playtest fixes for imported vehicle grounding, re-entry drive state and pickup mounted-gun presentation. */
UCLASS()
class OSTERCONFLICT_API UOCR13VehicleGameplayRepairSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;

private:
    void RepairVehicle(AOCVehicleBase* Vehicle);
    void RepairDriverTransition(AOCVehicleBase* Vehicle);
    void RepairImportedChassisGrounding(AOCVehicleBase* Vehicle);
    void EnsurePickupMountedMachineGun(AOCVehicleBase* Vehicle);

    float ScanAccumulator = 0.0f;
    TMap<TWeakObjectPtr<AOCVehicleBase>, TWeakObjectPtr<AOCCharacter>> LastDriverByVehicle;
    TSet<TWeakObjectPtr<AOCVehicleBase>> GroundingRepairedVehicles;
    TSet<TWeakObjectPtr<AOCVehicleBase>> MountedGunRepairedVehicles;
};
