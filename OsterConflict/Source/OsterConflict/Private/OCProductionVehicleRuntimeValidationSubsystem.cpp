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

        if (GunTruck->IsA<AOCHMMWVGunTruck>())
        {
            ++HMMWVGunTruckCount;
            if (ActorUsesMesh(GunTruck, HMMWV)) ++HMMWVGunTrucksUsingHMMWV;
        }
        else
        {
            ++PickupGunTruckCount;
            if (ActorUsesMesh(GunTruck, Pickup)) ++PickupGunTrucksUsingPickup;
        }

        if (GunTruckUsesProductionM2(GunTruck, M2)) ++GunTrucksUsingM2;
    }

    int32 BTRCount = 0;
    int32 BTRsUsingProductionShell = 0;
    for (TActorIterator<AOCBTR> It(&World); It; ++It)
    {
        AOCBTR* BTR = *It;
        if (!BTR) continue;
        ++BTRCount;
        if (ActorUsesMesh(BTR, BTR4)) ++BTRsUsingProductionShell;
    }

    const bool bHMMWVRuntimePass = HMMWVGunTruckCount == 0 ||
        HMMWVGunTrucksUsingHMMWV == HMMWVGunTruckCount;
    const bool bPickupRuntimePass = PickupGunTruckCount == 0 ||
        PickupGunTrucksUsingPickup == PickupGunTruckCount;
    const bool bM2RuntimePass = GunTruckCount == 0 || GunTrucksUsingM2 == GunTruckCount;
    const bool bBTRRuntimePass = BTRCount == 0 || BTRsUsingProductionShell == BTRCount;

    const bool bPass = bHMMWVAssetReady && bPickupAssetReady && bM2AssetReady && bBTR4AssetReady &&
        bHMMWVRuntimePass && bPickupRuntimePass && bM2RuntimePass && bBTRRuntimePass;

    if (bPass)
    {
        UE_LOG(LogTemp, Display,
            TEXT("Production vehicle validation PASS: assets HMMWV/Pickup/M2/BTR4 ready; hmmwvTrucks=%d hmmwvShell=%d; pickupTrucks=%d pickupShell=%d; m2=%d/%d; btrs=%d productionShell=%d."),
            HMMWVGunTruckCount, HMMWVGunTrucksUsingHMMWV,
            PickupGunTruckCount, PickupGunTrucksUsingPickup,
            GunTrucksUsingM2, GunTruckCount,
            BTRCount, BTRsUsingProductionShell);
        return;
    }

    UE_LOG(LogTemp, Warning,
        TEXT("Production vehicle validation FAILED: assetReady HMMWV=%d Pickup=%d M2=%d BTR4=%d; hmmwvTrucks=%d hmmwvShell=%d; pickupTrucks=%d pickupShell=%d; m2=%d/%d; btrs=%d productionShell=%d. Distinct production vehicle identities must not silently substitute each other."),
        bHMMWVAssetReady ? 1 : 0, bPickupAssetReady ? 1 : 0, bM2AssetReady ? 1 : 0, bBTR4AssetReady ? 1 : 0,
        HMMWVGunTruckCount, HMMWVGunTrucksUsingHMMWV,
        PickupGunTruckCount, PickupGunTrucksUsingPickup,
        GunTrucksUsingM2, GunTruckCount,
        BTRCount, BTRsUsingProductionShell);
}
