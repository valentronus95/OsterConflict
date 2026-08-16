#include "OCR13WeaponArtSubsystem.h"

#include "OCWeaponBase.h"
#include "CollisionQueryParams.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"

namespace
{
    const TCHAR* MeshPathForClass(EOCWeaponClass WeaponClass)
    {
        switch (WeaponClass)
        {
            case EOCWeaponClass::Pistol:       return TEXT("/Game/R13/Weapons/pistol.pistol");
            case EOCWeaponClass::SMG:          return TEXT("/Game/R13/Weapons/uzi.uzi");
            case EOCWeaponClass::SniperRifle:  return TEXT("/Game/R13/Weapons/sniper.sniper");
            case EOCWeaponClass::Shotgun:      return TEXT("/Game/R13/Weapons/shotgun.shotgun");
            case EOCWeaponClass::Launcher:     return TEXT("/Game/R13/Weapons/rocketlauncherModern.rocketlauncherModern");
            case EOCWeaponClass::LMG:          return TEXT("/Game/R13/Weapons/machinegun.machinegun");
            case EOCWeaponClass::AssaultRifle:
            default:                           return TEXT("/Game/AK-47/Mesh/SM_AK-47.SM_AK-47");
        }
    }

    UStaticMeshComponent* FindMainMesh(AOCWeaponBase* Weapon)
    {
        if (!Weapon) return nullptr;

        TArray<UStaticMeshComponent*> MeshComponents;
        Weapon->GetComponents<UStaticMeshComponent>(MeshComponents);
        for (UStaticMeshComponent* Component : MeshComponents)
        {
            if (Component && Component->GetFName() == TEXT("WeaponMesh"))
            {
                return Component;
            }
        }
        return nullptr;
    }

    void ApplyImportedTransform(UStaticMeshComponent* MainMesh, EOCWeaponClass WeaponClass, bool bWorldPickup)
    {
        if (!MainMesh) return;

        MainMesh->SetRelativeLocation(FVector::ZeroVector);

        if (WeaponClass == EOCWeaponClass::AssaultRifle)
        {
            // The Fab AK long axis is Y while the project weapon attach convention is X-forward.
            // Rotate the barrel into camera-forward space. A dropped AK is additionally rolled onto its side.
            MainMesh->SetRelativeRotation(bWorldPickup
                ? FRotator(0.0f, -90.0f, 90.0f)
                : FRotator(0.0f, -90.0f, 0.0f));
            MainMesh->SetRelativeScale3D(FVector(1.0f));
            return;
        }

        if (WeaponClass == EOCWeaponClass::Pistol)
        {
            // Remove the old 90-degree roll which made the temporary pistol stand vertically in first person.
            // Keep it deliberately smaller until a production pistol asset replaces this bridge mesh.
            MainMesh->SetRelativeRotation(bWorldPickup
                ? FRotator(0.0f, 90.0f, 90.0f)
                : FRotator(0.0f, 90.0f, 0.0f));
            MainMesh->SetRelativeScale3D(FVector(72.0f));
            return;
        }

        // Older R13 placeholder source geometry is metre-scale with Y-up.
        MainMesh->SetRelativeRotation(FRotator(0.0f, 90.0f, 90.0f));
        MainMesh->SetRelativeScale3D(FVector(100.0f));
    }
}

bool UOCR13WeaponArtSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13WeaponArtSubsystem::Tick(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World) return;

    ScanAccumulator += DeltaTime;
    if (ScanAccumulator < 0.20f) return;
    ScanAccumulator = 0.0f;

    for (TActorIterator<AOCWeaponBase> It(World); It; ++It)
    {
        AOCWeaponBase* Weapon = *It;
        if (!Weapon) continue;

        if (!ProcessedWeapons.Contains(Weapon))
        {
            ApplyArt(Weapon);
        }

        if (ProcessedWeapons.Contains(Weapon))
        {
            RepairPresentation(Weapon);
        }
    }
}

