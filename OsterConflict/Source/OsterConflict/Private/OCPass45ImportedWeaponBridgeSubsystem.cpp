#include "OCPass45ImportedWeaponBridgeSubsystem.h"

#include "OCGameMode.h"
#include "OCPass45LocalAssetResolver.h"
#include "OCWeaponBase.h"

#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/StreamableManager.h"
#include "Engine/World.h"
#include "EngineUtils.h"

#include <type_traits>

namespace
{
    const FName ProductionVisualTag(TEXT("OC_ProductionWeaponVisual"));
    const FName RealFallbackComponentTag(TEXT("OC_RealFallbackWeaponVisual"));
    const FName LocalBridgeTag(TEXT("OC_PASS45_LOCAL_IMPORTED_WEAPON"));
    const FName LocalBridgePreloadPendingTag(TEXT("OC_PASS45_LOCAL_IMPORTED_WEAPON_PRELOAD_PENDING"));
    const FName LocalBridgeSkeletalGapTag(TEXT("OC_PASS45_LOCAL_IMPORTED_WEAPON_SKELETAL_GAP"));
    const FName LocalBridgeStaticGapTag(TEXT("OC_PASS45_LOCAL_IMPORTED_WEAPON_STATIC_GAP"));
    const FName LocalInboxBoundTag(TEXT("OC_LocalInboxWeaponBound"));
    const FName LocalInboxPreloadPendingTag(TEXT("OC_LocalInboxWeaponPreloadPending"));
    constexpr int32 FastRefreshPasses = 8;
    constexpr float SandboxWatchIntervalSeconds = 1.50f;

    struct FLocalWeaponQuery
    {
        TArray<FName> Roots;
        TArray<FString> Tokens;
        float DesiredLengthCm = 80.0f;
    };

    bool HasProductionVisual(const AOCWeaponBase& Weapon)
    {
        TArray<UPrimitiveComponent*> Components;
        Weapon.GetComponents<UPrimitiveComponent>(Components);
        for (const UPrimitiveComponent* Component : Components)
        {
            if (Component && Component->ComponentHasTag(ProductionVisualTag) && Component->IsVisible())
            {
                return true;
            }
        }
        return false;
    }

    void RetireTemporaryRealFallbacks(AOCWeaponBase& Weapon)
    {
        TArray<UActorComponent*> Components;
        Weapon.GetComponents(Components);
        for (UActorComponent* Component : Components)
        {
            if (Component && Component->ComponentHasTag(RealFallbackComponentTag))
            {
                Component->DestroyComponent();
            }
        }
    }

    int32 HideSourceProxyVisuals(AOCWeaponBase& Weapon)
    {
        TArray<UStaticMeshComponent*> Components;
        Weapon.GetComponents<UStaticMeshComponent>(Components);

        int32 HiddenCount = 0;
        for (UStaticMeshComponent* Component : Components)
        {
            if (!Component || Component->ComponentHasTag(ProductionVisualTag)
                || Component->ComponentHasTag(RealFallbackComponentTag))
            {
                continue;
            }

            if (Component->IsVisible())
            {
                ++HiddenCount;
            }

            // Keep the component alive because WeaponMesh remains the drop/physics authority. Only the old
            // source/proxy rendering is retired. Do not propagate visibility to children because the production
            // visual is attached below WeaponRoot and must remain visible.
            Component->SetVisibility(false, false);
            Component->SetHiddenInGame(true, false);
        }
        return HiddenCount;
    }

