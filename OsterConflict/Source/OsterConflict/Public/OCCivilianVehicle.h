#pragma once

#include "CoreMinimal.h"
#include "OCVehicleBase.h"
#include "OCCivilianVehicle.generated.h"

UENUM(BlueprintType)
enum class EOCCivilianVehicleStyle : uint8
{
    Wagon,
    Sedan,
    Hatchback,
    BoxTruck
};

UCLASS()
class OSTERCONFLICT_API AOCCivilianVehicle : public AOCVehicleBase
{
    GENERATED_BODY()

public:
    AOCCivilianVehicle();

    void SetVehicleStyleServer(EOCCivilianVehicleStyle NewStyle);

    UFUNCTION(BlueprintPure, Category="Vehicle")
    EOCCivilianVehicleStyle GetVehicleStyle() const { return VehicleStyle; }

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    virtual void ApplyVehicleStyle() override;

private:
    UPROPERTY(ReplicatedUsing=OnRep_VehicleStyle)
    EOCCivilianVehicleStyle VehicleStyle = EOCCivilianVehicleStyle::Wagon;

    UFUNCTION()
    void OnRep_VehicleStyle();
};