#include "OCBTR.h"

#include "OCDamageTypes.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AOCBTR::AOCBTR()
{
    TurretDisplayName = TEXT("APC TURRET");
    TurretDamage = 58.0f;
    TurretRoundsPerMinute = 340.0f;
    TurretRangeCm = 16500.0f;
    TurretSpreadDegrees = 0.34f;
    TurretMagazineSize = 40;
    StartingTurretReserveAmmo = 160;
    TurretReloadSeconds = 5.4f;
    TurretDamageTypeClass = UOCVehicleCannonDamageType::StaticClass();
    MaxTurretYaw = 180.0f;
    MinTurretPitch = -10.0f;
    MaxTurretPitch = 38.0f;

    VehicleMassKg = 11800.0f;
    SpringStiffness = 37000.0f;
    SuspensionDamping = 6500.0f;

    // R12.2 driveability pass: the old prototype took far too long to get out of the teens.
    // Keep the 82 km/h cap, but give the APC enough low-speed drive force to reach 30+ km/h promptly.
    DriveForce = 3600000.0f;
    RollingBrakeForce = 450000.0f;
    HandbrakeForce = 1800000.0f;
    LateralGrip = 18000.0f;
    SteeringTorque = 170000000.0f;
    AeroDrag = 0.20f;
    MaxForwardSpeedKmh = 82.0f;
    MaxVehicleHealth = 1600.0f;
    WreckLifetimeSeconds = 38.0f;

    PhysicsBody->SetBoxExtent(FVector(315.0f, 128.0f, 78.0f));
    TurretPivot->SetRelativeLocation(FVector(-25.0f, 0.0f, 155.0f));
    MuzzlePoint->SetRelativeLocation(FVector(245.0f, 0.0f, 0.0f));
    BarrelMesh->SetRelativeLocation(FVector(118.0f, 0.0f, 0.0f));
    BarrelMesh->SetRelativeScale3D(FVector(2.35f, 0.11f, 0.11f));
    TurretBaseMesh->SetRelativeScale3D(FVector(1.05f, 1.05f, 0.34f));

    UpperHull = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("UpperHull"));
    NoseArmor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NoseArmor"));
    RearArmor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RearArmor"));
    WheelExtraFL = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WheelExtraFL"));
    WheelExtraFR = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WheelExtraFR"));
    WheelExtraRL = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WheelExtraRL"));
    WheelExtraRR = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WheelExtraRR"));

    UStaticMeshComponent* BTRParts[] = {UpperHull.Get(), NoseArmor.Get(), RearArmor.Get(),
        WheelExtraFL.Get(), WheelExtraFR.Get(), WheelExtraRL.Get(), WheelExtraRR.Get()};
    for (UStaticMeshComponent* C : BTRParts)
    {
        C->SetupAttachment(PhysicsBody);
        C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    if (CubeMesh.Succeeded())
    {
        UpperHull->SetStaticMesh(CubeMesh.Object);
        NoseArmor->SetStaticMesh(CubeMesh.Object);
        RearArmor->SetStaticMesh(CubeMesh.Object);
    }
    if (CylinderMesh.Succeeded())
    {
        UStaticMeshComponent* ExtraWheelMeshes[] = {WheelExtraFL.Get(), WheelExtraFR.Get(), WheelExtraRL.Get(), WheelExtraRR.Get()};
        for (UStaticMeshComponent* W : ExtraWheelMeshes) W->SetStaticMesh(CylinderMesh.Object);
    }

    UpperHull->SetRelativeLocation(FVector(-10.0f, 0.0f, 78.0f));
    UpperHull->SetRelativeScale3D(FVector(5.65f, 2.28f, 0.72f));
    NoseArmor->SetRelativeLocation(FVector(295.0f, 0.0f, 40.0f));
    NoseArmor->SetRelativeRotation(FRotator(0.0f, 0.0f, 18.0f));
    NoseArmor->SetRelativeScale3D(FVector(0.42f, 2.18f, 0.86f));
    RearArmor->SetRelativeLocation(FVector(-305.0f, 0.0f, 42.0f));
    RearArmor->SetRelativeScale3D(FVector(0.30f, 2.20f, 0.90f));

    const FVector PrimaryAxleWheelPositions[] = {
        FVector(225.0f, -128.0f, -62.0f), FVector(225.0f, 128.0f, -62.0f),
        FVector(-225.0f, -128.0f, -62.0f), FVector(-225.0f, 128.0f, -62.0f)
    };
    const FVector MidAxleWheelPositions[] = {
        FVector(75.0f, -128.0f, -62.0f), FVector(75.0f, 128.0f, -62.0f),
        FVector(-75.0f, -128.0f, -62.0f), FVector(-75.0f, 128.0f, -62.0f)
    };
    UStaticMeshComponent* ExtraWheels[] = {WheelExtraFL.Get(), WheelExtraFR.Get(), WheelExtraRL.Get(), WheelExtraRR.Get()};
    ClearSuspensionPointsLocal();
    for (int32 I = 0; I < 4; ++I)
    {
        if (WheelVisuals.IsValidIndex(I))
        {
            WheelVisuals[I]->SetRelativeLocation(PrimaryAxleWheelPositions[I]);
            WheelVisuals[I]->SetRelativeScale3D(FVector(0.72f, 0.72f, 0.30f));
        }
        ExtraWheels[I]->SetRelativeLocation(MidAxleWheelPositions[I]);
        ExtraWheels[I]->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
        ExtraWheels[I]->SetRelativeScale3D(FVector(0.72f, 0.72f, 0.30f));
        WheelVisuals.Add(ExtraWheels[I]);
        AddSuspensionPointLocal(PrimaryAxleWheelPositions[I] + FVector(0.0f, 0.0f, 24.0f));
        AddSuspensionPointLocal(MidAxleWheelPositions[I] + FVector(0.0f, 0.0f, 24.0f));
    }
}

bool AOCBTR::CanHullAcceptDamage(const FDamageEvent& DamageEvent) const
{
    const UClass* DamageClass = DamageEvent.DamageTypeClass;
    return DamageClass && DamageClass->IsChildOf(UOCAntiArmorDamageType::StaticClass());
}

float AOCBTR::ModifyHullDamage(float DamageAmount, const FDamageEvent&) const
{
    return DamageAmount;
}

void AOCBTR::ApplyVehicleStyle()
{
    Chassis->SetRelativeScale3D(FVector(6.15f, 2.45f, 0.72f));
    InteriorCamera->SetRelativeLocation(FVector(130.0f, -52.0f, 105.0f));
    ThirdPersonSpringArm->TargetArmLength = 820.0f;
    ThirdPersonSpringArm->SetRelativeLocation(FVector(-80.0f, 0.0f, 220.0f));
}
