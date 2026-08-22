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

namespace
{
    bool ApplyFittedBTRMesh(UStaticMeshComponent* Component, UStaticMesh* Mesh,
        const FVector& DesiredSizeCm, float GroundZCm)
    {
        if (!Component || !Mesh) return false;
        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector NativeSize = Bounds.BoxExtent * 2.0f;
        if (NativeSize.X <= 1.0f || NativeSize.Y <= 1.0f || NativeSize.Z <= 1.0f) return false;

        const FVector Scale(
            DesiredSizeCm.X / NativeSize.X,
            DesiredSizeCm.Y / NativeSize.Y,
            DesiredSizeCm.Z / NativeSize.Z);
        FVector Location = -Bounds.Origin * Scale;
        const float NativeBottomZ = Bounds.Origin.Z - Bounds.BoxExtent.Z;
        Location.Z = GroundZCm - NativeBottomZ * Scale.Z;

        Component->SetStaticMesh(Mesh);
        Component->SetRelativeRotation(FRotator::ZeroRotator);
        Component->SetRelativeScale3D(Scale);
        Component->SetRelativeLocation(Location);
        // Remove fallback component overrides while retaining the BTR mesh's imported materials.
        Component->EmptyOverrideMaterials();
        return true;
    }

    void DisableVisualProxy(UStaticMeshComponent* Component)
    {
        if (!Component) return;
        Component->SetVisibility(false, true);
        Component->SetHiddenInGame(true, true);
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(false);
        Component->SetCastShadow(false);
    }
}

AOCBTR::AOCBTR()
{
    TurretDisplayName = TEXT("BTR-4 TURRET");
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
    DriveForce = 1850000.0f;
    RollingBrakeForce = 650000.0f;
    HandbrakeForce = 1800000.0f;
    LateralGrip = 18000.0f;
    SteeringTorque = 170000000.0f;
    AeroDrag = 0.30f;
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
    bool bUsingBTR4 = false;
    if (Chassis)
    {
        if (UStaticMesh* ProductionBTR4 = LoadObject<UStaticMesh>(nullptr,
            TEXT("/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus.SM_BTR4_Bucephalus")))
        {
            // The source is already close to the real BTR-4E dimensions. Fit it lightly, then put
            // the wheel bottoms on the same ground plane as the authoritative 8-wheel suspension.
            bUsingBTR4 = ApplyFittedBTRMesh(Chassis, ProductionBTR4, FVector(776.0f, 293.0f, 300.0f), -98.0f);
        }
    }

    if (bUsingBTR4)
    {
        UStaticMeshComponent* ProxyParts[] =
        {
            UpperHull.Get(), NoseArmor.Get(), RearArmor.Get(),
            WheelExtraFL.Get(), WheelExtraFR.Get(), WheelExtraRL.Get(), WheelExtraRR.Get(),
            DriverDoor.Get(), PassengerDoor.Get(), FrontBumper.Get(), RearBumper.Get()
        };
        for (UStaticMeshComponent* Component : ProxyParts)
        {
            DisableVisualProxy(Component);
        }
        for (UStaticMeshComponent* Wheel : WheelVisuals)
        {
            DisableVisualProxy(Wheel);
        }

        // Production shell owns all rendered geometry. Gameplay physics remains exclusively on PhysicsBody,
        // while obsolete blockout turret meshes become fully inert instead of merely invisible obstacles.
        DisableVisualProxy(TurretBaseMesh);
        DisableVisualProxy(BarrelMesh);

        UE_LOG(LogTemp, Display, TEXT("BTR gameplay vehicle uses production BTR-4 Bucephalus visual shell; visual proxies disabled."));
    }
    else if (Chassis)
    {
        Chassis->SetRelativeScale3D(FVector(6.15f, 2.45f, 0.72f));
    }

    InteriorCamera->SetRelativeLocation(bUsingBTR4 ? FVector(145.0f, -58.0f, 112.0f) : FVector(130.0f, -52.0f, 105.0f));
    ThirdPersonSpringArm->TargetArmLength = bUsingBTR4 ? 900.0f : 820.0f;
    ThirdPersonSpringArm->SetRelativeLocation(bUsingBTR4 ? FVector(-110.0f, 0.0f, 245.0f) : FVector(-80.0f, 0.0f, 220.0f));
}
