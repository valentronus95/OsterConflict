#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCProductionVehicleVisualGuardSubsystem.generated.h"

/**
 * Short-lived runtime guard for imported HMMWV / M2 / BTR-4 visuals.
 * VehicleBase historically painted every StaticMeshComponent with BasicShapeMaterial after the
 * derived production mesh had already been attached. This guard restores the mesh-authored slots
 * for /Game/Production assets and then leaves the timer manager.
 */
UCLASS()
class OSTERCONFLICT_API UOCProductionVehicleVisualGuardSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

private:
    FTimerHandle AuditTimer;
    int32 AuditPass = 0;

    void AuditProductionVisuals();
};