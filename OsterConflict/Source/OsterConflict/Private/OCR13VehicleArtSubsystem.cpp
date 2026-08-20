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
#include "GameFramework/PlayerController.h"

namespace
{
    constexpr float ImportedWheelContactBelowBodyCm = 32.0f;

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
        if (Cast<AOCPickupGunTruck>(Vehicle))
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
                    TEXT("/Game/VehicleVarietyPack/Meshes/SM_SportsCar.SM_SportsCar"));
            case EOCCivilianVehicleStyle::BoxTruck:
                return LoadObject<UStaticMesh>(nullptr,
                    TEXT("/Game/VehicleVarietyPack/Meshes/SM_Truck_Box.SM_Truck_Box"));
            case EOCCivilianVehicleStyle::Wagon:
            default:
                return LoadObject<UStaticMesh>(nullptr,
                    TEXT("/Game/VehicleVarietyPack/Meshes/SM_SUV.SM_SUV"));
            }
        }

        return nullptr;
    }

    float VisualRoadContactZ(const UBoxComponent* PhysicsBody)
    {
        // R13.6 shares the exact same contact plane as VehicleGameplayRepair. Keeping cockpit eye height and visual
        // chassis grounding on one contract prevents the repaired pickup from being raised while its camera stays low.
        return PhysicsBody
            ? -PhysicsBody->GetUnscaledBoxExtent().Z - ImportedWheelContactBelowBodyCm
            : -80.0f;
    }

    float CockpitZFromRoadEye(const UBoxComponent* PhysicsBody, const float DesiredEyeHeightAboveRoadCm)
    {
        return VisualRoadContactZ(PhysicsBody) + DesiredEyeHeightAboveRoadCm;
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

        // Imported meshes already contain visible wheels. Align their visual bottom with the road-contact plane instead
        // of centering the mesh inside the suspended physics body, which made cars appear to hover or sink.
        const float ScaledMeshBottom = (Bounds.Origin.Z - Bounds.BoxExtent.Z) * UniformScale;
        const float DesiredVisualBottom = VisualRoadContactZ(PhysicsBody);
        const FVector GroundedLocation(
            -Bounds.Origin.X * UniformScale,
            -Bounds.Origin.Y * UniformScale,
            DesiredVisualBottom - ScaledMeshBottom);

        Chassis->SetStaticMesh(Mesh);
        Chassis->SetRelativeRotation(FRotator::ZeroRotator);
        Chassis->SetRelativeScale3D(FVector(UniformScale));
        Chassis->SetRelativeLocation(GroundedLocation);
        Chassis->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Chassis->SetHiddenInGame(false, true);
        Chassis->SetVisibility(true, true);
    }

    void RepairDriverCockpit(AOCVehicleBase* Vehicle)
    {
        if (!Vehicle || Cast<AOCBTR>(Vehicle)) return;

        UCameraComponent* InteriorCamera = FindCameraComponent(Vehicle, TEXT("InteriorCamera"));
        if (!InteriorCamera) return;
        UBoxComponent* PhysicsBody = FindPhysicsBody(Vehicle);

        // X/Y remain style-specific because imported cabin positions differ. Z is a human eye height measured from
        // the same repaired road-contact plane used to ground the visible chassis.
        if (const AOCCivilianVehicle* Civilian = Cast<AOCCivilianVehicle>(Vehicle))
        {
            switch (Civilian->GetVehicleStyle())
            {
            case EOCCivilianVehicleStyle::Sedan:
                InteriorCamera->SetRelativeLocation(FVector(
                    -48.0f, -42.0f, CockpitZFromRoadEye(PhysicsBody, 122.0f)));
                break;
            case EOCCivilianVehicleStyle::Hatchback:
                InteriorCamera->SetRelativeLocation(FVector(
                    18.0f, -41.0f, CockpitZFromRoadEye(PhysicsBody, 128.0f)));
                break;
            case EOCCivilianVehicleStyle::BoxTruck:
                InteriorCamera->SetRelativeLocation(FVector(
                    105.0f, -48.0f, CockpitZFromRoadEye(PhysicsBody, 218.0f)));
                break;
            case EOCCivilianVehicleStyle::Wagon:
            default:
                InteriorCamera->SetRelativeLocation(FVector(
                    22.0f, -43.0f, CockpitZFromRoadEye(PhysicsBody, 134.0f)));
                break;
            }
        }
        else if (Cast<AOCPickupGunTruck>(Vehicle))
        {
            InteriorCamera->SetRelativeLocation(FVector(
                24.0f, -43.0f, CockpitZFromRoadEye(PhysicsBody, 138.0f)));
        }
        else
        {
            InteriorCamera->SetRelativeLocation(FVector(
                30.0f, -43.0f, CockpitZFromRoadEye(PhysicsBody, 130.0f)));
        }
        InteriorCamera->SetFieldOfView(82.0f);
        InteriorCamera->SetRelativeRotation(FRotator::ZeroRotator);

        UStaticMeshComponent* Chassis = FindStaticMeshComponent(Vehicle, TEXT("Chassis"));
        const bool bUsesImportedRoadMesh = Chassis && Chassis->GetStaticMesh() &&
            !Chassis->GetStaticMesh()->GetPathName().StartsWith(TEXT("/Engine/BasicShapes/"));

        const bool bCockpitView = InteriorCamera->IsActive();
        const FName CockpitNames[] = { TEXT("Dashboard"), TEXT("SteeringWheel"), TEXT("Windshield") };
        for (const FName Name : CockpitNames)
        {
            if (UStaticMeshComponent* Component = FindStaticMeshComponent(Vehicle, Name))
            {
                const bool bShowProxyCockpit = bCockpitView && !bUsesImportedRoadMesh && Name != TEXT("Windshield");
                Component->SetHiddenInGame(!bShowProxyCockpit, true);
                Component->SetVisibility(bShowProxyCockpit, true);
                Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            }
        }
    }

    void RepairStaleVehicleView(UWorld* World)
    {
        if (!World) return;
        APlayerController* PC = World->GetFirstPlayerController();
        if (!PC || !PC->IsLocalController()) return;

        APawn* ControlledPawn = PC->GetPawn();
        if (!ControlledPawn || Cast<AOCVehicleBase>(ControlledPawn)) return;

        // Possession should normally retarget the camera automatically. Source listen-server testing exposed a case
        // where the BoxTruck interior camera remained the view target after the character had already been possessed.
        if (Cast<AOCVehicleBase>(PC->GetViewTarget()))
        {
            PC->SetViewTarget(ControlledPawn);
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

    // Camera ownership is cheap to validate every frame and avoids a visible one-scan delay after leaving a vehicle.
    RepairStaleVehicleView(World);

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

    // A real BTR/APC asset has not been selected yet. Keep the gameplay proxy until one is integrated.
    if (Cast<AOCBTR>(Vehicle))
    {
        ProcessedVehicles.Add(Vehicle);
        return;
    }

    UStaticMesh* Mesh = MeshForVehicle(Vehicle);
    if (!Mesh) return;

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