void UOCR13WeaponArtSubsystem::ApplyArt(AOCWeaponBase* Weapon)
{
    if (!Weapon) return;

    const EOCWeaponClass WeaponClass = Weapon->GetWeaponClass();
    UStaticMesh* ImportedMesh = LoadObject<UStaticMesh>(nullptr, MeshPathForClass(WeaponClass));
    if (!ImportedMesh)
    {
        // Do not mark it processed: hot-imported content can still be picked up on a later scan.
        return;
    }

    TArray<UStaticMeshComponent*> MeshComponents;
    Weapon->GetComponents<UStaticMeshComponent>(MeshComponents);
    UStaticMeshComponent* MainMesh = nullptr;
    for (UStaticMeshComponent* Component : MeshComponents)
    {
        if (!Component) continue;
        if (Component->GetFName() == TEXT("WeaponMesh"))
        {
            MainMesh = Component;
        }
        else
        {
            // Hide the old source-only receiver/barrel/stock primitive silhouette once imported art exists.
            Component->SetVisibility(false, true);
            Component->SetHiddenInGame(true, true);
            Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }

    if (!MainMesh) return;
    MainMesh->SetStaticMesh(ImportedMesh);
    MainMesh->SetVisibility(true, true);
    MainMesh->SetHiddenInGame(false, true);
    ApplyImportedTransform(MainMesh, WeaponClass, Weapon->IsWorldPickup());

    ProcessedWeapons.Add(Weapon);
    UE_LOG(LogTemp, Display, TEXT("R13 weapon art applied: %s -> %s"),
        *Weapon->GetWeaponDisplayName(), *ImportedMesh->GetName());
}

void UOCR13WeaponArtSubsystem::RepairPresentation(AOCWeaponBase* Weapon)
{
    if (!Weapon) return;

    UStaticMeshComponent* MainMesh = FindMainMesh(Weapon);
    if (!MainMesh || !MainMesh->GetStaticMesh()) return;

    const bool bWorldPickup = Weapon->IsWorldPickup();
    ApplyImportedTransform(MainMesh, Weapon->GetWeaponClass(), bWorldPickup);

    if (!bWorldPickup)
    {
        // Equipped/stored weapons must never block the owning character or camera.
        MainMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        return;
    }

    // World pickups need a visibility query so the interaction trace can hit them, but they are grounded
    // explicitly instead of letting a child mesh simulate physics independently from the replicated actor root.
    MainMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    MainMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    MainMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    MainMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    if (!Weapon->HasAuthority() || !GetWorld()) return;

    const FVector Current = Weapon->GetActorLocation();
    const FVector TraceStart = Current + FVector(0.0f, 0.0f, 80.0f);
    const FVector TraceEnd = Current - FVector(0.0f, 0.0f, 600.0f);

    FCollisionQueryParams Params(SCENE_QUERY_STAT(OCWeaponDropGrounding), false, Weapon);
    FHitResult Hit;
    if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params))
    {
        const float TargetZ = Hit.ImpactPoint.Z + 5.0f;
        if (!FMath::IsNearlyEqual(Current.Z, TargetZ, 1.5f))
        {
            Weapon->SetActorLocation(FVector(Current.X, Current.Y, TargetZ), false, nullptr, ETeleportType::TeleportPhysics);
        }

        // Do not preserve camera pitch/roll from the instant the item was dropped. That was the source of
        // rifles hanging diagonally in mid-air. Keep only heading; the mesh itself is rolled onto the ground.
        const FRotator CurrentRotation = Weapon->GetActorRotation();
        if (!FMath::IsNearlyZero(CurrentRotation.Pitch, 0.5f) || !FMath::IsNearlyZero(CurrentRotation.Roll, 0.5f))
        {
            Weapon->SetActorRotation(FRotator(0.0f, CurrentRotation.Yaw, 0.0f), ETeleportType::TeleportPhysics);
        }
    }
}

TStatId UOCR13WeaponArtSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCR13WeaponArtSubsystem, STATGROUP_Tickables);
}