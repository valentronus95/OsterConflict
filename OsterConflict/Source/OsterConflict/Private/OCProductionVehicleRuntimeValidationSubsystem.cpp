#include "OCProductionVehicleRuntimeValidationSubsystem.h"

#include "OCAntiArmorLauncher.h"
#include "OCBTR.h"
#include "OCGameMode.h"
#include "OCHMMWVGunTruck.h"
#include "OCPickupGunTruck.h"
#include "OCWeaponBase.h"
#include "OCWeaponVariants.h"

#include "Components/MeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "UObject/UObjectGlobals.h"

namespace
{
    constexpr float ValidationDelaySeconds = 6.25f;
    constexpr uint32 AllRequiredRackWeaponClassesMask = (1u << 11) - 1u;

    const TCHAR* HMMWVAssetPath = TEXT("/Game/Production/Vehicles/HMMWV/SM_HMMWV_UA.SM_HMMWV_UA");
    const TCHAR* PickupAssetPath = TEXT("/Game/VehicleVarietyPack/Meshes/SM_Pickup.SM_Pickup");
    const TCHAR* M2AssetPath = TEXT("/Game/Production/Weapons/M2/SM_M2_Browning.SM_M2_Browning");
    const TCHAR* BTR4AssetPath = TEXT("/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus.SM_BTR4_Bucephalus");
    const FName ProductionM2Tag(TEXT("OC_ProductionM2"));
    const FName RuntimeBaseRackTag(TEXT("OC_RuntimeBaseWeaponRack"));
    const FName ProductionWeaponVisualTag(TEXT("OC_ProductionWeaponVisual"));
    const FName RealFallbackWeaponVisualTag(TEXT("OC_RealFallbackWeaponVisual"));

    bool HasUsableBounds(const UStaticMesh* Mesh)
    {
        if (!Mesh) return false;
        const FVector Size = Mesh->GetBounds().BoxExtent * 2.0f;
        return Size.X > 1.0f && Size.Y > 1.0f && Size.Z > 1.0f;
    }

    bool HasUsableProductionAsset(const UStaticMesh* Mesh)
    {
        return HasUsableBounds(Mesh) && Mesh->GetStaticMaterials().Num() > 0;
    }

    bool ActorUsesMesh(AActor* Actor, const UStaticMesh* ExpectedMesh)
    {
        if (!Actor || !ExpectedMesh) return false;

        TInlineComponentArray<UStaticMeshComponent*> Components;
        Actor->GetComponents(Components);
        for (const UStaticMeshComponent* Component : Components)
        {
            if (Component && Component->GetStaticMesh() == ExpectedMesh && Component->IsVisible())
            {
                return true;
            }
        }
        return false;
    }

    bool GunTruckUsesProductionM2(AActor* Actor, const UStaticMesh* ExpectedM2)
    {
        if (!Actor || !ExpectedM2) return false;

        TInlineComponentArray<UStaticMeshComponent*> Components;
        Actor->GetComponents(Components);
        for (const UStaticMeshComponent* Component : Components)
        {
            if (!Component || !Component->ComponentHasTag(ProductionM2Tag)) continue;
            if (Component->GetStaticMesh() == ExpectedM2 && Component->IsVisible()) return true;
        }
        return false;
    }

    bool WeaponUsesVisualTag(AOCWeaponBase* Weapon, const FName VisualTag)
    {
        if (!Weapon) return false;

        TInlineComponentArray<UMeshComponent*> Components;
        Weapon->GetComponents(Components);
        for (const UMeshComponent* Component : Components)
        {
            if (Component && Component->ComponentHasTag(VisualTag) && Component->IsVisible())
            {
                return true;
            }
        }
        return false;
    }

    uint32 RequiredRackWeaponClassBit(const AOCWeaponBase* Weapon)
    {
        if (!Weapon) return 0u;
        if (Weapon->IsA<AOCWeapon_AssaultRifle>()) return 1u << 0;
        if (Weapon->IsA<AOCWeapon_SMG>()) return 1u << 1;
        if (Weapon->IsA<AOCWeapon_Pistol>()) return 1u << 2;
        if (Weapon->IsA<AOCWeapon_Sniper>()) return 1u << 3;
        if (Weapon->IsA<AOCWeapon_Shotgun>()) return 1u << 4;
        if (Weapon->IsA<AOCWeapon_LMG>()) return 1u << 5;
        if (Weapon->IsA<AOCWeapon_M14>()) return 1u << 6;
        if (Weapon->IsA<AOCWeapon_Mac10>()) return 1u << 7;
        if (Weapon->IsA<AOCWeapon_Tec9>()) return 1u << 8;
        if (Weapon->IsA<AOCWeapon_LeverAction>()) return 1u << 9;
        if (Weapon->IsA<AOCAntiArmorLauncher>()) return 1u << 10;
        return 0u;
    }