    bool ResolveQuery(const FString& DisplayName, FLocalWeaponQuery& Out)
    {
        const FName FabRoot(TEXT("/Game/Fab"));

        if (DisplayName.Equals(TEXT("M1911"), ESearchCase::IgnoreCase))
        {
            Out = {{ FName(TEXT("/Game/colt-m1911")), FabRoot }, { TEXT("1911"), TEXT("colt") }, 23.0f};
            return true;
        }
        if (DisplayName.Equals(TEXT("Remington 870"), ESearchCase::IgnoreCase))
        {
            Out = {{ FName(TEXT("/Game/shotgun")), FabRoot }, { TEXT("remington"), TEXT("870") }, 100.0f};
            return true;
        }
        if (DisplayName.Equals(TEXT("AK-47"), ESearchCase::IgnoreCase))
        {
            Out = {{ FName(TEXT("/Game/AK-47")), FabRoot }, { TEXT("ak-47"), TEXT("ak47") }, 88.0f};
            return true;
        }
        if (DisplayName.Equals(TEXT("MP5"), ESearchCase::IgnoreCase))
        {
            Out = {{ FName(TEXT("/Game/R13/Weapons/Stein/MP5")), FabRoot }, { TEXT("mp5") }, 68.0f};
            return true;
        }
        if (DisplayName.Equals(TEXT("M700"), ESearchCase::IgnoreCase))
        {
            Out = {{ FName(TEXT("/Game/R13/Weapons/Stein/M700")), FabRoot }, { TEXT("m700") }, 112.0f};
            return true;
        }
        if (DisplayName.Equals(TEXT("M249"), ESearchCase::IgnoreCase))
        {
            Out = {{ FName(TEXT("/Game/Production/Weapons/M249")), FabRoot }, { TEXT("m249") }, 104.0f};
            return true;
        }
        if (DisplayName.Equals(TEXT("M14"), ESearchCase::IgnoreCase))
        {
            Out = {{ FName(TEXT("/Game/R13/Weapons/Stein/M14")), FabRoot }, { TEXT("m14") }, 112.0f};
            return true;
        }
        if (DisplayName.Equals(TEXT("MAC-10"), ESearchCase::IgnoreCase))
        {
            Out = {{ FName(TEXT("/Game/R13/Weapons/Stein/Mac10")), FabRoot }, { TEXT("mac10"), TEXT("mac-10") }, 30.0f};
            return true;
        }
        if (DisplayName.Equals(TEXT("TEC-9"), ESearchCase::IgnoreCase))
        {
            Out = {{ FName(TEXT("/Game/R13/Weapons/Stein/Tec9")), FabRoot }, { TEXT("tec9"), TEXT("tec-9") }, 33.0f};
            return true;
        }
        if (DisplayName.Equals(TEXT("Lever Action .45-70"), ESearchCase::IgnoreCase))
        {
            Out = {{ FName(TEXT("/Game/R13/Weapons/Stein/LeverAction")), FabRoot }, { TEXT("leveraction"), TEXT("lever_action"), TEXT("lever-action") }, 101.0f};
            return true;
        }
        if (DisplayName.Equals(TEXT("AK-74M"), ESearchCase::IgnoreCase))
        {
            Out = {{ FName(TEXT("/Game/ak-74m")), FabRoot }, { TEXT("ak-74m"), TEXT("ak74m"), TEXT("ak_74m") }, 94.0f};
            return true;
        }
        if (DisplayName.Equals(TEXT("AKS-74U"), ESearchCase::IgnoreCase))
        {
            Out = {{ FName(TEXT("/Game/aks-74u")), FName(TEXT("/Game/AKS-74U")), FabRoot },
                { TEXT("aks-74u"), TEXT("aks74u"), TEXT("aks_74u") }, 73.5f};
            return true;
        }
        if (DisplayName.Equals(TEXT("AR-15"), ESearchCase::IgnoreCase))
        {
            Out = {{ FName(TEXT("/Game/ar15-rifle")), FabRoot }, { TEXT("ar15"), TEXT("ar-15") }, 86.0f};
            return true;
        }
        if (DisplayName.Equals(TEXT("M4A1"), ESearchCase::IgnoreCase))
        {
            Out = {{ FName(TEXT("/Game/assault-rifle-m4a1")), FabRoot }, { TEXT("m4a1") }, 84.0f};
            return true;
        }
        if (DisplayName.Equals(TEXT("FN Ballista"), ESearchCase::IgnoreCase))
        {
            Out = {{ FName(TEXT("/Game/fn-ballista-sniper-rifle")), FabRoot }, { TEXT("ballista") }, 120.0f};
            return true;
        }
        if (DisplayName.Equals(TEXT("Kar98k"), ESearchCase::IgnoreCase))
        {
            Out = {{ FName(TEXT("/Game/kar98k-free-model")), FabRoot }, { TEXT("kar98"), TEXT("kar-98") }, 111.0f};
            return true;
        }
        if (DisplayName.Equals(TEXT("Makarov PM"), ESearchCase::IgnoreCase))
        {
            Out = {{ FName(TEXT("/Game/makarov-pistol")), FabRoot }, { TEXT("makarov") }, 16.2f};
            return true;
        }
        if (DisplayName.Equals(TEXT("Revolver"), ESearchCase::IgnoreCase))
        {
            Out = {{ FabRoot }, { TEXT("revolver") }, 28.0f};
            return true;
        }
        if (DisplayName.Equals(TEXT("Thompson M1A1"), ESearchCase::IgnoreCase))
        {
            Out = {{ FName(TEXT("/Game/tommy-gun")), FabRoot }, { TEXT("tommy"), TEXT("thompson") }, 81.0f};
            return true;
        }
        if (DisplayName.Equals(TEXT("M72 LAW"), ESearchCase::IgnoreCase))
        {
            Out = {{ FName(TEXT("/Game/law-light-anti-tank-weapon-m72")), FabRoot }, { TEXT("m72"), TEXT("law") }, 88.0f};
            return true;
        }
        if (DisplayName.Equals(TEXT("RPG-26"), ESearchCase::IgnoreCase))
        {
            Out = {{ FName(TEXT("/Game/rpg-26-grenade-launcher-low-poly")), FabRoot }, { TEXT("rpg-26"), TEXT("rpg26") }, 77.0f};
            return true;
        }
        if (DisplayName.Equals(TEXT("RPG Launcher"), ESearchCase::IgnoreCase))
        {
            // This identity is intentionally Fab-only so it can never steal the separate local RPG-26 model.
            Out = {{ FabRoot }, { TEXT("rpg") }, 95.0f};
            return true;
        }
        return false;
    }

