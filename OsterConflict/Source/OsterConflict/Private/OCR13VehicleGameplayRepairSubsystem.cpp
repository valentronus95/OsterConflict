#include "OCR13VehicleGameplayRepairSubsystem.h"

#include "OCCharacter.h"
#include "OCPickupGunTruck.h"
#include "OCVehicleBase.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"

namespace
{
    constexpr float VehicleRepairScanIntervalSeconds = 0.15f;
    constexpr float ImportedWheelContactBelowBodyCm = 32.0f;
    constexpr float MountedMachineGunLengthCm = 190.0f;

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

    bool UsesImportedVehicleMesh(const UStaticMeshComponent* Chassis)
    {
        const UStaticMesh* Mesh = Chassis ? Chassis->GetStaticMesh() : nullptr;
        return Mesh && !Mesh->GetPathName().StartsWith(TEXT("/Engine/BasicShapes/"));
    }
}

bool UOCR13VehicleGameplayRepairSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13VehicleGameplayRepairSubsystem::Tick(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World || DeltaTime <= 0.0f) return;

    ScanAccumulator += DeltaTime;
    if (ScanAccumulator < VehicleRepairScanIntervalSeconds) return;
    ScanAccumulator = 0.0f;

    for (TActorIterator<AOCVehicleBase> It(World); It; ++It)
    {
        AOCVehicleBase* Vehicle = *It;
        if (Vehicle) RepairVehicle(Vehicle);
    }

    for (auto It = LastDriverByVehicle.CreateIterator(); It; ++It)
    {
        if (!It.Key().IsValid()) It.RemoveCurrent();
    }
    for (auto It = GroundingRepairedVehicles.CreateIterator(); It; ++It)
    {
        if (!It->IsValid()) It.RemoveCurrent();
    }
    for (auto It = MountedGunRepairedVehicles.CreateIterator(); It; ++It)
    {
        if (!It->IsValid()) It.RemoveCurrent();
    }
}

void UOCR13VehicleGameplayRepairSubsystem::RepairVehicle(AOCVehicleBase* Vehicle)
{
    RepairDriverTransition(Vehicle);

    const TWeakObjectPtr<AOCVehicleBase> VehicleKey(Vehicle);
    if (!GroundingRepairedVehicles.Contains(VehicleKey))
    {
        RepairImportedChassisGrounding(Vehicle);
    }
    if (Cast<AOCPickupGunTruck>(Vehicle) && !MountedGunRepairedVehicles.Contains(VehicleKey))
    {
        EnsurePickupMountedMachineGun(Vehicle);
    }
}

void UOCR13VehicleGameplayRepairSubsystem::RepairDriverTransition(AOCVehicleBase* Vehicle)
{
    if (!Vehicle) return;

    const TWeakObjectPtr<AOCVehicleBase> VehicleKey(Vehicle);
    const TWeakObjectPtr<AOCCharacter> CurrentDriver(Vehicle->GetDriverCharacter());
    const TWeakObjectPtr<AOCCharacter>* PreviousDriver = LastDriverByVehicle.Find(VehicleKey);
    const bool bDriverChanged = !PreviousDriver || *PreviousDriver != CurrentDriver;
    if (!bDriverChanged) return;

    LastDriverByVehicle.Add(VehicleKey, CurrentDriver);
    if (!CurrentDriver.IsValid()) return;

    // ExitDriverServer intentionally applies the handbrake. A later possession used to inherit that state until a
    // second input transition happened, which is why a re-entered car could require a short reverse nudge first.
    // Start every new driver possession from explicit neutral + released handbrake and wake the rigid body.
    if (Vehicle->HasAuthority())
    {
        Vehicle->SetAIDriveInputsServer(0.0f, 0.0f, false);
        if (UBoxComponent* PhysicsBody = FindPhysicsBody(Vehicle)) PhysicsBody->WakeAllRigidBodies();
        Vehicle->ForceNetUpdate();
    }

    if (Vehicle->IsLocallyControlled())
    {
        if (APlayerController* PC = Cast<APlayerController>(Vehicle->GetController()))
        {
            PC->FlushPressedKeys();
        }
    }
}

