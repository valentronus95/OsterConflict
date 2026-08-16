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
            default:                           return TEXT("/Game/R13/Weapons/machinegun.machinegun");
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

    UStaticMesh* ImportedMesh = LoadObject<UStaticMesh>(nullptr, MeshPathForClass(Weapon->GetWeaponClass()));
    if (!ImportedMesh)
    {
        // Do not mark it processed: if assets are imported during an editor session, the next scan may pick them up.
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
            // R04/R11 generated receiver/barrel/stock primitives. Imported R13 mesh replaces the whole silhouette.
            Component->SetVisibility(false, true);
            Component->SetHiddenInGame(true, true);
        }
    }

    if (!MainMesh) return;
    MainMesh->SetStaticMesh(ImportedMesh);
    MainMesh->SetVisibility(true, true);
    MainMesh->SetHiddenInGame(false, true);
    MainMesh->SetRelativeLocation(FVector::ZeroVector);
    // Kenney source geometry is authored in metre-scale coordinates with Y-up. Scale to UE centimetres and rotate
    // the long weapon axis into the project X-forward convention. This is an art bridge; per-model tuning follows QA.
    MainMesh->SetRelativeRotation(FRotator(0.0f, 90.0f, 90.0f));
    MainMesh->SetRelativeScale3D(FVector(100.0f));
    MainMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    ProcessedWeapons.Add(Weapon);
    UE_LOG(LogTemp, Display, TEXT("R13 weapon art applied: %s -> %s"),
        *Weapon->GetWeaponDisplayName(), *ImportedMesh->GetName());
}

TStatId UOCR13WeaponArtSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCR13WeaponArtSubsystem, STATGROUP_Tickables);
}
