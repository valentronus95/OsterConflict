#include "OCR13VehicleArtSubsystem.h"

#include "OCBTR.h"
#include "OCCivilianVehicle.h"
#include "OCPickupGunTruck.h"
#include "OCVehicleBase.h"

#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"

namespace
{
    UStaticMeshComponent* FindStaticMeshComponent(AActor* Actor, const FName Name)
    {
        if (!Actor) return nullptr;
        TInlineComponentArray<UStaticMeshComponent*> Components;
        Actor->GetComponents(Components);
        for (UStaticMeshComponent* Component : Components)
        {
            if (Component && Component->GetFName() == Name) return Component;
        }
        return nullptr;
    }

    UCameraComponent* FindCameraComponent(AActor* Actor, const FName Name)
    {
        if (!Actor) return nullptr;
        TInlineComponentArray<UCameraComponent*> Components;
        Actor->GetComponents(Components);
        for (UCameraComponent* Component : Components)
        {
            if (Component && Component->GetFName() == Name) return Component;
        }
        return nullptr;
    }

    UBoxComponent* FindPhysicsBody(AActor* Actor)
    {
        if (!Actor) return nullptr;
        TInlineComponentArray<UBoxComponent*> Components;
        Actor->GetComponents(Components);
        for (UBoxComponent* Component : Components)
        {
            if (Component && Component->GetFName() == TEXT("PhysicsBody")) return Component;
        }
        return nullptr;
    }

    bool ShouldHideCivilianProxyComponent(const FName Name)
    {
        static const TSet<FName> Hidden = {
            TEXT("Dashboard"), TEXT("SteeringWheel"), TEXT("Windshield"),
            TEXT("DriverDoor"), TEXT("PassengerDoor"), TEXT("FrontBumper"), TEXT("RearBumper"),
            TEXT("WheelFL"), TEXT("WheelFR"), TEXT("WheelRL"), TEXT("WheelRR")
        };
        return Hidden.Contains(Name);
    }

    bool ShouldHidePickupProxyComponent(const FName Name)
    {
        if (ShouldHideCivilianProxyComponent(Name)) return true;
        static const TSet<FName> Hidden = {
            TEXT("CabRoof"), TEXT("BedFloor"), TEXT("BedLeft"), TEXT("BedRight")
        };
        return Hidden.Contains(Name);
    }

