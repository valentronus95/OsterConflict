#include "OCBTR.h"

#include "OCCharacter.h"
#include "OCDamageTypes.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
    bool ApplyProportionalGroundedBTRMesh(UStaticMeshComponent* Component, UStaticMesh* Mesh,
        float DesiredLengthCm, float GroundZCm)
    {
        if (!Component || !Mesh) return false;
        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector NativeSize = Bounds.BoxExtent * 2.0f;
        const float NativeLength = FMath::Max3(NativeSize.X, NativeSize.Y, NativeSize.Z);
        if (NativeSize.X <= 1.0f || NativeSize.Y <= 1.0f || NativeSize.Z <= 1.0f || NativeLength <= 1.0f)
        {
            return false;
        }

        // PASS45 item 30: the canonical BTR source is authored with +X as the nose/forward axis.
        // Do not guess another longest axis or its sign at runtime. If import transposes that contract,
        // reject the production presentation instead of confidently driving a BTR backwards.
        const bool bCanonicalPositiveXForward =
            NativeSize.X >= NativeSize.Y && NativeSize.X >= NativeSize.Z;
        if (!bCanonicalPositiveXForward)
        {
            UE_LOG(LogTemp, Error,
                TEXT("PASS45_BTR4_FORWARD_AXIS_FAIL native_cm=%s expected_long_axis=X canonical_forward=+X production_visible=0"),
                *NativeSize.ToCompactString());
            return false;
        }

        const float UniformScale = DesiredLengthCm / NativeSize.X;
        const FQuat AxisCorrection = FQuat::Identity;
        const FVector CorrectedOrigin = Bounds.Origin;
        const FVector CorrectedExtent = Bounds.BoxExtent.GetAbs();

        FVector Location = -CorrectedOrigin * UniformScale;
        Location.Z = GroundZCm - (CorrectedOrigin.Z - CorrectedExtent.Z) * UniformScale;

        Component->SetStaticMesh(Mesh);
        Component->SetRelativeRotation(AxisCorrection.Rotator());
        Component->SetRelativeScale3D(FVector(UniformScale));
        Component->SetRelativeLocation(Location);
        Component->EmptyOverrideMaterials();

        UE_LOG(LogTemp, Display,
            TEXT("PASS45_BTR4_FORWARD_AXIS_READY canonical_forward=+X runtime_axis_correction=identity native_cm=%s"),
            *NativeSize.ToCompactString());
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_BTR4_PROPORTIONAL_VISUAL_READY native_cm=%s uniform_scale=%.4f desired_length_cm=%.1f axis_correction=%s nonuniform_stretch=0"),
            *NativeSize.ToCompactString(), UniformScale, DesiredLengthCm, *AxisCorrection.Rotator().ToCompactString());
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

    // Remote weapon station optic: unlike the HMMWV open-ring camera, the BTR operator looks
    // through a sensor that follows both the turret yaw hierarchy and the barrel pitch hierarchy.
    if (GunnerCameraPivot && BarrelPivot)
    {
        GunnerCameraPivot->SetupAttachment(BarrelPivot);
        GunnerCameraPivot->SetRelativeLocation(FVector(64.0f, -36.0f, 35.0f));
        GunnerCameraPivot->SetRelativeRotation(FRotator::ZeroRotator);
    }

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