    template <typename TMesh, typename TComponent>
    bool ApplyLocalVisualTyped(AOCWeaponBase& Weapon, TMesh* Mesh, float DesiredLength)
    {
        if (!Mesh || HasProductionVisual(Weapon)) return false;
        USceneComponent* VisualRoot = Weapon.GetWeaponVisualRoot();
        if (!VisualRoot) return false;

        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector NativeSize = Bounds.BoxExtent * 2.0f;
        const float NativeLength = FMath::Max3(NativeSize.X, NativeSize.Y, NativeSize.Z);
        if (NativeLength <= 1.0f) return false;

        const float UniformScale = DesiredLength / NativeLength;
        TComponent* Visual = NewObject<TComponent>(
            &Weapon,
            MakeUniqueObjectName(&Weapon, TComponent::StaticClass(), FName(TEXT("Pass45LocalImportedWeaponVisual"))));
        if (!Visual) return false;

        Visual->SetupAttachment(VisualRoot);
        if constexpr (std::is_same_v<TComponent, UStaticMeshComponent>)
        {
            Visual->SetStaticMesh(Mesh);
        }
        else
        {
            Visual->SetSkeletalMeshAsset(Mesh);
        }
        Visual->SetRelativeLocation(-Bounds.Origin * UniformScale);
        Visual->SetRelativeRotation(FRotator::ZeroRotator);
        Visual->SetRelativeScale3D(FVector(UniformScale));
        Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Visual->SetGenerateOverlapEvents(false);
        Visual->SetCanEverAffectNavigation(false);
        Visual->SetCastShadow(true);
        Visual->SetHiddenInGame(false, true);
        Visual->SetVisibility(true, true);
        Visual->ComponentTags.AddUnique(ProductionVisualTag);
        Visual->ComponentTags.AddUnique(LocalBridgeTag);
        Weapon.AddInstanceComponent(Visual);
        Visual->RegisterComponent();

        const int32 HiddenSourceProxyVisuals = HideSourceProxyVisuals(Weapon);
        RetireTemporaryRealFallbacks(Weapon);
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_LOCAL_IMPORTED_WEAPON_READY weapon=%s asset=%s desired_length_cm=%.1f mesh_kind=%s production_visual=1 temporary_fallback_retired=1 source_proxy_visuals_hidden=%d physics_root_preserved=1 resident_asset=1 sync_load=0 runtime_acceptance=0"),
            *Weapon.GetWeaponDisplayName(), *Mesh->GetPathName(), DesiredLength,
            std::is_same_v<TComponent, USkeletalMeshComponent> ? TEXT("skeletal") : TEXT("static"),
            HiddenSourceProxyVisuals);
        return true;
    }
}

bool UOCPass45ImportedWeaponBridgeSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && World->GetNetMode() != NM_DedicatedServer &&
        (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCPass45ImportedWeaponBridgeSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    InWorld.GetTimerManager().SetTimer(
        RefreshTimer,
        this,
        &UOCPass45ImportedWeaponBridgeSubsystem::RefreshWeapons,
        0.35f,
        true,
        0.08f);
}

void UOCPass45ImportedWeaponBridgeSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(RefreshTimer);
    for (const TSharedPtr<FStreamableHandle>& Handle : ResidentPreloadHandles)
    {
        if (Handle.IsValid()) Handle->CancelHandle();
    }
    ResidentPreloadHandles.Reset();
    RefreshPass = 0;
    Super::Deinitialize();
}

bool UOCPass45ImportedWeaponBridgeSubsystem::ApplyExactLocalVisual(AOCWeaponBase& Weapon)
{
    if (Weapon.ActorHasTag(LocalBridgePreloadPendingTag) ||
        Weapon.ActorHasTag(LocalInboxBoundTag) ||
        Weapon.ActorHasTag(LocalInboxPreloadPendingTag) ||
        HasProductionVisual(Weapon))
    {
        return false;
    }

    FLocalWeaponQuery Query;
    if (!ResolveQuery(Weapon.GetWeaponDisplayName(), Query)) return false;

    bool bSkeletal = false;
    FSoftObjectPath AssetPath;
    if (!Weapon.ActorHasTag(LocalBridgeSkeletalGapTag))
    {
        AssetPath = OCPass45FindLocalSkeletalMeshPathStrict(Query.Roots, Query.Tokens);
        bSkeletal = AssetPath.IsValid();
    }
    if (!AssetPath.IsValid() && !Weapon.ActorHasTag(LocalBridgeStaticGapTag))
    {
        AssetPath = OCPass45FindLocalStaticMeshPathStrict(Query.Roots, Query.Tokens);
        bSkeletal = false;
    }
    if (!AssetPath.IsValid()) return false;

    if (UObject* Resolved = AssetPath.ResolveObject())
    {
        if (bSkeletal)
        {
            if (USkeletalMesh* Mesh = Cast<USkeletalMesh>(Resolved))
            {
                return ApplyLocalVisualTyped<USkeletalMesh, USkeletalMeshComponent>(
                    Weapon, Mesh, Query.DesiredLengthCm);
            }
        }
        else if (UStaticMesh* Mesh = Cast<UStaticMesh>(Resolved))
        {
            return ApplyLocalVisualTyped<UStaticMesh, UStaticMeshComponent>(
                Weapon, Mesh, Query.DesiredLengthCm);
        }

        Weapon.Tags.AddUnique(bSkeletal ? LocalBridgeSkeletalGapTag : LocalBridgeStaticGapTag);
        return false;
    }

    Weapon.Tags.AddUnique(LocalBridgePreloadPendingTag);
    const TWeakObjectPtr<AOCWeaponBase> WeakWeapon(&Weapon);
    TArray<FSoftObjectPath> Paths;
    Paths.Add(AssetPath);
    TSharedPtr<FStreamableHandle> Handle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
        Paths,
        FStreamableDelegate::CreateUObject(
            this,
            &UOCPass45ImportedWeaponBridgeSubsystem::CompleteExactLocalVisual,
            WeakWeapon,
            AssetPath,
            Query.DesiredLengthCm,
            bSkeletal));

    if (!Handle.IsValid())
    {
        Weapon.Tags.Remove(LocalBridgePreloadPendingTag);
        Weapon.Tags.AddUnique(bSkeletal ? LocalBridgeSkeletalGapTag : LocalBridgeStaticGapTag);
        UE_LOG(LogTemp, Error,
            TEXT("GAME_RECOVERY_IMPORTED_WEAPON_PRELOAD_FAIL weapon=%s asset=%s reason=invalid_handle sync_load=0"),
            *Weapon.GetWeaponDisplayName(), *AssetPath.ToString());
        return false;
    }

    ResidentPreloadHandles.Add(Handle);
    UE_LOG(LogTemp, Display,
        TEXT("GAME_RECOVERY_IMPORTED_WEAPON_PRELOAD_BEGIN weapon=%s asset=%s mesh_kind=%s async=1 sync_load=0"),
        *Weapon.GetWeaponDisplayName(), *AssetPath.ToString(), bSkeletal ? TEXT("skeletal") : TEXT("static"));
    return false;
}

