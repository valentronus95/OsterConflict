#include "OCPickupGunTruck.h"

#include "OCDamageTypes.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UObjectGlobals.h"

namespace
{
    bool ApplyFittedVehicleMesh(UStaticMeshComponent* Component, UStaticMesh* Mesh, const FVector& DesiredSizeCm)
    {
        if (!Component || !Mesh) return false;
        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector NativeSize = Bounds.BoxExtent * 2.0f;
        if (NativeSize.X <= 1.0f || NativeSize.Y <= 1.0f || NativeSize.Z <= 1.0f) return false;

        const FVector Scale(
            DesiredSizeCm.X / NativeSize.X,
            DesiredSizeCm.Y / NativeSize.Y,
            DesiredSizeCm.Z / NativeSize.Z);
        Component->SetStaticMesh(Mesh);
        Component->SetRelativeRotation(FRotator::ZeroRotator);
        Component->SetRelativeScale3D(Scale);
        Component->SetRelativeLocation(-Bounds.Origin * Scale);
        Component->EmptyOverrideMaterials();
        return true;
    }

    bool ApplyGroundedVehicleMesh(UStaticMeshComponent* Component, UStaticMesh* Mesh,
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
        Component->EmptyOverrideMaterials();
        return true;
    }

    UStaticMeshComponent* AddFittedTurretVisual(AActor* Owner, USceneComponent* Parent,
        UStaticMesh* Mesh, float DesiredLengthCm, const FName ComponentName, const FName VisualTag)
    {
        if (!Owner || !Parent || !Mesh) return nullptr;
        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector NativeSize = Bounds.BoxExtent * 2.0f;
        if (NativeSize.X <= 1.0f || NativeSize.Y <= 1.0f || NativeSize.Z <= 1.0f) return nullptr;

        UStaticMeshComponent* Visual = NewObject<UStaticMeshComponent>(Owner, ComponentName);
        if (!Visual) return nullptr;
        const float NativeLength = FMath::Max3(NativeSize.X, NativeSize.Y, NativeSize.Z);
        const float UniformScale = DesiredLengthCm / NativeLength;
        Visual->SetupAttachment(Parent);
        Visual->SetStaticMesh(Mesh);
        Visual->SetRelativeLocation(-Bounds.Origin * UniformScale);
        Visual->SetRelativeRotation(FRotator::ZeroRotator);
        Visual->SetRelativeScale3D(FVector(UniformScale));
        Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Visual->SetGenerateOverlapEvents(false);
        Visual->SetCanEverAffectNavigation(false);
        Visual->SetCastShadow(true);
        Visual->ComponentTags.Add(VisualTag);
        Owner->AddInstanceComponent(Visual);
        Visual->RegisterComponent();
        return Visual;
    }
}

