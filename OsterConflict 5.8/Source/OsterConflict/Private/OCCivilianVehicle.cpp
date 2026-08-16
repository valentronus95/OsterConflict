#include "OCCivilianVehicle.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"

AOCCivilianVehicle::AOCCivilianVehicle()
{
    VehicleMassKg = 1450.0f;
    MaxVehicleHealth = 450.0f;
    MaxForwardSpeedKmh = 135.0f;
}

void AOCCivilianVehicle::SetVehicleStyleServer(EOCCivilianVehicleStyle NewStyle)
{
    if (!HasAuthority())
    {
        return;
    }
    VehicleStyle = NewStyle;
    ApplyVehicleStyle();
    ForceNetUpdate();
}

void AOCCivilianVehicle::ApplyVehicleStyle()
{
    if (!Chassis)
    {
        return;
    }

    switch (VehicleStyle)
    {
    case EOCCivilianVehicleStyle::Sedan:
        Chassis->SetRelativeScale3D(FVector(4.55f, 1.80f, 0.54f));
        if (PhysicsBody) PhysicsBody->SetBoxExtent(FVector(228.0f, 90.0f, 47.0f));
        VehicleMassKg = 1380.0f;
        MaxForwardSpeedKmh = 142.0f;
        break;
    case EOCCivilianVehicleStyle::Hatchback:
        Chassis->SetRelativeScale3D(FVector(3.95f, 1.76f, 0.58f));
        if (PhysicsBody) PhysicsBody->SetBoxExtent(FVector(198.0f, 88.0f, 49.0f));
        VehicleMassKg = 1260.0f;
        MaxForwardSpeedKmh = 128.0f;
        break;
    case EOCCivilianVehicleStyle::Wagon:
    default:
        Chassis->SetRelativeScale3D(FVector(4.65f, 1.84f, 0.60f));
        if (PhysicsBody) PhysicsBody->SetBoxExtent(FVector(233.0f, 92.0f, 50.0f));
        VehicleMassKg = 1490.0f;
        MaxForwardSpeedKmh = 136.0f;
        break;
    }
    if (PhysicsBody) PhysicsBody->SetMassOverrideInKg(NAME_None, VehicleMassKg, true);
}

void AOCCivilianVehicle::OnRep_VehicleStyle()
{
    ApplyVehicleStyle();
}

void AOCCivilianVehicle::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AOCCivilianVehicle, VehicleStyle);
}
