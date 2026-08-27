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
    FQuat ResolveLongAxisToForward(const FVector& NativeSize)
    {
        FVector NativeForward = FVector::ForwardVector;
        if (NativeSize.Y >= NativeSize.X && NativeSize.Y >= NativeSize.Z)
        {
            NativeForward = FVector::RightVector;
        }
        else if (NativeSize.Z >= NativeSize.X && NativeSize.Z >= NativeSize.Y)
        {
            NativeForward = FVector::UpVector;
        }
        return FQuat::FindBetweenNormals(NativeForward, FVector::ForwardVector);
    }

    bool ApplyProportionalVehicleMesh(UStaticMeshComponent* Component, UStaticMesh* Mesh,
        float DesiredLengthCm, TOptional<float> GroundZCm)
    {
        if (!Component || !Mesh) return false;
        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector NativeSize = Bounds.BoxExtent * 2.0f;
        const float NativeLength = FMath::Max3(NativeSize.X, NativeSize.Y, NativeSize.Z);
        if (NativeSize.X <= 1.0f || NativeSize.Y <= 1.0f || NativeSize.Z <= 1.0f || NativeLength <= 1.0f)
        {
            return false;
        }

        const float UniformScale = DesiredLengthCm / NativeLength;
        const FQuat AxisCorrection = ResolveLongAxisToForward(NativeSize);
        const FVector CorrectedOrigin = AxisCorrection.RotateVector(Bounds.Origin);
        const FVector CorrectedExtent = AxisCorrection.RotateVector(Bounds.BoxExtent).GetAbs();
        FVector Location = -CorrectedOrigin * UniformScale;
        if (GroundZCm.IsSet())
        {
            Location.Z = GroundZCm.GetValue() - (CorrectedOrigin.Z - CorrectedExtent.Z) * UniformScale;
        }

        Component->SetStaticMesh(Mesh);
        Component->SetRelativeRotation(AxisCorrection.Rotator());
        Component->SetRelativeScale3D(FVector(UniformScale));
        Component->SetRelativeLocation(Location);
        Component->EmptyOverrideMaterials();

        UE_LOG(LogTemp, Display,
            TEXT("PASS45_HMMWV_PROPORTIONAL_VISUAL_READY asset=%s native_cm=%s uniform_scale=%.4f desired_length_cm=%.1f axis_correction=%s nonuniform_stretch=0"),
            *Mesh->GetPathName(), *NativeSize.ToCompactString(), UniformScale, DesiredLengthCm,
            *AxisCorrection.Rotator().ToCompactString());
        return true;
    }

    UStaticMeshComponent* AddGroundedTurretVisual(AActor* Owner, USceneComponent* Parent,
        UStaticMesh* Mesh, float DesiredLengthCm, const FName ComponentName, const FName VisualTag)
    {
        if (!Owner || !Parent || !Mesh) return nullptr;
        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector NativeSize = Bounds.BoxExtent * 2.0f;
        const float NativeLength = FMath::Max3(NativeSize.X, NativeSize.Y, NativeSize.Z);
        if (NativeLength <= 1.0f) return nullptr;

        UStaticMeshComponent* Visual = NewObject<UStaticMeshComponent>(Owner, ComponentName);
        if (!Visual) return nullptr;

        const float UniformScale = DesiredLengthCm / NativeLength;
        const FQuat AxisCorrection = ResolveLongAxisToForward(NativeSize);
        const FVector CorrectedOrigin = AxisCorrection.RotateVector(Bounds.Origin);
        const FVector CorrectedExtent = AxisCorrection.RotateVector(Bounds.BoxExtent).GetAbs();

        FVector Location = -CorrectedOrigin * UniformScale;
        // Parent origin is the physical gun mount plane. Rest the mesh bottom on it instead of centering
        // the imported mesh through the roof/turret, which produced the visibly crooked M2 mount.
        Location.Z = -(CorrectedOrigin.Z - CorrectedExtent.Z) * UniformScale;

        Visual->SetupAttachment(Parent);
        Visual->SetStaticMesh(Mesh);
        Visual->SetRelativeRotation(AxisCorrection.Rotator());
        Visual->SetRelativeLocation(Location);
        Visual->SetRelativeScale3D(FVector(UniformScale));
        Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Visual->SetGenerateOverlapEvents(false);
        Visual->SetCanEverAffectNavigation(false);
        Visual->SetCastShadow(true);
        Visual->EmptyOverrideMaterials();
        Visual->ComponentTags.Add(VisualTag);
        Owner->AddInstanceComponent(Visual);
        Visual->RegisterComponent();

        UE_LOG(LogTemp, Display,
            TEXT("PASS45_M2_MOUNT_ALIGNMENT_READY native_cm=%s uniform_scale=%.4f bottom_on_mount=1 axis_correction=%s"),
            *NativeSize.ToCompactString(), UniformScale, *AxisCorrection.Rotator().ToCompactString());
        return Visual;
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

    // PASS45 item 27: the open HMMWV ring has full azimuth travel. The client-facing yaw limit
    // becomes effectively unbounded and the authoritative presentation is normalized each update.
    bContinuousTurretYaw = true;
    MaxTurretYaw = 180.0f;

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
            bUsingHMMWV = ApplyProportionalVehicleMesh(Chassis, HMMWV, 465.0f, TOptional<float>(-86.0f));
            bUsingProductionVehicle = bUsingHMMWV;
        }
    }

    if (!bUsingProductionVehicle && Chassis)
    {
        if (UStaticMesh* PickupMesh = LoadObject<UStaticMesh>(nullptr,
            TEXT("/Game/VehicleVarietyPack/Meshes/SM_Pickup.SM_Pickup")))
        {
            bUsingProductionVehicle = ApplyProportionalVehicleMesh(Chassis, PickupMesh, 485.0f, TOptional<float>());
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
            DisableVisualProxy(Component);
        }
        for (UStaticMeshComponent* Wheel : WheelVisuals)
        {
            DisableVisualProxy(Wheel);
        }
    }

    if (bUsingHMMWV && TurretPivot)
    {
        // TurretPivot is the ring yaw owner. BarrelPivot is the M2 pitch owner. GunnerCameraPivot is
        // also parented to TurretPivot, so gunner view and weapon cannot drift into separate hierarchies.
        TurretPivot->SetRelativeLocation(FVector(20.0f, 0.0f, 132.0f));
        if (BarrelPivot) BarrelPivot->SetRelativeLocation(FVector::ZeroVector);
        if (GunnerCameraPivot) GunnerCameraPivot->SetRelativeLocation(FVector(-24.0f, 0.0f, 62.0f));
    }

    bool bUsingMountedGunAsset = false;
    USceneComponent* M2Parent = BarrelPivot.Get();
    if (!M2Parent) M2Parent = TurretPivot.Get();
    if (UStaticMesh* M2 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Production/Weapons/M2/SM_M2_Browning.SM_M2_Browning")))
    {
        if (AddGroundedTurretVisual(this, M2Parent, M2, 165.0f,
            FName(TEXT("ProductionM2Browning")), FName(TEXT("OC_ProductionM2"))))
        {
            bUsingMountedGunAsset = true;
            if (MuzzlePoint) MuzzlePoint->SetRelativeLocation(FVector(82.5f, 0.0f, 18.0f));
        }
    }

    if (!bUsingMountedGunAsset)
    {
        if (UStaticMesh* RealMachineGunFallback = LoadObject<UStaticMesh>(nullptr,
            TEXT("/Game/R13/Weapons/machinegun.machinegun")))
        {
            if (AddGroundedTurretVisual(this, M2Parent, RealMachineGunFallback, 145.0f,
                FName(TEXT("RealMountedMachineGunFallback")), FName(TEXT("OC_RealMountedGunFallback"))))
            {
                bUsingMountedGunAsset = true;
                if (MuzzlePoint) MuzzlePoint->SetRelativeLocation(FVector(72.5f, 0.0f, 18.0f));
                UE_LOG(LogTemp, Warning,
                    TEXT("Exact M2 Browning asset unavailable; using real R13 machine-gun visual fallback."));
            }
        }
    }

    DisableVisualProxy(TurretBaseMesh);
    DisableVisualProxy(BarrelMesh);
    if (!bUsingMountedGunAsset)
    {
        UE_LOG(LogTemp, Error,
            TEXT("Gun truck mounted-gun visual missing: import SM_M2_Browning or restore the R13 machinegun fallback asset."));
    }

    InteriorCamera->SetRelativeLocation(bUsingHMMWV ? FVector(38.0f, -48.0f, 92.0f) : FVector(28.0f, -45.0f, 88.0f));
    InteriorCamera->SetFieldOfView(92.0f);
    DisableVisualProxy(Windshield);
    ThirdPersonSpringArm->TargetArmLength = bUsingHMMWV ? 660.0f : 620.0f;

    if (bUsingHMMWV)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_HMMWV_M2_RUNTIME_CORRECTION_READY proportional_vehicle=1 m2_grounded_mount=1 proxies_disabled=1"));
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_HMMWV_M2_HIERARCHY_READY ring_owner=TurretPivot gun_owner=BarrelPivot muzzle_owner=BarrelPivot camera_owner=GunnerCameraPivot continuous_yaw=1 hard_stop=0 authored_m2=%d primitive_turret_visible=0"),
            bUsingMountedGunAsset ? 1 : 0);
        // No separate authored shield asset is tracked today. Never resurrect a Cube as fake armour just to turn
        // a checkbox green; the shield remains an explicit content gap until a real asset is committed/imported.
        UE_LOG(LogTemp, Warning,
            TEXT("PASS45_HMMWV_M2_SHIELD_CONTENT_GAP separate_authored_shield=0 primitive_shield_fallback=0 ring_hierarchy_ready=1"));
    }
    else if (bUsingProductionVehicle)
    {
        UE_LOG(LogTemp, Display, TEXT("Pickup gun truck uses production pickup visual; blockout proxies disabled."));
    }
}