    void QuarantineInvalidVehicle(AActor* Actor, const TCHAR* Identity, bool bBodyReady, bool bWeaponReady)
    {
        if (!Actor) return;

        // Vehicle exact-production identity remains a hard Gate G requirement. A failed body/turret must not
        // continue masquerading as the requested production vehicle.
        Actor->SetActorHiddenInGame(true);
        Actor->SetActorEnableCollision(false);

        UE_LOG(LogTemp, Error,
            TEXT("PASS7_PRODUCTION_VEHICLE_RUNTIME_FAIL identity=%s actor=%s bodyReady=%d weaponReady=%d location=%s"),
            Identity,
            *Actor->GetName(),
            bBodyReady ? 1 : 0,
            bWeaponReady ? 1 : 0,
            *Actor->GetActorLocation().ToCompactString());
    }
}

bool UOCProductionVehicleRuntimeValidationSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCProductionVehicleRuntimeValidationSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle Timer;
    InWorld.GetTimerManager().SetTimer(Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) ValidateProductionVehicles(*World);
        }), ValidationDelaySeconds, false);
}

void UOCProductionVehicleRuntimeValidationSubsystem::ValidateProductionVehicles(UWorld& World)
{
    UStaticMesh* HMMWV = LoadObject<UStaticMesh>(nullptr, HMMWVAssetPath);
    UStaticMesh* Pickup = LoadObject<UStaticMesh>(nullptr, PickupAssetPath);
    UStaticMesh* M2 = LoadObject<UStaticMesh>(nullptr, M2AssetPath);
    UStaticMesh* BTR4 = LoadObject<UStaticMesh>(nullptr, BTR4AssetPath);

    const bool bHMMWVAssetReady = HasUsableProductionAsset(HMMWV);
    const bool bPickupAssetReady = HasUsableProductionAsset(Pickup);
    const bool bM2AssetReady = HasUsableProductionAsset(M2);
    const bool bBTR4AssetReady = HasUsableProductionAsset(BTR4);

    int32 GunTruckCount = 0;
    int32 HMMWVGunTruckCount = 0;
    int32 HMMWVGunTrucksUsingHMMWV = 0;
    int32 PickupGunTruckCount = 0;
    int32 PickupGunTrucksUsingPickup = 0;
    int32 GunTrucksUsingM2 = 0;

    for (TActorIterator<AOCPickupGunTruck> It(&World); It; ++It)
    {
        AOCPickupGunTruck* GunTruck = *It;
        if (!GunTruck) continue;
        ++GunTruckCount;

        const bool bUsesM2 = GunTruckUsesProductionM2(GunTruck, M2);
        if (bUsesM2) ++GunTrucksUsingM2;

        if (GunTruck->IsA<AOCHMMWVGunTruck>())
        {
            ++HMMWVGunTruckCount;
            const bool bUsesHMMWV = ActorUsesMesh(GunTruck, HMMWV);
            if (bUsesHMMWV) ++HMMWVGunTrucksUsingHMMWV;
            if (!bUsesHMMWV || !bUsesM2)
            {
                QuarantineInvalidVehicle(GunTruck, TEXT("HMMWV_M2"), bUsesHMMWV, bUsesM2);
            }
        }
        else
        {
            ++PickupGunTruckCount;
            const bool bUsesPickup = ActorUsesMesh(GunTruck, Pickup);
            if (bUsesPickup) ++PickupGunTrucksUsingPickup;
            if (!bUsesPickup || !bUsesM2)
            {
                QuarantineInvalidVehicle(GunTruck, TEXT("PICKUP_M2"), bUsesPickup, bUsesM2);
            }
        }
    }

    int32 BTRCount = 0;
    int32 BTRsUsingProductionShell = 0;
    for (TActorIterator<AOCBTR> It(&World); It; ++It)
    {
        AOCBTR* BTR = *It;
        if (!BTR) continue;
        ++BTRCount;
        const bool bUsesBTR4 = ActorUsesMesh(BTR, BTR4);
        if (bUsesBTR4)
        {
            ++BTRsUsingProductionShell;
        }
        else
        {
            QuarantineInvalidVehicle(BTR, TEXT("BTR4"), false, true);
        }
    }

    // Normal gameplay seeds explicit HMMWV and BTR spawn points. Zero actors is therefore a failed runtime proof,
    // not a vacuous success. The optional production pickup remains valid when absent from current fleet balance.
    const bool bExpectedNormalFleetPresent = HMMWVGunTruckCount > 0 && BTRCount > 0 && GunTruckCount > 0;
    const bool bHMMWVRuntimePass = HMMWVGunTruckCount > 0 &&
        HMMWVGunTrucksUsingHMMWV == HMMWVGunTruckCount;
    const bool bPickupRuntimePass = PickupGunTruckCount == 0 ||
        (bPickupAssetReady && PickupGunTrucksUsingPickup == PickupGunTruckCount);
    const bool bM2RuntimePass = GunTruckCount > 0 && GunTrucksUsingM2 == GunTruckCount;
    const bool bBTRRuntimePass = BTRCount > 0 && BTRsUsingProductionShell == BTRCount;

    const bool bVehiclePass = bExpectedNormalFleetPresent && bHMMWVAssetReady && bM2AssetReady && bBTR4AssetReady &&
        bHMMWVRuntimePass && bPickupRuntimePass && bM2RuntimePass && bBTRRuntimePass;

    if (bVehiclePass)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS7_PRODUCTION_VEHICLES_READY HMMWV=%d/%d M2=%d/%d BTR4=%d/%d pickup=%d/%d"),
            HMMWVGunTrucksUsingHMMWV, HMMWVGunTruckCount,
            GunTrucksUsingM2, GunTruckCount,
            BTRsUsingProductionShell, BTRCount,
            PickupGunTrucksUsingPickup, PickupGunTruckCount);
    }
    else
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS7_PRODUCTION_VEHICLE_RUNTIME_FAIL summary=1 assetReady_HMMWV=%d assetReady_Pickup=%d assetReady_M2=%d assetReady_BTR4=%d expectedFleet=%d HMMWV=%d/%d pickup=%d/%d M2=%d/%d BTR4=%d/%d"),
            bHMMWVAssetReady ? 1 : 0, bPickupAssetReady ? 1 : 0, bM2AssetReady ? 1 : 0, bBTR4AssetReady ? 1 : 0,
            bExpectedNormalFleetPresent ? 1 : 0,
            HMMWVGunTrucksUsingHMMWV, HMMWVGunTruckCount,
            PickupGunTrucksUsingPickup, PickupGunTruckCount,
            GunTrucksUsingM2, GunTruckCount,
            BTRsUsingProductionShell, BTRCount);
    }

    int32 RackWeaponCount = 0;
    int32 RackWeaponsUsingExactProductionVisual = 0;
    int32 RackWeaponsUsingRealFallbackVisual = 0;
    uint32 RackWeaponClassMask = 0u;

    for (TActorIterator<AOCWeaponBase> It(&World); It; ++It)
    {
        AOCWeaponBase* Weapon = *It;
        if (!Weapon || !Weapon->ActorHasTag(RuntimeBaseRackTag)) continue;

        ++RackWeaponCount;
        RackWeaponClassMask |= RequiredRackWeaponClassBit(Weapon);

        const bool bUsesExactProduction = WeaponUsesVisualTag(Weapon, ProductionWeaponVisualTag);
        const bool bUsesRealFallback = !bUsesExactProduction && WeaponUsesVisualTag(Weapon, RealFallbackWeaponVisualTag);
        if (bUsesExactProduction)
        {
            ++RackWeaponsUsingExactProductionVisual;
        }
        else if (bUsesRealFallback)
        {
            ++RackWeaponsUsingRealFallbackVisual;
        }
        else
        {
            UE_LOG(LogTemp, Error,
                TEXT("PASS45_REQUIRED_AVAILABLE_WEAPON_RUNTIME_FAIL actor=%s class=%s reason=no_exact_or_real_fallback_visual location=%s validation_only=1 mutation=0"),
                *Weapon->GetName(),
                *Weapon->GetClass()->GetName(),
                *Weapon->GetActorLocation().ToCompactString());
        }
    }

    const bool bAllRequiredRackWeaponClassesPresent = RackWeaponClassMask == AllRequiredRackWeaponClassesMask;
    const int32 RackWeaponsUsingAllowedVisual =
        RackWeaponsUsingExactProductionVisual + RackWeaponsUsingRealFallbackVisual;
    const bool bWeaponRuntimePass = RackWeaponCount >= 11 && bAllRequiredRackWeaponClassesPresent &&
        RackWeaponsUsingAllowedVisual == RackWeaponCount;

    if (bWeaponRuntimePass)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_REQUIRED_AVAILABLE_WEAPONS_READY rackWeapons=%d exactProductionVisuals=%d realFallbackVisuals=%d requiredClasses=11/11 classMask=0x%X validation_only=1 mutation=0"),
            RackWeaponCount,
            RackWeaponsUsingExactProductionVisual,
            RackWeaponsUsingRealFallbackVisual,
            RackWeaponClassMask);

        if (RackWeaponsUsingRealFallbackVisual > 0)
        {
            UE_LOG(LogTemp, Warning,
                TEXT("PASS45_EXACT_WEAPON_CONTENT_GAP realFallbackWeapons=%d exactProductionReadyNotClaimed=1 gameplayVisualReady=1"),
                RackWeaponsUsingRealFallbackVisual);
        }
    }
    else
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_REQUIRED_AVAILABLE_WEAPON_RUNTIME_FAIL summary=1 rackWeapons=%d exactProductionVisuals=%d realFallbackVisuals=%d allowedVisuals=%d requiredClassesComplete=%d classMask=0x%X expectedMask=0x%X validation_only=1 mutation=0"),
            RackWeaponCount,
            RackWeaponsUsingExactProductionVisual,
            RackWeaponsUsingRealFallbackVisual,
            RackWeaponsUsingAllowedVisual,
            bAllRequiredRackWeaponClassesPresent ? 1 : 0,
            RackWeaponClassMask,
            AllRequiredRackWeaponClassesMask);
    }
}
