#include "OCLocalInboxWeaponOverrideSubsystem.h"

#include "OCAntiArmorLauncher.h"
#include "OCLocalInboxRuntimeSubsystem.h"
#include "OCWeaponBase.h"
#include "OCWeaponVariants.h"

#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/StreamableManager.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/UObjectGlobals.h"

namespace
{
    const FName LocalVisualTag(TEXT("OC_LocalInboxWeaponVisual"));
    const FName LocalBoundTag(TEXT("OC_LocalInboxWeaponBound"));
    const FName LocalPreloadPendingTag(TEXT("OC_LocalInboxWeaponPreloadPending"));
    const FName ProductionVisualTag(TEXT("OC_ProductionWeaponVisual"));

    template <typename TMesh, typename TComponent>
    TComponent* AddVisual(AOCWeaponBase* Weapon, USceneComponent* Root, TMesh* Mesh,
        const FName BaseName, const float DesiredLengthCm)
    {
        if (!Weapon || !Root || !Mesh) return nullptr;
        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector NativeSize = Bounds.BoxExtent * 2.0f;
        const float Longest = FMath::Max3(NativeSize.X, NativeSize.Y, NativeSize.Z);
        if (Longest <= 1.0f) return nullptr;

        const FName UniqueName = MakeUniqueObjectName(Weapon, TComponent::StaticClass(), BaseName);
        TComponent* Visual = NewObject<TComponent>(Weapon, UniqueName);
        if (!Visual) return nullptr;

        const float UniformScale = DesiredLengthCm / Longest;
        Visual->SetupAttachment(Root);
        Visual->SetRelativeLocation(-Bounds.Origin * UniformScale);
        Visual->SetRelativeRotation(FRotator::ZeroRotator);
        Visual->SetRelativeScale3D(FVector(UniformScale));
        Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Visual->SetGenerateOverlapEvents(false);
        Visual->SetCanEverAffectNavigation(false);
        Visual->SetCastShadow(true);
        Visual->ComponentTags.Add(LocalVisualTag);
        Visual->ComponentTags.Add(ProductionVisualTag);
        Weapon->AddInstanceComponent(Visual);
        Visual->RegisterComponent();
        return Visual;
    }

    void HideOldWeaponPresentation(AOCWeaponBase* Weapon)
    {
        if (!Weapon) return;

        TInlineComponentArray<UStaticMeshComponent*> StaticComponents;
        Weapon->GetComponents(StaticComponents);
        for (UStaticMeshComponent* Component : StaticComponents)
        {
            if (Component && !Component->ComponentHasTag(LocalVisualTag))
            {
                // Preserve the component as physics/pickup authority. Only its old rendering is retired.
                Component->SetVisibility(false, false);
                Component->SetHiddenInGame(true, false);
            }
        }

        TInlineComponentArray<USkeletalMeshComponent*> SkeletalComponents;
        Weapon->GetComponents(SkeletalComponents);
        for (USkeletalMeshComponent* Component : SkeletalComponents)
        {
            if (Component && !Component->ComponentHasTag(LocalVisualTag))
            {
                Component->SetVisibility(false, false);
                Component->SetHiddenInGame(true, false);
            }
        }
    }
}

bool UOCLocalInboxWeaponOverrideSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCLocalInboxWeaponOverrideSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (InWorld.GetNetMode() == NM_DedicatedServer) return;
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    int32 BoundAtStart = 0;
    for (TActorIterator<AOCWeaponBase> It(&InWorld); It; ++It)
    {
        const bool bBefore = It->ActorHasTag(LocalBoundTag);
        ApplyLocalVisual(*It);
        if (!bBefore && It->ActorHasTag(LocalBoundTag)) ++BoundAtStart;
    }

    ActorSpawnedHandle = InWorld.AddOnActorSpawnedHandler(
        FOnActorSpawned::FDelegate::CreateUObject(this, &UOCLocalInboxWeaponOverrideSubsystem::HandleActorSpawned));
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_LOCAL_WEAPON_OVERRIDE_READY initial_bound=%d spawn_hook=1 blocking_load=0 async_missing_assets=1"),
        BoundAtStart);
}

void UOCLocalInboxWeaponOverrideSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        if (ActorSpawnedHandle.IsValid()) World->RemoveOnActorSpawnedHandler(ActorSpawnedHandle);
    }
    ActorSpawnedHandle.Reset();

    for (const TSharedPtr<FStreamableHandle>& Handle : ResidentPreloadHandles)
    {
        if (Handle.IsValid()) Handle->CancelHandle();
    }
    ResidentPreloadHandles.Reset();

    Super::Deinitialize();
}

void UOCLocalInboxWeaponOverrideSubsystem::HandleActorSpawned(AActor* Actor)
{
    AOCWeaponBase* Weapon = Cast<AOCWeaponBase>(Actor);
    UWorld* World = GetWorld();
    if (!Weapon || !World) return;

    const TWeakObjectPtr<AOCWeaponBase> WeakWeapon(Weapon);
    World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this, WeakWeapon]()
    {
        if (AOCWeaponBase* LiveWeapon = WeakWeapon.Get()) ApplyLocalVisual(LiveWeapon);
    }));
}

bool UOCLocalInboxWeaponOverrideSubsystem::ResolveVisualForWeapon(AOCWeaponBase* Weapon,
    FString& OutObjectPath, float& OutDesiredLengthCm, FString& OutCategory) const
{
    if (!Weapon) return false;

    const FString ForcedCategoryPrefix(TEXT("OC_FORCE_WEAPON_CATEGORY_"));
    const FString ForcedPathIndexPrefix(TEXT("OC_FORCE_WEAPON_PATH_INDEX_"));
    FString ForcedCategory;
    int32 ForcedPathIndex = INDEX_NONE;
    for (const FName& Tag : Weapon->Tags)
    {
        const FString TagText = Tag.ToString();
        if (TagText.StartsWith(ForcedCategoryPrefix, ESearchCase::CaseSensitive))
        {
            ForcedCategory = TagText.Mid(ForcedCategoryPrefix.Len());
        }
        else if (TagText.StartsWith(ForcedPathIndexPrefix, ESearchCase::CaseSensitive))
        {
            ForcedPathIndex = FCString::Atoi(*TagText.Mid(ForcedPathIndexPrefix.Len()));
        }
    }

    auto UseCanonicalPath = [&](const TCHAR* ObjectPath, const float LengthCm, const TCHAR* Category)
    {
        OutObjectPath = ObjectPath;
        OutDesiredLengthCm = LengthCm;
        OutCategory = Category;
        return true;
    };

    if (!ForcedCategory.IsEmpty())
    {
        // Exact built-in identities must not be replaced by an arbitrary manifest fallback. These assets are
        // already tracked by the project and are loaded asynchronously by ApplyLocalVisual below.
        if (ForcedCategory.Equals(TEXT("AK47"), ESearchCase::IgnoreCase))
            return UseCanonicalPath(TEXT("/Game/AK-47/Mesh/SKM_AK-47.SKM_AK-47"), 88.0f, TEXT("AK47"));
        if (ForcedCategory.Equals(TEXT("M700"), ESearchCase::IgnoreCase))
            return UseCanonicalPath(TEXT("/Game/R13/Weapons/Stein/M700/SKM_M700.SKM_M700"), 112.0f, TEXT("M700"));

        TArray<FString> ForcedPaths;
        UOCLocalInboxRuntimeSubsystem::GetAssetObjectPathsForCategory(ForcedCategory, ForcedPaths);
        if (!ForcedPaths.IsEmpty())
        {
            const int32 SafePathIndex = ForcedPaths.IsValidIndex(ForcedPathIndex) ? ForcedPathIndex : 0;
            OutObjectPath = ForcedPaths[SafePathIndex];
            OutCategory = ForcedCategory;

            if (Cast<AOCWeapon_M14>(Weapon)) OutDesiredLengthCm = 112.0f;
            else if (Cast<AOCWeapon_Mac10>(Weapon)) OutDesiredLengthCm = 30.0f;
            else if (Cast<AOCWeapon_Tec9>(Weapon)) OutDesiredLengthCm = 32.0f;
            else if (Cast<AOCWeapon_LeverAction>(Weapon)) OutDesiredLengthCm = 105.0f;
            else if (Cast<AOCAntiArmorLauncher>(Weapon)) OutDesiredLengthCm = ForcedCategory.Equals(TEXT("M72"), ESearchCase::IgnoreCase) ? 78.0f : 105.0f;
            else if (Cast<AOCWeapon_Shotgun>(Weapon)) OutDesiredLengthCm = 100.0f;
            else if (Cast<AOCWeapon_LMG>(Weapon)) OutDesiredLengthCm = ForcedCategory.Equals(TEXT("M249"), ESearchCase::IgnoreCase) ? 104.0f : 105.0f;
            else if (Cast<AOCWeapon_Sniper>(Weapon))
            {
                if (ForcedCategory.Equals(TEXT("BALLISTA"), ESearchCase::IgnoreCase)) OutDesiredLengthCm = 118.0f;
                else if (ForcedCategory.Equals(TEXT("KAR98"), ESearchCase::IgnoreCase)) OutDesiredLengthCm = 111.0f;
                else if (ForcedCategory.Equals(TEXT("SNIPER_GENERIC"), ESearchCase::IgnoreCase)) OutDesiredLengthCm = 115.0f;
                else OutDesiredLengthCm = 112.0f;
            }
            else if (Cast<AOCWeapon_Pistol>(Weapon))
            {
                if (ForcedCategory.Equals(TEXT("MAKAROV"), ESearchCase::IgnoreCase)) OutDesiredLengthCm = 22.0f;
                else if (ForcedCategory.Equals(TEXT("PISTOL_GENERIC"), ESearchCase::IgnoreCase)) OutDesiredLengthCm = 24.0f;
                else OutDesiredLengthCm = 23.0f;
            }
            else if (Cast<AOCWeapon_SMG>(Weapon)) OutDesiredLengthCm = ForcedCategory.Equals(TEXT("MP5"), ESearchCase::IgnoreCase) ? 68.0f : 62.0f;
            else if (Cast<AOCWeapon_AssaultRifle>(Weapon))
            {
                if (ForcedCategory.Equals(TEXT("AK74"), ESearchCase::IgnoreCase)) OutDesiredLengthCm = 94.0f;
                else if (ForcedCategory.Equals(TEXT("AK47"), ESearchCase::IgnoreCase)) OutDesiredLengthCm = 88.0f;
                else if (ForcedCategory.Equals(TEXT("ASSAULT_GENERIC"), ESearchCase::IgnoreCase)) OutDesiredLengthCm = 98.0f;
                else if (ForcedCategory.Equals(TEXT("RIFLE_GENERIC"), ESearchCase::IgnoreCase)) OutDesiredLengthCm = 105.0f;
                else OutDesiredLengthCm = 100.0f;
            }
            else OutDesiredLengthCm = 100.0f;

            return true;
        }
    }

    auto TryCategory = [&](const TCHAR* Category, const float LengthCm)
    {
        TArray<FString> Paths;
        UOCLocalInboxRuntimeSubsystem::GetAssetObjectPathsForCategory(Category, Paths);
        if (Paths.IsEmpty()) return false;
        const uint32 Seed = GetTypeHash(Weapon->GetName());
        OutObjectPath = Paths[Seed % static_cast<uint32>(Paths.Num())];
        OutDesiredLengthCm = LengthCm;
        OutCategory = Category;
        return true;
    };

    // Check specific subclasses before broad assault/SMG/pistol families. Otherwise a MAC-10/M14/etc.
    // can be swallowed by a parent class and the downloaded exact model never gets a chance to appear.
    if (Cast<AOCWeapon_M14>(Weapon)) return TryCategory(TEXT("M14"), 112.0f) || TryCategory(TEXT("RIFLE_GENERIC"), 108.0f);
    if (Cast<AOCWeapon_Mac10>(Weapon)) return TryCategory(TEXT("MAC10"), 30.0f) || TryCategory(TEXT("SMG_GENERIC"), 55.0f);
    if (Cast<AOCWeapon_Tec9>(Weapon)) return TryCategory(TEXT("TEC9"), 32.0f) || TryCategory(TEXT("SMG_GENERIC"), 55.0f);
    if (Cast<AOCWeapon_LeverAction>(Weapon)) return TryCategory(TEXT("LEVER_ACTION"), 105.0f) || TryCategory(TEXT("RIFLE_GENERIC"), 108.0f);

    // AOCAntiArmorLauncher owns its normal exact visual itself. LocalInbox is used only for explicitly forced
    // rack variants, otherwise both owners attach a launcher mesh to the same actor.
    if (Cast<AOCAntiArmorLauncher>(Weapon)) return false;

    if (Cast<AOCWeapon_Shotgun>(Weapon))
    {
        return TryCategory(TEXT("REMINGTON870"), 100.0f) || TryCategory(TEXT("SHOTGUN_GENERIC"), 100.0f);
    }
    if (Cast<AOCWeapon_LMG>(Weapon))
    {
        // Exact locally imported M249 wins when the manifest has it. Older tracked R13 machinegun is a real-mesh
        // fallback so the LMG is never an invisible gameplay weapon merely because the local production import is absent.
        return TryCategory(TEXT("M249"), 104.0f) ||
            UseCanonicalPath(TEXT("/Game/R13/Weapons/machinegun.machinegun"), 104.0f, TEXT("M249_TRACKED_FALLBACK"));
    }
    if (Cast<AOCWeapon_Sniper>(Weapon))
    {
        return UseCanonicalPath(TEXT("/Game/R13/Weapons/Stein/M700/SKM_M700.SKM_M700"), 112.0f, TEXT("M700"));
    }
    if (Cast<AOCWeapon_Pistol>(Weapon))
    {
        return TryCategory(TEXT("M1911"), 23.0f) || TryCategory(TEXT("MAKAROV"), 22.0f) ||
            TryCategory(TEXT("PISTOL_GENERIC"), 24.0f);
    }
    if (Cast<AOCWeapon_SMG>(Weapon))
    {
        return TryCategory(TEXT("MP5"), 68.0f) || TryCategory(TEXT("SMG_GENERIC"), 62.0f);
    }
    if (Cast<AOCWeapon_AssaultRifle>(Weapon))
    {
        return UseCanonicalPath(TEXT("/Game/AK-47/Mesh/SKM_AK-47.SKM_AK-47"), 88.0f, TEXT("AK47"));
    }
    return false;
}

