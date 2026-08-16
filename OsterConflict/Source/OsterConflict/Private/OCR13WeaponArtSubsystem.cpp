#include "OCR13WeaponArtSubsystem.h"

#include "OCWeaponBase.h"
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

    void ApplyImportedTransform(UStaticMeshComponent* MainMesh, EOCWeaponClass WeaponClass)
    {
        if (!MainMesh) return;

        MainMesh->SetRelativeLocation(FVector::ZeroVector);

        if (WeaponClass == EOCWeaponClass::AssaultRifle)
        {
            // Fab AK-47 is already authored for Unreal units. Keep real-world scale rather than the 100x
            // conversion used by the older metre-authored R13 placeholder meshes. Orientation will be QA-tuned
            // after the first runtime screenshot, but this gives us the real textured weapon immediately.
            MainMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
            MainMesh->SetRelativeScale3D(FVector(1.0f));
        }
        else
        {
            // Older R13 placeholder source geometry is metre-scale with Y-up.
            MainMesh->SetRelativeRotation(FRotator(0.0f, 90.0f, 90.0f));
            MainMesh->SetRelativeScale3D(FVector(100.0f));
        }
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
    if (ScanAccumulator < 0.35f) return;
    ScanAccumulator = 0.0f;

    for (TActorIterator<AOCWeaponBase> It(World); It; ++It)
    {
        AOCWeaponBase* Weapon = *It;
        if (!Weapon || ProcessedWeapons.Contains(Weapon)) continue;
        ApplyArt(Weapon);
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
            // Hide the old source-only receiver/barrel/stock primitive silhouette once real art exists.
            Component->SetVisibility(false, true);
            Component->SetHiddenInGame(true, true);
        }
    }

    if (!MainMesh) return;
    MainMesh->SetStaticMesh(ImportedMesh);
    MainMesh->SetVisibility(true, true);
    MainMesh->SetHiddenInGame(false, true);
    ApplyImportedTransform(MainMesh, WeaponClass);
    MainMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    ProcessedWeapons.Add(Weapon);
    UE_LOG(LogTemp, Display, TEXT("R13 weapon art applied: %s -> %s"),
        *Weapon->GetWeaponDisplayName(), *ImportedMesh->GetName());
}

TStatId UOCR13WeaponArtSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCR13WeaponArtSubsystem, STATGROUP_Tickables);
}
