#pragma once

#include "CoreMinimal.h"
#include "OCArmedVehicleBase.h"
#include "OCBTR.generated.h"

class AController;
class AOCCharacter;
class UStaticMeshComponent;

/** S11 armoured personnel carrier prototype. Small-arms and vehicle-gun damage cannot destroy its hull. */
UCLASS()
class OSTERCONFLICT_API AOCBTR : public AOCArmedVehicleBase
{
    GENERATED_BODY()

public:
    AOCBTR();
    virtual void Tick(float DeltaSeconds) override;
    virtual void PossessedBy(AController* NewController) override;
    virtual void UnPossessed() override;
    virtual void PawnClientRestart() override;

protected:
    virtual bool CanHullAcceptDamage(const FDamageEvent& DamageEvent) const override;
    virtual float ModifyHullDamage(float DamageAmount, const FDamageEvent& DamageEvent) const override;
    virtual void ApplyVehicleStyle() override;
    virtual float GetCollisionDamageScale() const override { return 0.0f; }

private:
    bool ValidateProductionBTR4MaterialState(const TCHAR* Phase);

    // PASS45 item 30: unlike the open HMMWV ring, BTR gunner gameplay is a remote optic.
    // The inherited gunner camera pivot is re-parented to BarrelPivot so the sensor follows
    // both turret yaw and barrel pitch. This value is presentation-only and does not alter aim authority.
    float BTRRemoteOpticFieldOfView = 48.0f;
    TWeakObjectPtr<AOCCharacter> ActiveOpticGunner;

    UPROPERTY() TObjectPtr<UStaticMeshComponent> UpperHull;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> NoseArmor;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> RearArmor;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> WheelExtraFL;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> WheelExtraFR;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> WheelExtraRL;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> WheelExtraRR;
};
