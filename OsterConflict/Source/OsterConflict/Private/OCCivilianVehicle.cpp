#include "OCCivilianVehicle.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Net/UnrealNetwork.h"

namespace
{
    bool ApplyProductionVehicleMesh(UStaticMeshComponent* Chassis, const TCHAR* AssetPath, const FVector& DesiredSizeCm)
    {
        if (!Chassis) return false;
        UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, AssetPath);
        if (!Mesh) return false;

        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector NativeSize = Bounds.BoxExtent * 2.0f;
        if (NativeSize.X <= 1.0f || NativeSize.Y <= 1.0f || NativeSize.Z <= 1.0f) return false;

        const FVector Scale(
            DesiredSizeCm.X / NativeSize.X,
            DesiredSizeCm.Y / NativeSize.Y,
            DesiredSizeCm.Z / NativeSize.Z);

        Chassis->SetStaticMesh(Mesh);
        Chassis->SetRelativeRotation(FRotator::ZeroRotator);
        Chassis->SetRelativeScale3D(Scale);
        Chassis->SetRelativeLocation(-Bounds.Origin * Scale);
        Chassis->SetVisibility(true, true);
        Chassis->SetHiddenInGame(false, true);

        for (int32 MaterialIndex = 0; MaterialIndex < Chassis->GetNumMaterials(); ++MaterialIndex)
        {
            Chassis->SetMaterial(MaterialIndex, nullptr);
        }
        return true;
    }
}

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

    const TCHAR* ProductionAsset = nullptr;
    FVector DesiredSizeCm = FVector::ZeroVector;

    switch (VehicleStyle)
    {
    case EOCCivilianVehicleStyle::Sedan:
        ProductionAsset = TEXT("/Game/VehicleVarietyPack/Meshes/SM_SportsCar.SM_SportsCar");
        DesiredSizeCm = FVector(455.0f, 180.0f, 140.0f);
        if (PhysicsBody) PhysicsBody->SetBoxExtent(FVector(228.0f, 90.0f, 47.0f));
        VehicleMassKg = 1380.0f;
        MaxForwardSpeedKmh = 142.0f;
        break;
    case EOCCivilianVehicleStyle::Hatchback:
        ProductionAsset = TEXT("/Game/VehicleVarietyPack/Meshes/SM_Hatchback.SM_Hatchback");
        DesiredSizeCm = FVector(395.0f, 176.0f, 145.0f);
        if (PhysicsBody) PhysicsBody->SetBoxExtent(FVector(198.0f, 88.0f, 49.0f));
        VehicleMassKg = 1260.0f;
        MaxForwardSpeedKmh = 128.0f;
        break;
    case EOCCivilianVehicleStyle::Wagon:
    default:
        ProductionAsset = TEXT("/Game/VehicleVarietyPack/Meshes/SM_SUV.SM_SUV");
        DesiredSizeCm = FVector(465.0f, 184.0f, 155.0f);
        if (PhysicsBody) PhysicsBody->SetBoxExtent(FVector(233.0f, 92.0f, 50.0f));
        VehicleMassKg = 1490.0f;
        MaxForwardSpeedKmh = 136.0f;
        break;
    }

    const bool bUsingProductionMesh = ApplyProductionVehicleMesh(Chassis, ProductionAsset, DesiredSizeCm);
    if (bUsingProductionMesh)
    {
        // The imported mesh already contains its exterior body and wheels. Keep source-only
        // interior controls for the first-person cab, but remove duplicate exterior primitives.
        for (UStaticMeshComponent* Wheel : WheelVisuals)
        {
            if (Wheel) Wheel->SetVisibility(false, true);
        }
        if (DriverDoor) DriverDoor->SetVisibility(false, true);
        if (PassengerDoor) PassengerDoor->SetVisibility(false, true);
        if (FrontBumper) FrontBumper->SetVisibility(false, true);
        if (RearBumper) RearBumper->SetVisibility(false, true);
        if (Windshield) Windshield->SetVisibility(false, true);
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
