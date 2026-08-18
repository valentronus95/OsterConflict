#include "OCR13WeaponArtSubsystem.h"

#include "OCWeaponBase.h"
#include "OCWeaponVariants.h"
#include "CollisionQueryParams.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"

namespace
{
    const TCHAR* MeshPathForWeapon(const AOCWeaponBase* Weapon)
    {
        if (!Weapon) return nullptr;

        if (Weapon->IsA(AOCWeapon_M14::StaticClass()))
        {
            return TEXT("/Game/R13/Weapons/Stein/M14/SKM_M14.SKM_M14");
        }
        if (Weapon->IsA(AOCWeapon_LeverAction::StaticClass()))
        {
            return TEXT("/Game/R13/Weapons/Stein/LeverAction/SKM_LeverAction.SKM_LeverAction");
        }
        if (Weapon->IsA(AOCWeapon_MAC10::StaticClass()))
        {
            return TEXT("/Game/R13/Weapons/Stein/Mac10/SKM_Mac10.SKM_Mac10");
        }
        if (Weapon->IsA(AOCWeapon_Tec9::StaticClass()))
        {
            return TEXT("/Game/R13/Weapons/Stein/Tec9/SKM_Tec9.SKM_Tec9");
        }

        switch (Weapon->GetWeaponClass())
        {
            case EOCWeaponClass::Pistol:
                return TEXT("/Game/R13/Weapons/Stein/1911/SKM_1911.SKM_1911");
            case EOCWeaponClass::SMG:
                return TEXT("/Game/R13/Weapons/Stein/MP5/SKM_MP5.SKM_MP5");
            case EOCWeaponClass::SniperRifle:
                return TEXT("/Game/R13/Weapons/Stein/M700/SKM_M700.SKM_M700");
            case EOCWeaponClass::Shotgun:
                return TEXT("/Game/R13/Weapons/shotgun.shotgun");
            case EOCWeaponClass::Launcher:
                return TEXT("/Game/R13/Weapons/rocketlauncherModern.rocketlauncherModern");
            case EOCWeaponClass::LMG:
                return TEXT("/Game/R13/Weapons/machinegun.machinegun");
            case EOCWeaponClass::AssaultRifle:
            default:
                // Keep the already-proven Fab AK presentation for the primary rifle. The Stein AK is imported and
                // ready for a later explicit variant instead of replacing a working asset merely because it exists.
                return TEXT("/Game/AK-47/Mesh/SM_AK-47.SM_AK-47");
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

    bool IsSteinMesh(const UStaticMeshComponent* MainMesh)
    {
        const UStaticMesh* Mesh = MainMesh ? MainMesh->GetStaticMesh() : nullptr;
        return Mesh && Mesh->GetName().StartsWith(TEXT("SKM_"));
    }

    void ApplyImportedTransform(UStaticMeshComponent* MainMesh, EOCWeaponClass WeaponClass, bool bWorldPickup)
    {
        if (!MainMesh) return;

        MainMesh->SetRelativeLocation(FVector::ZeroVector);

        if (IsSteinMesh(MainMesh))
        {
            // Stein FBX meshes are imported through UE's static-mesh FBX pipeline in project units. Keep authored
            // scale/orientation for equipped use; dropped copies are rolled onto their side.
            MainMesh->SetRelativeRotation(bWorldPickup
                ? FRotator(0.0f, 0.0f, 90.0f)
                : FRotator::ZeroRotator);
            MainMesh->SetRelativeScale3D(FVector(1.0f));
            return;
        }

        if (WeaponClass == EOCWeaponClass::AssaultRifle)
        {
            // The Fab AK long axis is Y while the project weapon attach convention is X-forward.
            MainMesh->SetRelativeRotation(bWorldPickup
                ? FRotator(0.0f, -90.0f, 90.0f)
                : FRotator(0.0f, -90.0f, 0.0f));
            MainMesh->SetRelativeScale3D(FVector(1.0f));
            return;
        }

        // Remaining Kenney fallback source geometry is metre-scale with Y-up.
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
    const TCHAR* MeshPath = MeshPathForWeapon(Weapon);
    UStaticMesh* ImportedMesh = MeshPath ? LoadObject<UStaticMesh>(nullptr, MeshPath) : nullptr;
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
        MainMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        return;
    }

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
