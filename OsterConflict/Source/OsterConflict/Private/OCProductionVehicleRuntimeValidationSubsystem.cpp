#include "OCProductionVehicleRuntimeValidationSubsystem.h"

#include "OCBTR.h"
#include "OCGameMode.h"
#include "OCHMMWVGunTruck.h"
#include "OCPickupGunTruck.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "UObject/UObjectGlobals.h"

namespace
{
    constexpr float ValidationDelaySeconds = 6.25f;

    const TCHAR* HMMWVAssetPath = TEXT("/Game/Production/Vehicles/HMMWV/SM_HMMWV_UA.SM_HMMWV_UA");
    const TCHAR* PickupAssetPath = TEXT("/Game/VehicleVarietyPack/Meshes/SM_Pickup.SM_Pickup");
    const TCHAR* M2AssetPath = TEXT("/Game/Production/Weapons/M2/SM_M2_Browning.SM_M2_Browning");
    const TCHAR* BTR4AssetPath = TEXT("/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus.SM_BTR4_Bucephalus");
    const FName ProductionM2Tag(TEXT("OC_ProductionM2"));

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

    void QuarantineInvalidVehicle(AActor* Actor, const TCHAR* Identity, bool bBodyReady, bool bWeaponReady)
    {
        if (!Actor) return;

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
            TEXT("PASS7_PRODUCTION_VEHICLES_READY HMMWV=%d/%d M2=%d/%d BTR4=%d/%d pickup=%d/%d validation_owner=vehicles_only"),
            HMMWVGunTrucksUsingHMMWV, HMMWVGunTruckCount,
            GunTrucksUsingM2, GunTruckCount,
            BTRsUsingProductionShell, BTRCount,
            PickupGunTrucksUsingPickup, PickupGunTruckCount);
    }
    else
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS7_PRODUCTION_VEHICLE_RUNTIME_FAIL summary=1 assetReady_HMMWV=%d assetReady_Pickup=%d assetReady_M2=%d assetReady_BTR4=%d expectedFleet=%d HMMWV=%d/%d pickup=%d/%d M2=%d/%d BTR4=%d/%d validation_owner=vehicles_only"),
            bHMMWVAssetReady ? 1 : 0, bPickupAssetReady ? 1 : 0, bM2AssetReady ? 1 : 0, bBTR4AssetReady ? 1 : 0,
            bExpectedNormalFleetPresent ? 1 : 0,
            HMMWVGunTrucksUsingHMMWV, HMMWVGunTruckCount,
            PickupGunTrucksUsingPickup, PickupGunTruckCount,
            GunTrucksUsingM2, GunTruckCount,
            BTRsUsingProductionShell, BTRCount);
    }

    // Weapon-rack completeness is intentionally not checked here anymore.
    // UOCPass45WeaponCatalogSpawnSubsystem is the single current owner of the complete weapon catalog and exact-visual validation.
}