void UOCLocalInboxWeaponOverrideSubsystem::ApplyLocalVisual(AOCWeaponBase* Weapon)
{
    if (!Weapon || Weapon->ActorHasTag(LocalBoundTag) || Weapon->ActorHasTag(LocalPreloadPendingTag)) return;

    FString ObjectPath;
    FString Category;
    float DesiredLengthCm = 100.0f;
    if (!ResolveVisualForWeapon(Weapon, ObjectPath, DesiredLengthCm, Category)) return;

    const FSoftObjectPath AssetPath(ObjectPath);
    if (!AssetPath.IsValid())
    {
        UE_LOG(LogTemp, Error,
            TEXT("GAME_RECOVERY_LOCAL_WEAPON_PRELOAD_FAIL reason=invalid_path category=%s asset=%s sync_load=0"),
            *Category, *ObjectPath);
        return;
    }

    if (AssetPath.ResolveObject())
    {
        ApplyResidentLocalVisual(Weapon, ObjectPath, DesiredLengthCm, Category);
        return;
    }

    Weapon->Tags.AddUnique(LocalPreloadPendingTag);
    const TWeakObjectPtr<AOCWeaponBase> WeakWeapon(Weapon);
    TArray<FSoftObjectPath> Paths;
    Paths.Add(AssetPath);
    TSharedPtr<FStreamableHandle> Handle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
        Paths,
        FStreamableDelegate::CreateUObject(
            this,
            &UOCLocalInboxWeaponOverrideSubsystem::CompleteLocalVisualPreload,
            WeakWeapon,
            ObjectPath,
            DesiredLengthCm,
            Category));

    if (!Handle.IsValid())
    {
        Weapon->Tags.Remove(LocalPreloadPendingTag);
        UE_LOG(LogTemp, Error,
            TEXT("GAME_RECOVERY_LOCAL_WEAPON_PRELOAD_FAIL reason=invalid_handle category=%s asset=%s sync_load=0"),
            *Category, *ObjectPath);
        return;
    }

    ResidentPreloadHandles.Add(Handle);
    UE_LOG(LogTemp, Display,
        TEXT("GAME_RECOVERY_LOCAL_WEAPON_PRELOAD_BEGIN weapon=%s category=%s asset=%s async=1 sync_load=0"),
        *Weapon->GetName(), *Category, *ObjectPath);
}