AOCPickupGunTruck::AOCPickupGunTruck()
{
    TurretDisplayName = TEXT("M2 BROWNING .50");
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
    bool bUsingProductionVehicle = false;
    bool bUsingHMMWV = false;

    if (Chassis && ShouldUseHMMWVProductionVisual())
    {
        if (UStaticMesh* HMMWV = LoadObject<UStaticMesh>(nullptr,
            TEXT("/Game/Production/Vehicles/HMMWV/SM_HMMWV_UA.SM_HMMWV_UA")))
        {
            bUsingHMMWV = ApplyGroundedVehicleMesh(Chassis, HMMWV, FVector(465.0f, 216.0f, 275.0f), -86.0f);
            bUsingProductionVehicle = bUsingHMMWV;
        }
    }

    if (!bUsingProductionVehicle && Chassis)
    {
        if (UStaticMesh* PickupMesh = LoadObject<UStaticMesh>(nullptr,
            TEXT("/Game/VehicleVarietyPack/Meshes/SM_Pickup.SM_Pickup")))
        {
            bUsingProductionVehicle = ApplyFittedVehicleMesh(Chassis, PickupMesh, FVector(485.0f, 194.0f, 170.0f));
        }
    }

    if (!bUsingProductionVehicle && Chassis)
    {
        Chassis->SetRelativeScale3D(FVector(4.85f, 1.94f, 0.58f));
    }

    if (bUsingProductionVehicle)
    {
        UStaticMeshComponent* SourceOnlyPickupParts[] =
        {
            CabRoof.Get(), BedFloor.Get(), BedLeft.Get(), BedRight.Get(),
            DriverDoor.Get(), PassengerDoor.Get(), FrontBumper.Get(), RearBumper.Get()
        };
        for (UStaticMeshComponent* Component : SourceOnlyPickupParts)
        {
            if (Component) Component->SetVisibility(false, true);
        }
        for (UStaticMeshComponent* Wheel : WheelVisuals)
        {
            if (Wheel) Wheel->SetVisibility(false, true);
        }
    }

    if (bUsingHMMWV && TurretPivot)
    {
        TurretPivot->SetRelativeLocation(FVector(72.0f, 0.0f, 103.0f));
        if (BarrelPivot) BarrelPivot->SetRelativeLocation(FVector::ZeroVector);
    }

    bool bUsingMountedGunAsset = false;
    USceneComponent* M2Parent = BarrelPivot.Get();
    if (!M2Parent) M2Parent = TurretPivot.Get();
    if (UStaticMesh* M2 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Production/Weapons/M2/SM_M2_Browning.SM_M2_Browning")))
    {
        if (AddFittedTurretVisual(this, M2Parent, M2, 165.0f,
            FName(TEXT("ProductionM2Browning")), FName(TEXT("OC_ProductionM2"))))
        {
            bUsingMountedGunAsset = true;
            if (MuzzlePoint) MuzzlePoint->SetRelativeLocation(FVector(118.0f, 0.0f, 0.0f));
            UE_LOG(LogTemp, Display, TEXT("Gun truck uses production M2 Browning visual."));
        }
    }

    if (!bUsingMountedGunAsset)
    {
        if (UStaticMesh* RealMachineGunFallback = LoadObject<UStaticMesh>(nullptr,
            TEXT("/Game/R13/Weapons/machinegun.machinegun")))
        {
            if (AddFittedTurretVisual(this, M2Parent, RealMachineGunFallback, 145.0f,
                FName(TEXT("RealMountedMachineGunFallback")), FName(TEXT("OC_RealMountedGunFallback"))))
            {
                bUsingMountedGunAsset = true;
                if (MuzzlePoint) MuzzlePoint->SetRelativeLocation(FVector(108.0f, 0.0f, 0.0f));
                UE_LOG(LogTemp, Warning,
                    TEXT("Exact M2 Browning asset unavailable; using real R13 machine-gun visual fallback."));
            }
        }
    }

    // Never show the old primitive disc/bar turret in a runtime build. If both real gun assets are absent,
    // the mount stays visually empty and logs a hard content warning instead of pretending the proxy is a Browning.
    if (TurretBaseMesh) TurretBaseMesh->SetVisibility(false, true);
    if (BarrelMesh) BarrelMesh->SetVisibility(false, true);
    if (!bUsingMountedGunAsset)
    {
        UE_LOG(LogTemp, Error,
            TEXT("Gun truck mounted-gun visual missing: import SM_M2_Browning or restore the R13 machinegun fallback asset."));
    }

    InteriorCamera->SetRelativeLocation(bUsingHMMWV ? FVector(38.0f, -48.0f, 92.0f) : FVector(28.0f, -45.0f, 88.0f));
    InteriorCamera->SetFieldOfView(92.0f);

    if (Windshield)
    {
        Windshield->SetVisibility(false, true);
    }

    ThirdPersonSpringArm->TargetArmLength = bUsingHMMWV ? 660.0f : 620.0f;

    if (bUsingHMMWV)
    {
        UE_LOG(LogTemp, Display, TEXT("HMMWV gun truck uses Ukrainian HMMWV production visual."));
    }
    else if (bUsingProductionVehicle)
    {
        UE_LOG(LogTemp, Display, TEXT("Pickup gun truck uses production pickup visual."));
    }
}
