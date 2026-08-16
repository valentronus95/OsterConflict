#include "OCPickupGunTruck.h"

#include "OCDamageTypes.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AOCPickupGunTruck::AOCPickupGunTruck()
{
    TurretDisplayName = TEXT("MOUNTED MG");
    TurretDamage = 30.0f;
    TurretRoundsPerMinute = 680.0f;
    TurretRangeCm = 13000.0f;
    TurretSpreadDegrees = 0.72f;
    TurretMagazineSize = 120;
    StartingTurretReserveAmmo = 480;
    TurretReloadSeconds = 4.2f;
    TurretDamageTypeClass = UOCBallisticDamageType::StaticClass();

    VehicleMassKg = 2250.0f;
    DriveForce = 720000.0f;
    SteeringTorque = 91000000.0f;
    MaxForwardSpeedKmh = 118.0f;
    MaxVehicleHealth = 700.0f;
    WreckLifetimeSeconds = 28.0f;

    PhysicsBody->SetBoxExtent(FVector(245.0f, 98.0f, 52.0f));
    TurretPivot->SetRelativeLocation(FVector(-85.0f, 0.0f, 105.0f));

    CabRoof = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CabRoof"));
    BedFloor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BedFloor"));
    BedLeft = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BedLeft"));
    BedRight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BedRight"));
    UStaticMeshComponent* PickupParts[] = {CabRoof.Get(), BedFloor.Get(), BedLeft.Get(), BedRight.Get()};
    for (UStaticMeshComponent* C : PickupParts)
    {
        C->SetupAttachment(PhysicsBody);
        C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh.Succeeded())
    {
        CabRoof->SetStaticMesh(CubeMesh.Object);
        BedFloor->SetStaticMesh(CubeMesh.Object);
        BedLeft->SetStaticMesh(CubeMesh.Object);
        BedRight->SetStaticMesh(CubeMesh.Object);
    }

    CabRoof->SetRelativeLocation(FVector(90.0f, 0.0f, 88.0f));
    CabRoof->SetRelativeScale3D(FVector(1.15f, 1.55f, 0.10f));
    BedFloor->SetRelativeLocation(FVector(-95.0f, 0.0f, 36.0f));
    BedFloor->SetRelativeScale3D(FVector(1.45f, 1.62f, 0.08f));
    BedLeft->SetRelativeLocation(FVector(-95.0f, -88.0f, 61.0f));
    BedLeft->SetRelativeScale3D(FVector(1.45f, 0.08f, 0.42f));
    BedRight->SetRelativeLocation(FVector(-95.0f, 88.0f, 61.0f));
    BedRight->SetRelativeScale3D(FVector(1.45f, 0.08f, 0.42f));
}

void AOCPickupGunTruck::ApplyVehicleStyle()
{
    Chassis->SetRelativeScale3D(FVector(4.85f, 1.94f, 0.58f));
    InteriorCamera->SetRelativeLocation(FVector(82.0f, -45.0f, 79.0f));
    ThirdPersonSpringArm->TargetArmLength = 620.0f;
}
