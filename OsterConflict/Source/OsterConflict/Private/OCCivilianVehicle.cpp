#include "OCCivilianVehicle.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Net/UnrealNetwork.h"

namespace
{
    UStaticMesh* LoadVehicleMesh(EOCCivilianVehicleStyle Style)
    {
        const TCHAR* Path = nullptr;
        switch (Style)
        {
        case EOCCivilianVehicleStyle::Hatchback:
            Path = TEXT("/Game/VehicleVarietyPack/Meshes/SM_Hatchback.SM_Hatchback");
            break;
        case EOCCivilianVehicleStyle::Sedan:
            // The recovered pack has no sedan body; SportsCar is the closest low passenger-car silhouette.
            Path = TEXT("/Game/VehicleVarietyPack/Meshes/SM_SportsCar.SM_SportsCar");
            break;
        case EOCCivilianVehicleStyle::Wagon:
        default:
            // The recovered SUV is closer to the prototype wagon footprint than the old stretched cube.
            Path = TEXT("/Game/VehicleVarietyPack/Meshes/SM_SUV.SM_SUV");
            break;
        }
        return Path ? LoadObject<UStaticMesh>(nullptr, Path) : nullptr;
    }

    void SetLegacyProxyVisible(UStaticMeshComponent* Component, bool bVisible)
    {
        if (!Component) return;
        Component->SetVisibility(bVisible, true);
        Component->SetHiddenInGame(!bVisible, true);
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

    switch (VehicleStyle)
    {
    case EOCCivilianVehicleStyle::Sedan:
        if (PhysicsBody) PhysicsBody->SetBoxExtent(FVector(228.0f, 90.0f, 47.0f));
        VehicleMassKg = 1380.0f;
        MaxForwardSpeedKmh = 142.0f;
        break;
    case EOCCivilianVehicleStyle::Hatchback:
        if (PhysicsBody) PhysicsBody->SetBoxExtent(FVector(198.0f, 88.0f, 49.0f));
        VehicleMassKg = 1260.0f;
        MaxForwardSpeedKmh = 128.0f;
        break;
    case EOCCivilianVehicleStyle::Wagon:
    default:
        if (PhysicsBody) PhysicsBody->SetBoxExtent(FVector(233.0f, 92.0f, 50.0f));
        VehicleMassKg = 1490.0f;
        MaxForwardSpeedKmh = 136.0f;
        break;
    }

    const bool bCanUseRecoveredVisual = GetNetMode() != NM_DedicatedServer;
    UStaticMesh* RecoveredMesh = bCanUseRecoveredVisual ? LoadVehicleMesh(VehicleStyle) : nullptr;
    if (RecoveredMesh)
    {
        Chassis->SetStaticMesh(RecoveredMesh);
        Chassis->SetRelativeLocation(FVector::ZeroVector);
        Chassis->SetRelativeRotation(FRotator::ZeroRotator);
        Chassis->SetRelativeScale3D(FVector(1.0f));

        // The imported body already contains the exterior silhouette. Hide the old cube/cylinder dressing,
        // while retaining the existing cameras, interaction and server-authoritative rigid-body physics.
        SetLegacyProxyVisible(Dashboard, false);
        SetLegacyProxyVisible(SteeringWheel, false);
        SetLegacyProxyVisible(Windshield, false);
        SetLegacyProxyVisible(DriverDoor, false);
        SetLegacyProxyVisible(PassengerDoor, false);
        SetLegacyProxyVisible(FrontBumper, false);
        SetLegacyProxyVisible(RearBumper, false);
        for (UStaticMeshComponent* Wheel : WheelVisuals)
        {
            SetLegacyProxyVisible(Wheel, false);
        }
    }
    else
    {
        // Keep the source-only fallback usable when the recovered content is unavailable.
        switch (VehicleStyle)
        {
        case EOCCivilianVehicleStyle::Sedan:
            Chassis->SetRelativeScale3D(FVector(4.55f, 1.80f, 0.54f));
            break;
        case EOCCivilianVehicleStyle::Hatchback:
            Chassis->SetRelativeScale3D(FVector(3.95f, 1.76f, 0.58f));
            break;
        case EOCCivilianVehicleStyle::Wagon:
        default:
            Chassis->SetRelativeScale3D(FVector(4.65f, 1.84f, 0.60f));
            break;
        }

        SetLegacyProxyVisible(Dashboard, true);
        SetLegacyProxyVisible(SteeringWheel, true);
        SetLegacyProxyVisible(Windshield, true);
        SetLegacyProxyVisible(DriverDoor, true);
        SetLegacyProxyVisible(PassengerDoor, true);
        SetLegacyProxyVisible(FrontBumper, true);
        SetLegacyProxyVisible(RearBumper, true);
        for (UStaticMeshComponent* Wheel : WheelVisuals)
        {
            SetLegacyProxyVisible(Wheel, true);
        }
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