void UOCPass45ImportedWeaponBridgeSubsystem::CompleteExactLocalVisual(
    TWeakObjectPtr<AOCWeaponBase> WeakWeapon,
    FSoftObjectPath AssetPath,
    const float DesiredLengthCm,
    const bool bSkeletal)
{
    AOCWeaponBase* Weapon = WeakWeapon.Get();
    if (!Weapon || Weapon->IsActorBeingDestroyed()) return;

    Weapon->Tags.Remove(LocalBridgePreloadPendingTag);
    if (Weapon->ActorHasTag(LocalInboxBoundTag) || HasProductionVisual(*Weapon)) return;

    UObject* Resolved = AssetPath.ResolveObject();
    bool bApplied = false;
    if (bSkeletal)
    {
        if (USkeletalMesh* Mesh = Cast<USkeletalMesh>(Resolved))
        {
            bApplied = ApplyLocalVisualTyped<USkeletalMesh, USkeletalMeshComponent>(
                *Weapon, Mesh, DesiredLengthCm);
        }
    }
    else if (UStaticMesh* Mesh = Cast<UStaticMesh>(Resolved))
    {
        bApplied = ApplyLocalVisualTyped<UStaticMesh, UStaticMeshComponent>(
            *Weapon, Mesh, DesiredLengthCm);
    }

    if (bApplied) return;

    Weapon->Tags.AddUnique(bSkeletal ? LocalBridgeSkeletalGapTag : LocalBridgeStaticGapTag);
    UE_LOG(LogTemp, Error,
        TEXT("GAME_RECOVERY_IMPORTED_WEAPON_PRELOAD_GAP weapon=%s asset=%s mesh_kind=%s resolved=%d sync_load=0 runtime_acceptance=0"),
        *Weapon->GetWeaponDisplayName(), *AssetPath.ToString(),
        bSkeletal ? TEXT("skeletal") : TEXT("static"), Resolved ? 1 : 0);

    // A failed exact skeletal candidate may still have a valid exact static sibling. Retry metadata selection
    // without ever synchronously loading the failed package.
    if (bSkeletal) ApplyExactLocalVisual(*Weapon);
}

void UOCPass45ImportedWeaponBridgeSubsystem::RefreshWeapons()
{
    UWorld* World = GetWorld();
    if (!World) return;
    ++RefreshPass;

    int32 ExactCandidates = 0;
    int32 Applied = 0;
    for (TActorIterator<AOCWeaponBase> It(World); It; ++It)
    {
        AOCWeaponBase* Weapon = *It;
        if (!Weapon || Weapon->IsActorBeingDestroyed() || HasProductionVisual(*Weapon) ||
            Weapon->ActorHasTag(LocalBridgePreloadPendingTag) ||
            Weapon->ActorHasTag(LocalInboxBoundTag) ||
            Weapon->ActorHasTag(LocalInboxPreloadPendingTag))
        {
            continue;
        }

        FLocalWeaponQuery Query;
        if (!ResolveQuery(Weapon->GetWeaponDisplayName(), Query)) continue;
        ++ExactCandidates;
        if (ApplyExactLocalVisual(*Weapon)) ++Applied;
    }

    if (Applied > 0)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_LOCAL_IMPORTED_WEAPON_BRIDGE_PASS pass=%d exact_candidates=%d applied=%d all_declared_local_identities_supported=1 wrong_identity_substitution=0 blocking_asset_loads=0"),
            RefreshPass, ExactCandidates, Applied);
    }

    if (RefreshPass == FastRefreshPasses)
    {
        const AOCGameMode* GameMode = World->GetAuthGameMode<AOCGameMode>();
        if (GameMode && GameMode->IsSandboxMode())
        {
            World->GetTimerManager().SetTimer(
                RefreshTimer,
                this,
                &UOCPass45ImportedWeaponBridgeSubsystem::RefreshWeapons,
                SandboxWatchIntervalSeconds,
                true,
                SandboxWatchIntervalSeconds);
            UE_LOG(LogTemp, Display,
                TEXT("PASS45_LOCAL_IMPORTED_WEAPON_BRIDGE_SANDBOX_WATCH_READY fast_passes=%d interval_s=%.2f late_spawn_support=1 permanent_scan_sandbox_only=1 wrong_identity_substitution=0 blocking_asset_loads=0"),
                FastRefreshPasses, SandboxWatchIntervalSeconds);
        }
        else
        {
            World->GetTimerManager().ClearTimer(RefreshTimer);
            UE_LOG(LogTemp, Display,
                TEXT("PASS45_LOCAL_IMPORTED_WEAPON_BRIDGE_STOPPED passes=%d permanent_scan=0 sandbox=0 wrong_identity_substitution=0 blocking_asset_loads=0"),
                RefreshPass);
        }
    }
}