void AOCBTR::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    AOCCharacter* CurrentGunner = GetGunnerCharacter();
    AOCCharacter* PreviousGunner = ActiveOpticGunner.Get();
    if (PreviousGunner != CurrentGunner)
    {
        if (PreviousGunner && PreviousGunner->IsLocallyControlled())
        {
            if (APlayerController* PC = Cast<APlayerController>(PreviousGunner->GetController()))
            {
                if (PC->PlayerCameraManager)
                {
                    PC->PlayerCameraManager->UnlockFOV();
                }
            }
        }
        ActiveOpticGunner = CurrentGunner;
    }

    if (CurrentGunner && CurrentGunner->IsLocallyControlled())
    {
        if (APlayerController* PC = Cast<APlayerController>(CurrentGunner->GetController()))
        {
            if (PC->PlayerCameraManager)
            {
                PC->PlayerCameraManager->SetFOV(BTRRemoteOpticFieldOfView);
            }
        }
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
            bUsingBTR4 = ApplyProportionalGroundedBTRMesh(Chassis, ProductionBTR4, 776.0f, -98.0f);
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

        DisableVisualProxy(TurretBaseMesh);
        DisableVisualProxy(BarrelMesh);

        UE_LOG(LogTemp, Display,
            TEXT("BTR gameplay vehicle uses production BTR-4 Bucephalus visual shell with preserved native proportions; visual proxies disabled."));

        const bool bRemoteOpticHierarchyReady = GunnerCameraPivot && BarrelPivot &&
            GunnerCameraPivot->GetAttachParent() == BarrelPivot;
        if (bRemoteOpticHierarchyReady)
        {
            UE_LOG(LogTemp, Display,
                TEXT("PASS45_BTR4_REMOTE_OPTIC_READY owner=GunnerCameraPivot parent=BarrelPivot follows_yaw=1 follows_pitch=1 locked_fov=%.1f open_ring_view=0"),
                BTRRemoteOpticFieldOfView);
        }
        else
        {
            UE_LOG(LogTemp, Error,
                TEXT("PASS45_BTR4_REMOTE_OPTIC_FAIL parent=BarrelPivot follows_yaw=0 follows_pitch=0"));
        }

        ValidateProductionBTR4MaterialState(TEXT("ApplyVehicleStyle"));
    }
    else if (Chassis)
    {
        Chassis->SetRelativeScale3D(FVector(6.15f, 2.45f, 0.72f));
    }

    InteriorCamera->SetRelativeLocation(bUsingBTR4 ? FVector(145.0f, -58.0f, 112.0f) : FVector(130.0f, -52.0f, 105.0f));
    ThirdPersonSpringArm->TargetArmLength = bUsingBTR4 ? 900.0f : 820.0f;
    ThirdPersonSpringArm->SetRelativeLocation(bUsingBTR4 ? FVector(-110.0f, 0.0f, 245.0f) : FVector(-80.0f, 0.0f, 220.0f));
}

void AOCBTR::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    ValidateProductionBTR4MaterialState(TEXT("PossessedBy"));
}

void AOCBTR::UnPossessed()
{
    Super::UnPossessed();
    ValidateProductionBTR4MaterialState(TEXT("UnPossessed"));
}

void AOCBTR::PawnClientRestart()
{
    Super::PawnClientRestart();
    ValidateProductionBTR4MaterialState(TEXT("PawnClientRestart"));
}

bool AOCBTR::ValidateProductionBTR4MaterialState(const TCHAR* Phase)
{
    if (!Chassis)
    {
        return false;
    }

    UStaticMesh* Mesh = Chassis->GetStaticMesh();
    const FString MeshPath = Mesh ? Mesh->GetPathName() : FString();
    if (!MeshPath.StartsWith(TEXT("/Game/Production/Vehicles/BTR4/")))
    {
        return false;
    }

    const int32 MaterialSlots = Chassis->GetNumMaterials();
    bool bAllAuthored = MaterialSlots > 0;
    FString InvalidSlots;

    for (int32 Slot = 0; Slot < MaterialSlots; ++Slot)
    {
        UMaterialInterface* Material = Chassis->GetMaterial(Slot);
        const FString MaterialPath = Material ? Material->GetPathName() : FString();
        const bool bDefaultOrPrimitive = !Material ||
            MaterialPath.Contains(TEXT("/Engine/EngineMaterials/DefaultMaterial")) ||
            MaterialPath.Contains(TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
        const bool bBTRAuthored = MaterialPath.StartsWith(TEXT("/Game/Production/Vehicles/BTR4/"));
        if (bDefaultOrPrimitive || !bBTRAuthored)
        {
            bAllAuthored = false;
            if (!InvalidSlots.IsEmpty()) InvalidSlots += TEXT(",");
            InvalidSlots += FString::Printf(TEXT("%d:%s"), Slot, Material ? *MaterialPath : TEXT("NULL"));
        }
    }

    const TCHAR* SafePhase = Phase ? Phase : TEXT("Unknown");
    if (bAllAuthored)
    {
        Chassis->SetVisibility(true, true);
        Chassis->SetHiddenInGame(false, true);
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_BTR4_MATERIAL_STATE_READY phase=%s slots=%d mesh=%s default_material=0 primitive_material=0"),
            SafePhase, MaterialSlots, *MeshPath);
        return true;
    }

    // Fail closed. A missing/DefaultMaterial production BTR is not allowed to become the familiar
    // bright-white vehicle after possession just because Unreal can render an emergency material.
    Chassis->SetVisibility(false, true);
    Chassis->SetHiddenInGame(true, true);
    UE_LOG(LogTemp, Error,
        TEXT("PASS45_BTR4_MATERIAL_STATE_FAIL phase=%s slots=%d invalid=%s mesh=%s production_visible=0"),
        SafePhase, MaterialSlots, *InvalidSlots, *MeshPath);
    return false;
}
