#include "OCContentReadinessPass19Subsystem.h"

#include "OCAntiArmorLauncher.h"
#include "OCGameMode.h"
#include "OCWeaponBase.h"
#include "OCWeaponVariants.h"

#include "Components/MeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"

namespace
{
    constexpr float ValidationDelaySeconds = 7.0f;
    constexpr uint32 AllRequiredRackWeaponClassesMask = (1u << 11) - 1u;

    const FName RuntimeBaseRackTag(TEXT("OC_RuntimeBaseWeaponRack"));
    const FName ProductionWeaponVisualTag(TEXT("OC_ProductionWeaponVisual"));
    const FName RealFallbackWeaponVisualTag(TEXT("OC_RealFallbackWeaponVisual"));

    bool WeaponHasVisibleTaggedMesh(const AOCWeaponBase* Weapon, const FName Tag)
    {
        if (!Weapon) return false;
        TInlineComponentArray<UMeshComponent*> Components;
        Weapon->GetComponents(Components);
        for (const UMeshComponent* Component : Components)
        {
            if (Component && Component->ComponentHasTag(Tag) && Component->IsVisible()) return true;
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
}

bool UOCContentReadinessPass19Subsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCContentReadinessPass19Subsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (InWorld.GetNetMode() == NM_DedicatedServer) return;
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle Timer;
    InWorld.GetTimerManager().SetTimer(
        Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) ValidatePlayableWeaponSet(*World);
        }),
        ValidationDelaySeconds,
        false);
}

void UOCContentReadinessPass19Subsystem::ValidatePlayableWeaponSet(UWorld& World)
{
    int32 RackWeaponCount = 0;
    int32 ExactProductionCount = 0;
    int32 RealFallbackCount = 0;
    int32 PrimitiveOnlyCount = 0;
    uint32 RackWeaponClassMask = 0u;

    for (TActorIterator<AOCWeaponBase> It(&World); It; ++It)
    {
        AOCWeaponBase* Weapon = *It;
        if (!Weapon || !Weapon->ActorHasTag(RuntimeBaseRackTag)) continue;

        ++RackWeaponCount;
        RackWeaponClassMask |= RequiredRackWeaponClassBit(Weapon);

        const bool bExactProduction = WeaponHasVisibleTaggedMesh(Weapon, ProductionWeaponVisualTag);
        const bool bRealFallback = WeaponHasVisibleTaggedMesh(Weapon, RealFallbackWeaponVisualTag);

        if (bExactProduction)
        {
            ++ExactProductionCount;
        }
        else if (bRealFallback)
        {
            ++RealFallbackCount;

            // The strict Pass 7 exact-art validator is allowed to fail, but it must not make a legitimate
            // real-mesh fallback disappear from a focused gameplay/FPS recovery run.
            Weapon->SetActorHiddenInGame(false);
            Weapon->SetActorEnableCollision(true);
        }
        else
        {
            ++PrimitiveOnlyCount;
        }
    }

    const bool bAllRequiredClassesPresent = RackWeaponClassMask == AllRequiredRackWeaponClassesMask;
    const bool bPlayable = RackWeaponCount >= 11 && bAllRequiredClassesPresent && PrimitiveOnlyCount == 0 &&
        (ExactProductionCount + RealFallbackCount) == RackWeaponCount;

    if (bPlayable)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS19_PLAYABLE_WEAPON_SET_READY rackWeapons=%d exact=%d realFallback=%d primitiveOnly=%d requiredClasses=11/11 classMask=0x%X"),
            RackWeaponCount,
            ExactProductionCount,
            RealFallbackCount,
            PrimitiveOnlyCount,
            RackWeaponClassMask);
    }
    else
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS19_PLAYABLE_WEAPON_SET_FAIL rackWeapons=%d exact=%d realFallback=%d primitiveOnly=%d requiredClassesComplete=%d classMask=0x%X expectedMask=0x%X"),
            RackWeaponCount,
            ExactProductionCount,
            RealFallbackCount,
            PrimitiveOnlyCount,
            bAllRequiredClassesPresent ? 1 : 0,
            RackWeaponClassMask,
            AllRequiredRackWeaponClassesMask);
    }
}