void UOCR13VehicleGameplayRepairSubsystem::RepairImportedChassisGrounding(AOCVehicleBase* Vehicle)
{
    if (!Vehicle) return;
    UStaticMeshComponent* Chassis = FindStaticMeshComponent(Vehicle, TEXT("Chassis"));
    UBoxComponent* PhysicsBody = FindPhysicsBody(Vehicle);
    if (!UsesImportedVehicleMesh(Chassis) || !PhysicsBody) return;

    UStaticMesh* Mesh = Chassis->GetStaticMesh();
    const FBoxSphereBounds Bounds = Mesh->GetBounds();
    const FVector Scale = Chassis->GetRelativeScale3D().GetAbs();
    if (Scale.Z <= KINDA_SMALL_NUMBER) return;

    // R13.1 placed the imported visual bottom 60 cm below the physics box. On the pickup this put a large part of
    // the wheels/body underground. The raycast suspension visually reads closer to ~32 cm below the box bottom.
    const float DesiredVisualBottom = -PhysicsBody->GetUnscaledBoxExtent().Z - ImportedWheelContactBelowBodyCm;
    const float ScaledMeshBottom = (Bounds.Origin.Z - Bounds.BoxExtent.Z) * Scale.Z;
    FVector RelativeLocation = Chassis->GetRelativeLocation();
    RelativeLocation.Z = DesiredVisualBottom - ScaledMeshBottom;
    Chassis->SetRelativeLocation(RelativeLocation);

    GroundingRepairedVehicles.Add(TWeakObjectPtr<AOCVehicleBase>(Vehicle));
    UE_LOG(LogTemp, Display, TEXT("R13.6 vehicle grounding repaired: %s visual bottom=%.1f."),
        *Vehicle->GetClass()->GetName(), DesiredVisualBottom);
}

void UOCR13VehicleGameplayRepairSubsystem::EnsurePickupMountedMachineGun(AOCVehicleBase* Vehicle)
{
    AOCPickupGunTruck* Pickup = Cast<AOCPickupGunTruck>(Vehicle);
    if (!Pickup) return;

    if (FindObjectFast<UStaticMeshComponent>(Pickup, TEXT("R13_PickupMountedMG")))
    {
        MountedGunRepairedVehicles.Add(TWeakObjectPtr<AOCVehicleBase>(Vehicle));
        return;
    }

    USceneComponent* BarrelPivot = FindObjectFast<USceneComponent>(Pickup, TEXT("BarrelPivot"));
    if (!BarrelPivot) return;

    UStaticMesh* MachineGunMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/R13/Weapons/machinegun.machinegun"));
    if (!MachineGunMesh) return;

    UStaticMeshComponent* MountedMG = NewObject<UStaticMeshComponent>(Pickup, TEXT("R13_PickupMountedMG"));
    if (!MountedMG) return;
    MountedMG->SetupAttachment(BarrelPivot);
    MountedMG->SetStaticMesh(MachineGunMesh);
    MountedMG->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    MountedMG->SetGenerateOverlapEvents(false);
    MountedMG->SetCastShadow(true);

    const FVector MeshSize = MachineGunMesh->GetBounds().BoxExtent * 2.0f;
    const float LongestAxis = FMath::Max3(MeshSize.X, MeshSize.Y, MeshSize.Z);
    const float UniformScale = LongestAxis > KINDA_SMALL_NUMBER
        ? FMath::Clamp(MountedMachineGunLengthCm / LongestAxis, 0.15f, 140.0f)
        : 1.0f;

    // Same orientation contract as the existing runtime LMG weapon art: imported fallback mesh becomes X-forward.
    MountedMG->SetRelativeRotation(FRotator(0.0f, 90.0f, 90.0f));
    MountedMG->SetRelativeLocation(FVector(78.0f, 0.0f, 8.0f));
    MountedMG->SetRelativeScale3D(FVector(UniformScale));
    Pickup->AddInstanceComponent(MountedMG);
    MountedMG->RegisterComponent();

    // The functional BarrelPivot/MuzzlePoint still drive aiming and firing. Hide only the old rectangular barrel art.
    if (UStaticMeshComponent* ProxyBarrel = FindStaticMeshComponent(Pickup, TEXT("BarrelMesh")))
    {
        ProxyBarrel->SetVisibility(false, true);
        ProxyBarrel->SetHiddenInGame(true, true);
    }

    MountedGunRepairedVehicles.Add(TWeakObjectPtr<AOCVehicleBase>(Vehicle));
    UE_LOG(LogTemp, Display, TEXT("R13.6 pickup mounted machine-gun art attached: %s."), *MachineGunMesh->GetName());
}

TStatId UOCR13VehicleGameplayRepairSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCR13VehicleGameplayRepairSubsystem, STATGROUP_Tickables);
}