void UOCLocalInboxWeaponOverrideSubsystem::CompleteLocalVisualPreload(
    TWeakObjectPtr<AOCWeaponBase> WeakWeapon,
    FString ObjectPath,
    float DesiredLengthCm,
    FString Category)
{
    AOCWeaponBase* Weapon = WeakWeapon.Get();
    if (!Weapon || Weapon->IsActorBeingDestroyed()) return;

    Weapon->Tags.Remove(LocalPreloadPendingTag);
    ApplyResidentLocalVisual(Weapon, ObjectPath, DesiredLengthCm, Category);
}

void UOCLocalInboxWeaponOverrideSubsystem::ApplyResidentLocalVisual(
    AOCWeaponBase* Weapon,
    const FString& ObjectPath,
    const float DesiredLengthCm,
    const FString& Category)
{
    if (!Weapon || Weapon->ActorHasTag(LocalBoundTag)) return;

    USceneComponent* VisualRoot = Weapon->GetWeaponVisualRoot();
    if (!VisualRoot) return;

    UObject* ResolvedAsset = FSoftObjectPath(ObjectPath).ResolveObject();
    bool bBound = false;
    if (UStaticMesh* StaticMesh = Cast<UStaticMesh>(ResolvedAsset))
    {
        if (UStaticMeshComponent* Visual = AddVisual<UStaticMesh, UStaticMeshComponent>(
            Weapon, VisualRoot, StaticMesh, FName(TEXT("LocalInboxWeaponStatic")), DesiredLengthCm))
        {
            Visual->SetStaticMesh(StaticMesh);
            bBound = true;
        }
    }
    else if (USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(ResolvedAsset))
    {
        if (USkeletalMeshComponent* Visual = AddVisual<USkeletalMesh, USkeletalMeshComponent>(
            Weapon, VisualRoot, SkeletalMesh, FName(TEXT("LocalInboxWeaponSkeletal")), DesiredLengthCm))
        {
            Visual->SetSkeletalMeshAsset(SkeletalMesh);
            bBound = true;
        }
    }

    if (!bBound)
    {
        UE_LOG(LogTemp, Error,
            TEXT("GAME_RECOVERY_LOCAL_WEAPON_PRELOAD_GAP category=%s asset=%s resolved=%d sync_load=0 runtime_acceptance=0"),
            *Category, *ObjectPath, ResolvedAsset ? 1 : 0);
        return;
    }

    HideOldWeaponPresentation(Weapon);
    Weapon->Tags.AddUnique(LocalBoundTag);
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_LOCAL_WEAPON_RUNTIME_BOUND weapon=%s category=%s asset=%s production_visual=1 visual_root_unscaled=1 physics_root_preserved=1 resident_asset=1 sync_load=0 runtime_acceptance=0"),
        *Weapon->GetName(), *Category, *ObjectPath);
}
