#include "OCCivilianVehicle.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"

AOCCivilianVehicle::AOCCivilianVehicle()
{
    VehicleMassKg = 1450.0f;
    MaxVehicleHealth = 450.0f;

    // R13 gameplay tuning: these are road cars, not 30 km/h utility carts. Keep the prototype physics stable while
    // giving them a believable road-gameplay envelope and enough low-speed steering authority for streets.
    DriveForce = 1200000.0f;
    SteeringTorque = 145000000.0f;
    LateralGrip = 10500.0f;
    AeroDrag = 0.10f;
    MaxForwardSpeedKmh = 88.0f;
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

    // Reset the shared road-car handling and suspension before applying a heavier/special style.
    DriveForce = 1200000.0f;
    SteeringTorque = 145000000.0f;
    LateralGrip = 10500.0f;
    AeroDrag = 0.10f;
    MaxVehicleHealth = 450.0f;
    SuspensionTraceLengthCm = 78.0f;
    WheelRadiusCm = 31.0f;
    SpringStiffness = 11500.0f;
    SuspensionDamping = 2100.0f;

    auto ConfigureFourWheelSuspension = [this](float LongitudinalCm, float LateralCm, float LocalZCm)
    {
        ClearSuspensionPointsLocal();
        AddSuspensionPointLocal(FVector( LongitudinalCm, -LateralCm, LocalZCm));
        AddSuspensionPointLocal(FVector( LongitudinalCm,  LateralCm, LocalZCm));
        AddSuspensionPointLocal(FVector(-LongitudinalCm, -LateralCm, LocalZCm));
        AddSuspensionPointLocal(FVector(-LongitudinalCm,  LateralCm, LocalZCm));
    };

    switch (VehicleStyle)
    {
    case EOCCivilianVehicleStyle::Sedan:
        Chassis->SetRelativeScale3D(FVector(4.55f, 1.80f, 0.54f));
        if (PhysicsBody) PhysicsBody->SetBoxExtent(FVector(228.0f, 90.0f, 47.0f));
        VehicleMassKg = 1380.0f;
        MaxForwardSpeedKmh = 90.0f;
        ConfigureFourWheelSuspension(150.0f, 82.0f, -30.0f);
        break;
    case EOCCivilianVehicleStyle::Hatchback:
        Chassis->SetRelativeScale3D(FVector(3.95f, 1.76f, 0.58f));
        if (PhysicsBody) PhysicsBody->SetBoxExtent(FVector(198.0f, 88.0f, 49.0f));
        VehicleMassKg = 1260.0f;
        MaxForwardSpeedKmh = 85.0f;
        ConfigureFourWheelSuspension(132.0f, 80.0f, -31.0f);
        break;
    case EOCCivilianVehicleStyle::BoxTruck:
        Chassis->SetRelativeScale3D(FVector(6.25f, 2.20f, 2.55f));
        if (PhysicsBody) PhysicsBody->SetBoxExtent(FVector(313.0f, 110.0f, 128.0f));
        VehicleMassKg = 4200.0f;
        MaxVehicleHealth = 700.0f;
        DriveForce = 1850000.0f;
        SteeringTorque = 190000000.0f;
        LateralGrip = 14000.0f;
        AeroDrag = 0.14f;
        MaxForwardSpeedKmh = 72.0f;

        // The old BoxTruck inherited compact-car suspension points near Z=-31, so its traces often never produced
        // useful ground contact for the much taller physics body. Match the contact plane and spring capacity to 4.2 t.
        SuspensionTraceLengthCm = 92.0f;
        WheelRadiusCm = 42.0f;
        SpringStiffness = 22000.0f;
        SuspensionDamping = 3800.0f;
        ConfigureFourWheelSuspension(205.0f, 95.0f, -106.0f);
        break;
    case EOCCivilianVehicleStyle::Wagon:
    default:
        Chassis->SetRelativeScale3D(FVector(4.65f, 1.84f, 0.60f));
        if (PhysicsBody) PhysicsBody->SetBoxExtent(FVector(233.0f, 92.0f, 50.0f));
        VehicleMassKg = 1490.0f;
        MaxForwardSpeedKmh = 88.0f;
        ConfigureFourWheelSuspension(154.0f, 84.0f, -32.0f);
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