    void HideProxyParts(AOCVehicleBase* Vehicle, bool bPickup)
    {
        if (!Vehicle) return;
        TInlineComponentArray<UStaticMeshComponent*> Components;
        Vehicle->GetComponents(Components);
        for (UStaticMeshComponent* Component : Components)
        {
            if (!Component || Component->GetFName() == TEXT("Chassis")) continue;
            const bool bHide = bPickup
                ? ShouldHidePickupProxyComponent(Component->GetFName())
                : ShouldHideCivilianProxyComponent(Component->GetFName());
            if (!bHide) continue;
            Component->SetVisibility(false, true);
            Component->SetHiddenInGame(true, true);
            Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }

    UStaticMesh* MeshForVehicle(AOCVehicleBase* Vehicle)
    {
        if (AOCPickupGunTruck* Pickup = Cast<AOCPickupGunTruck>(Vehicle))
        {
            return LoadObject<UStaticMesh>(nullptr,
                TEXT("/Game/VehicleVarietyPack/Meshes/SM_Pickup.SM_Pickup"));
        }

        if (AOCCivilianVehicle* Civilian = Cast<AOCCivilianVehicle>(Vehicle))
        {
            switch (Civilian->GetVehicleStyle())
            {
            case EOCCivilianVehicleStyle::Hatchback:
                return LoadObject<UStaticMesh>(nullptr,
                    TEXT("/Game/VehicleVarietyPack/Meshes/SM_Hatchback.SM_Hatchback"));
            case EOCCivilianVehicleStyle::Sedan:
                return LoadObject<UStaticMesh>(nullptr,
                    TEXT("/Game/VehicleVarietyPack/Meshes/SM_SUV.SM_SUV"));
            case EOCCivilianVehicleStyle::Wagon:
            default:
                return LoadObject<UStaticMesh>(nullptr,
                    TEXT("/Game/VehicleVarietyPack/Meshes/SM_SUV.SM_SUV"));
            }
        }

        return nullptr;
    }

    void FitMeshToPhysicsBody(UStaticMeshComponent* Chassis, UStaticMesh* Mesh, UBoxComponent* PhysicsBody)
    {
        if (!Chassis || !Mesh || !PhysicsBody) return;

        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector MeshSize = Bounds.BoxExtent * 2.0f;
        const FVector TargetSize = PhysicsBody->GetUnscaledBoxExtent() * 2.0f;
        if (MeshSize.X <= KINDA_SMALL_NUMBER || MeshSize.Y <= KINDA_SMALL_NUMBER) return;

        const float ScaleX = TargetSize.X / MeshSize.X;
        const float ScaleY = TargetSize.Y / MeshSize.Y;
        const float UniformScale = FMath::Clamp(FMath::Min(ScaleX, ScaleY), 0.20f, 5.0f);

        Chassis->SetStaticMesh(Mesh);
        Chassis->SetRelativeRotation(FRotator::ZeroRotator);
        Chassis->SetRelativeScale3D(FVector(UniformScale));
        Chassis->SetRelativeLocation(-Bounds.Origin * UniformScale);
        Chassis->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Chassis->SetHiddenInGame(false, true);
        Chassis->SetVisibility(true, true);
    }

    void RepairDriverCockpit(AOCVehicleBase* Vehicle)
    {
        if (!Vehicle || Cast<AOCBTR>(Vehicle)) return;

        UCameraComponent* InteriorCamera = FindCameraComponent(Vehicle, TEXT("InteriorCamera"));
        if (!InteriorCamera) return;

        // Place the camera at a driver's head position behind the authored dashboard rather than near the front of
        // the visual body. A slightly narrower FOV also stops the road-vehicle shell from reading like a bumper cam.
        InteriorCamera->SetRelativeLocation(FVector(34.0f, -43.0f, 70.0f));
        InteriorCamera->SetRelativeRotation(FRotator::ZeroRotator);
        InteriorCamera->SetFieldOfView(82.0f);

        const bool bCockpitView = InteriorCamera->IsActive();
        const FName CockpitNames[] = { TEXT("Dashboard"), TEXT("SteeringWheel") };
        for (const FName Name : CockpitNames)
        {
            if (UStaticMeshComponent* Component = FindStaticMeshComponent(Vehicle, Name))
            {
                // The imported road meshes provide the exterior. These two simple interior reference parts are only
                // visible from the driver's camera so first person has a dashboard/wheel instead of floating in air.
                Component->SetHiddenInGame(!bCockpitView, true);
                Component->SetVisibility(bCockpitView, true);
                Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            }
        }
    }
}

bool UOCR13VehicleArtSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13VehicleArtSubsystem::Tick(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World) return;

    ScanAccumulator += DeltaTime;
    if (ScanAccumulator < 0.20f) return;
    ScanAccumulator = 0.0f;

    for (TActorIterator<AOCVehicleBase> It(World); It; ++It)
    {
        AOCVehicleBase* Vehicle = *It;
        if (!Vehicle) continue;

        if (!ProcessedVehicles.Contains(Vehicle))
        {
            TryApplyVehicleArt(Vehicle);
        }

        RepairDriverCockpit(Vehicle);
    }
}

void UOCR13VehicleArtSubsystem::TryApplyVehicleArt(AOCVehicleBase* Vehicle)
{
    if (!Vehicle) return;

    // A real BTR/APC asset has not been selected yet. Do not disguise the APC as a civilian car just to make the
    // cube disappear; keep the current gameplay proxy until a correctly licensed military vehicle is integrated.
    if (Cast<AOCBTR>(Vehicle))
    {
        ProcessedVehicles.Add(Vehicle);
        return;
    }

    UStaticMesh* Mesh = MeshForVehicle(Vehicle);
    if (!Mesh)
    {
        return;
    }

    UStaticMeshComponent* Chassis = FindStaticMeshComponent(Vehicle, TEXT("Chassis"));
    UBoxComponent* PhysicsBody = FindPhysicsBody(Vehicle);
    if (!Chassis || !PhysicsBody) return;

    const bool bPickup = Cast<AOCPickupGunTruck>(Vehicle) != nullptr;
    FitMeshToPhysicsBody(Chassis, Mesh, PhysicsBody);
    HideProxyParts(Vehicle, bPickup);

    ProcessedVehicles.Add(Vehicle);
    UE_LOG(LogTemp, Display, TEXT("R13 vehicle art applied: %s -> %s"),
        *Vehicle->GetClass()->GetName(), *Mesh->GetName());
}

TStatId UOCR13VehicleArtSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCR13VehicleArtSubsystem, STATGROUP_Tickables);
}
