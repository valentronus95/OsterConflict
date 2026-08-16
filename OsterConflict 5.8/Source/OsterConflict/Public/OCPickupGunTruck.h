#pragma once

#include "CoreMinimal.h"
#include "OCArmedVehicleBase.h"
#include "OCPickupGunTruck.generated.h"

class UStaticMeshComponent;

/** Light two-player gun truck. Driver cannot fire; the second player occupies the mounted MG. */
UCLASS()
class OSTERCONFLICT_API AOCPickupGunTruck : public AOCArmedVehicleBase
{
    GENERATED_BODY()

public:
    AOCPickupGunTruck();

protected:
    virtual void ApplyVehicleStyle() override;

private:
    UPROPERTY() TObjectPtr<UStaticMeshComponent> CabRoof;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> BedFloor;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> BedLeft;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> BedRight;
};
