#include "OCRealWeaponFallbackSubsystem.h"

#include "OCWeaponBase.h"

#include "Components/MeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/StaticMesh.h"
#include "Engine/StreamableManager.h"
#include "Engine/Texture.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Materials/MaterialInterface.h"
#include "UObject/SoftObjectPath.h"

namespace
{
    constexpr const TCHAR* GenericMachineGunPath = TEXT("/Game/R13/Weapons/machinegun.machinegun");
    constexpr const TCHAR* GenericPistolPath = TEXT("/Game/R13/Weapons/pistol.pistol");
    constexpr const TCHAR* GenericSMGPath = TEXT("/Game/R13/Weapons/uzi.uzi");
    constexpr const TCHAR* GenericShotgunPath = TEXT("/Game/R13/Weapons/shotgun.shotgun");
    constexpr const TCHAR* AuthoredAKFallbackPath = TEXT("/Game/AK-47/Mesh/SM_AK-47.SM_AK-47");

    const FName RealFallbackTag(TEXT("OC_RealMeshFallbackApplied"));
    const FName ProductionVisualTag(TEXT("OC_ProductionWeaponVisual"));
    const FName RealFallbackComponentTag(TEXT("OC_RealFallbackWeaponVisual"));
    const FName MaterialAuditCompleteTag(TEXT("OC_WeaponMaterialAuditComplete"));
    const FName AuthoredMaterialGapTag(TEXT("OC_WeaponAuthoredMaterialGap"));
    const FName RuntimeBaseRackTag(TEXT("OC_RuntimeBaseWeaponRack"));
    const FName PrimitiveVisualRetiredTag(TEXT("OC_PrimitiveWeaponVisualRetired"));
    constexpr int32 RequiredRackWeaponCountPerTeam = 11;
    constexpr int32 MaxExpectedRackWeapons = 22;
    constexpr int32 MaxRefreshPasses = 12;

    TArray<FSoftObjectPath> BuildFallbackPreloadPaths()
    {
        return {
            FSoftObjectPath(GenericMachineGunPath),
            FSoftObjectPath(GenericPistolPath),
            FSoftObjectPath(GenericSMGPath),
            FSoftObjectPath(GenericShotgunPath),
            FSoftObjectPath(AuthoredAKFallbackPath),
        };
    }

    UStaticMesh* ResolveResidentStaticMesh(const TCHAR* Path)
    {
        return Cast<UStaticMesh>(FSoftObjectPath(Path).ResolveObject());
    }

    bool HasProductionVisual(const AOCWeaponBase& Weapon)
    {
        TArray<UStaticMeshComponent*> StaticComponents;
        Weapon.GetComponents<UStaticMeshComponent>(StaticComponents);
        for (const UStaticMeshComponent* Component : StaticComponents)
        {
            if (IsValid(Component) && Component->ComponentHasTag(ProductionVisualTag) &&
                IsValid(Component->GetStaticMesh()))
            {
                return true;
            }
        }

        TArray<USkeletalMeshComponent*> SkeletalComponents;
        Weapon.GetComponents<USkeletalMeshComponent>(SkeletalComponents);
        for (const USkeletalMeshComponent* Component : SkeletalComponents)
        {
            if (IsValid(Component) && Component->ComponentHasTag(ProductionVisualTag) &&
                IsValid(Component->GetSkeletalMeshAsset()))
            {
                return true;
            }
        }
        return false;
    }

    bool IsRejectedPrimitiveMesh(const UStaticMeshComponent* Component)
    {
        if (!IsValid(Component) || !IsValid(Component->GetStaticMesh())) return false;
        const FString MeshPath = Component->GetStaticMesh()->GetPathName();
        return MeshPath.Contains(TEXT("/Engine/BasicShapes/"), ESearchCase::IgnoreCase);
    }

    int32 HideRejectedPrimitiveVisuals(AOCWeaponBase& Weapon)
    {
        TArray<UStaticMeshComponent*> StaticComponents;
        Weapon.GetComponents<UStaticMeshComponent>(StaticComponents);
        int32 HiddenCount = 0;
        for (UStaticMeshComponent* Component : StaticComponents)
        {
            if (!IsRejectedPrimitiveMesh(Component)) continue;

            const bool bWasRendered = Component->IsVisible();
            Component->SetVisibility(false, false);
            Component->SetHiddenInGame(true, false);
            Component->SetCastShadow(false);
            Component->SetCanEverAffectNavigation(false);
            if (bWasRendered) ++HiddenCount;
        }

        if (!Weapon.ActorHasTag(PrimitiveVisualRetiredTag))
        {
            Weapon.Tags.AddUnique(PrimitiveVisualRetiredTag);
            UE_LOG(LogTemp, Display,
                TEXT("PASS45_PRIMITIVE_WEAPON_VISUAL_RETIRED weapon=%s hidden_basicshape_components=%d collision_authority_preserved=1"),
                *Weapon.GetWeaponDisplayName(), HiddenCount);
        }
        return HiddenCount;
    }

    bool HasVisibleRejectedPrimitive(const AOCWeaponBase& Weapon)
    {
        TArray<UStaticMeshComponent*> StaticComponents;
        Weapon.GetComponents<UStaticMeshComponent>(StaticComponents);
        for (const UStaticMeshComponent* Component : StaticComponents)
        {
            if (IsRejectedPrimitiveMesh(Component) && Component->IsVisible())
            {
                return true;
            }
        }
        return false;
    }

    bool IsPlaceholderTexture(const UTexture* Texture)
    {
        if (!Texture) return true;
        const FString Path = Texture->GetPathName();
        const FString Name = Texture->GetName();
        return Path.Contains(TEXT("DefaultTexture"), ESearchCase::IgnoreCase) ||
            Path.Contains(TEXT("WhiteSquareTexture"), ESearchCase::IgnoreCase) ||
            Name.Equals(TEXT("DefaultTexture"), ESearchCase::IgnoreCase) ||
            Name.Equals(TEXT("WhiteSquareTexture"), ESearchCase::IgnoreCase);
    }

    bool IsMissingOrDefaultMaterial(const UMaterialInterface* Material)
    {
        if (!Material) return true;
        const FString Path = Material->GetPathName();
        const bool bPlaceholderMaterial =
            Path.Contains(TEXT("/Engine/EngineMaterials/DefaultMaterial"), ESearchCase::IgnoreCase) ||
            Path.Contains(TEXT("/Engine/BasicShapes/BasicShapeMaterial"), ESearchCase::IgnoreCase) ||
            Path.Contains(TEXT("WorldGridMaterial"), ESearchCase::IgnoreCase) ||
            Material->GetName().Equals(TEXT("DefaultMaterial"), ESearchCase::IgnoreCase) ||
            Material->GetName().Equals(TEXT("BasicShapeMaterial"), ESearchCase::IgnoreCase);
        if (bPlaceholderMaterial) return true;

        TArray<UTexture*> UsedTextures;
        Material->GetUsedTextures(
            UsedTextures,
            EMaterialQualityLevel::High,
            true,
            ERHIFeatureLevel::SM5,
            true);
        if (UsedTextures.IsEmpty()) return true;

        for (const UTexture* Texture : UsedTextures)
        {
            if (IsPlaceholderTexture(Texture)) return true;
        }
        return false;
    }
}

bool UOCRealWeaponFallbackSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCRealWeaponFallbackSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    if (InWorld.GetNetMode() == NM_DedicatedServer) return;
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    const TArray<FSoftObjectPath> Paths = BuildFallbackPreloadPaths();
    FallbackPreloadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
        Paths,
        FStreamableDelegate::CreateUObject(this, &UOCRealWeaponFallbackSubsystem::CompleteFallbackPreload));

    if (!FallbackPreloadHandle.IsValid())
    {
        UE_LOG(LogTemp, Error,
            TEXT("GAME_RECOVERY_REAL_WEAPON_FALLBACK_PRELOAD_GAP reason=invalid_handle assets=%d sync_load=0 runtime_acceptance=0"),
            Paths.Num());
        InWorld.GetTimerManager().SetTimer(
            RefreshTimer,
            this,
            &UOCRealWeaponFallbackSubsystem::RefreshWeaponFallbacks,
            0.50f,
            true,
            0.0f);
        return;
    }

    UE_LOG(LogTemp, Display,
        TEXT("GAME_RECOVERY_REAL_WEAPON_FALLBACK_PRELOAD_BEGIN assets=%d async=1 sync_load=0 resident_until_deinitialize=1"),
        Paths.Num());
}

void UOCRealWeaponFallbackSubsystem::CompleteFallbackPreload()
{
    UWorld* World = GetWorld();
    if (!World) return;

    GenericMachineGun = ResolveResidentStaticMesh(GenericMachineGunPath);
    GenericPistol = ResolveResidentStaticMesh(GenericPistolPath);
    GenericSMG = ResolveResidentStaticMesh(GenericSMGPath);
    GenericShotgun = ResolveResidentStaticMesh(GenericShotgunPath);
    AuthoredAKFallback = ResolveResidentStaticMesh(AuthoredAKFallbackPath);

    const int32 ResolvedCount =
        (GenericMachineGun ? 1 : 0) +
        (GenericPistol ? 1 : 0) +
        (GenericSMG ? 1 : 0) +
        (GenericShotgun ? 1 : 0) +
        (AuthoredAKFallback ? 1 : 0);

    UE_LOG(LogTemp, ResolvedCount == 5 ? Display : Warning,
        TEXT("GAME_RECOVERY_REAL_WEAPON_FALLBACK_PRELOAD_READY resolved=%d expected=5 async=1 resident_only=1 sync_load=0 runtime_acceptance=0"),
        ResolvedCount);

    World->GetTimerManager().SetTimer(
        RefreshTimer,
        this,
        &UOCRealWeaponFallbackSubsystem::RefreshWeaponFallbacks,
        0.50f,
        true,
        0.0f);
}

void UOCRealWeaponFallbackSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(RefreshTimer);
    if (FallbackPreloadHandle.IsValid())
    {
        FallbackPreloadHandle->CancelHandle();
        FallbackPreloadHandle.Reset();
    }
    GenericMachineGun = nullptr;
    GenericPistol = nullptr;
    GenericSMG = nullptr;
    GenericShotgun = nullptr;
    AuthoredAKFallback = nullptr;
    bRackMaterialAuditReadyLogged = false;
    RefreshPassCount = 0;
    Super::Deinitialize();
}

int32 UOCRealWeaponFallbackSubsystem::AuditAndRepairWeaponMaterials(AOCWeaponBase& Weapon)
{
    if (Weapon.ActorHasTag(MaterialAuditCompleteTag))
    {
        return Weapon.ActorHasTag(AuthoredMaterialGapTag) ? 1 : 0;
    }

    TInlineComponentArray<UMeshComponent*> MeshComponents;
    Weapon.GetComponents(MeshComponents);

    int32 VisualComponents = 0;
    int32 MissingAuthoredSlots = 0;

    for (UMeshComponent* Component : MeshComponents)
    {
        if (!IsValid(Component)) continue;
        const bool bRelevantVisual = Component->ComponentHasTag(ProductionVisualTag) ||
            Component->ComponentHasTag(RealFallbackComponentTag);
        if (!bRelevantVisual) continue;

        ++VisualComponents;
        if (Weapon.ActorHasTag(RuntimeBaseRackTag))
        {
            Component->SetCastShadow(false);
            Component->SetCanEverAffectNavigation(false);
        }

        const int32 SlotCount = FMath::Max(1, Component->GetNumMaterials());
        for (int32 Slot = 0; Slot < SlotCount; ++Slot)
        {
            if (IsMissingOrDefaultMaterial(Component->GetMaterial(Slot)))
            {
                ++MissingAuthoredSlots;
            }
        }
    }

    if (VisualComponents <= 0) return 0;

    Weapon.Tags.AddUnique(MaterialAuditCompleteTag);
    if (MissingAuthoredSlots > 0)
    {
        Weapon.Tags.AddUnique(AuthoredMaterialGapTag);
        UE_LOG(LogTemp, Error,
            TEXT("PASS44_WEAPON_AUTHORED_MATERIAL_GAP weapon=%s missing_or_placeholder_slots=%d basicshape_repair=0 exact_material_ready=0"),
            *Weapon.GetWeaponDisplayName(), MissingAuthoredSlots);
    }
    else
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS44_WEAPON_AUTHORED_MATERIAL_READY weapon=%s missing_or_placeholder_slots=0 basicshape_repair=0 exact_material_ready=1"),
            *Weapon.GetWeaponDisplayName());
    }

    return MissingAuthoredSlots;
}

void UOCRealWeaponFallbackSubsystem::RefreshWeaponFallbacks()
{
    UWorld* World = GetWorld();
    if (!World) return;

    ++RefreshPassCount;

    int32 RackWeapons = 0;
    int32 RackAudited = 0;
    int32 RackGapWeapons = 0;
    int32 RackVisiblePrimitiveWeapons = 0;

    for (TActorIterator<AOCWeaponBase> It(World); It; ++It)
    {
        AOCWeaponBase* Weapon = *It;
        if (!IsValid(Weapon) || Weapon->IsActorBeingDestroyed()) continue;

        HideRejectedPrimitiveVisuals(*Weapon);
        if (HasVisibleRejectedPrimitive(*Weapon))
        {
            if (Weapon->ActorHasTag(RuntimeBaseRackTag)) ++RackVisiblePrimitiveWeapons;
            UE_LOG(LogTemp, Error,
                TEXT("PASS45_VISIBLE_PRIMITIVE_WEAPON_FAIL weapon=%s basicshape_visible=1 runtime_acceptance=0"),
                *Weapon->GetWeaponDisplayName());
        }

        if (Weapon->ActorHasTag(RuntimeBaseRackTag)) ++RackWeapons;

        AuditAndRepairWeaponMaterials(*Weapon);
        if (Weapon->ActorHasTag(RuntimeBaseRackTag) && Weapon->ActorHasTag(MaterialAuditCompleteTag))
        {
            ++RackAudited;
            if (Weapon->ActorHasTag(AuthoredMaterialGapTag)) ++RackGapWeapons;
        }

        if (Weapon->ActorHasTag(RealFallbackTag) || HasProductionVisual(*Weapon)) continue;

        const FString Name = Weapon->GetWeaponDisplayName();
        if (Name.Equals(TEXT("AK-47"), ESearchCase::IgnoreCase))
        {
            ApplyRealFallback(*Weapon, AuthoredAKFallback.Get(), 88.0f, TEXT("committed AK-47 static sibling"));
        }
        else if (Name.Equals(TEXT("MP5"), ESearchCase::IgnoreCase))
        {
            ApplyRealFallback(*Weapon, GenericSMG.Get(), 68.0f, TEXT("R13 real SMG temporary MP5 fallback"));
        }
        else if (Name.Equals(TEXT("M249"), ESearchCase::IgnoreCase))
        {
            ApplyRealFallback(*Weapon, GenericMachineGun.Get(), 104.0f, TEXT("R13 generic machinegun"));
        }
        else if (Name.Equals(TEXT("M1911"), ESearchCase::IgnoreCase))
        {
            ApplyRealFallback(*Weapon, GenericPistol.Get(), 24.0f, TEXT("R13 generic pistol"));
        }
        else if (Name.Equals(TEXT("MAC-10"), ESearchCase::IgnoreCase) || Name.Equals(TEXT("MAC10"), ESearchCase::IgnoreCase))
        {
            ApplyRealFallback(*Weapon, GenericSMG.Get(), 31.0f, TEXT("R13 generic SMG"));
        }
        else if (Name.Equals(TEXT("Remington 870"), ESearchCase::IgnoreCase))
        {
            ApplyRealFallback(*Weapon, GenericShotgun.Get(), 103.0f, TEXT("R13 generic shotgun"));
        }
    }

    const bool bRackCountValid = RackWeapons >= RequiredRackWeaponCountPerTeam && RackWeapons <= MaxExpectedRackWeapons;
    if (bRackCountValid && RackVisiblePrimitiveWeapons == 0)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_PRIMITIVE_WEAPON_RUNTIME_READY rack_weapons=%d visible_basicshape_weapons=0 content_readiness_separate=1"),
            RackWeapons);
    }

    const bool bRackAuditComplete = bRackCountValid && RackAudited == RackWeapons;

    if (bRackAuditComplete)
    {
        World->GetTimerManager().ClearTimer(RefreshTimer);

        if (RackGapWeapons == 0)
        {
            if (!bRackMaterialAuditReadyLogged)
            {
                bRackMaterialAuditReadyLogged = true;
                UE_LOG(LogTemp, Display,
                    TEXT("PASS36_WEAPON_MATERIAL_AUDIT_READY rack_weapons=%d audited=%d authored_material_gap_weapons=0 basicshape_repair=0"),
                    RackWeapons, RackAudited);
            }
            UE_LOG(LogTemp, Display,
                TEXT("PASS38_WEAPON_FALLBACK_SCAN_STOPPED reason=ready passes=%d rack_weapons=%d audited=%d permanent_scan=0"),
                RefreshPassCount, RackWeapons, RackAudited);
        }
        else
        {
            UE_LOG(LogTemp, Error,
                TEXT("PASS44_WEAPON_RACK_AUTHORED_MATERIAL_GAP rack_weapons=%d audited=%d gap_weapons=%d exact_material_ready=0 basicshape_repair=0"),
                RackWeapons, RackAudited, RackGapWeapons);
            UE_LOG(LogTemp, Display,
                TEXT("PASS38_WEAPON_FALLBACK_SCAN_STOPPED reason=material_gap_audited passes=%d rack_weapons=%d audited=%d gap_weapons=%d permanent_scan=0"),
                RefreshPassCount, RackWeapons, RackAudited, RackGapWeapons);
        }
        return;
    }

    if (RefreshPassCount >= MaxRefreshPasses)
    {
        World->GetTimerManager().ClearTimer(RefreshTimer);
        UE_LOG(LogTemp, Error,
            TEXT("PASS38_WEAPON_FALLBACK_SCAN_BOUNDED_STOP passes=%d max_passes=%d rack_weapons=%d audited=%d permanent_scan=0"),
            RefreshPassCount, MaxRefreshPasses, RackWeapons, RackAudited);
    }
}

bool UOCRealWeaponFallbackSubsystem::ApplyRealFallback(
    AOCWeaponBase& Weapon,
    UStaticMesh* Mesh,
    float DesiredLengthCm,
    const TCHAR* FallbackLabel)
{
    USceneComponent* PhysicsRoot = Weapon.GetRootComponent();
    USceneComponent* VisualRoot = Weapon.GetWeaponVisualRoot();
    if (!IsValid(&Weapon) || Weapon.IsActorBeingDestroyed() || !IsValid(Mesh) ||
        !IsValid(PhysicsRoot) || !IsValid(VisualRoot))
    {
        return false;
    }

    const FBoxSphereBounds Bounds = Mesh->GetBounds();
    const FVector NativeSize = Bounds.BoxExtent * 2.0f;
    const float NativeLength = FMath::Max3(NativeSize.X, NativeSize.Y, NativeSize.Z);
    if (NativeLength <= 1.0f) return false;

    TArray<UStaticMeshComponent*> StaticComponents;
    Weapon.GetComponents<UStaticMeshComponent>(StaticComponents);
    for (UStaticMeshComponent* Existing : StaticComponents)
    {
        if (!IsValid(Existing) || Existing->ComponentHasTag(RealFallbackComponentTag)) continue;
        Existing->SetVisibility(false, false);
        Existing->SetHiddenInGame(true, false);
        Existing->SetCastShadow(false);
        Existing->SetCanEverAffectNavigation(false);
        if (Existing != PhysicsRoot)
        {
            Existing->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }

    const FName ComponentName = MakeUniqueObjectName(
        &Weapon,
        UStaticMeshComponent::StaticClass(),
        FName(TEXT("OC_RealWeaponFallback")));
    UStaticMeshComponent* Visual = NewObject<UStaticMeshComponent>(&Weapon, ComponentName);
    if (!IsValid(Visual)) return false;

    Visual->SetupAttachment(VisualRoot);
    Visual->SetStaticMesh(Mesh);
    Visual->SetRelativeLocation(-Bounds.Origin * (DesiredLengthCm / NativeLength));
    Visual->SetRelativeRotation(FRotator::ZeroRotator);
    Visual->SetRelativeScale3D(FVector(DesiredLengthCm / NativeLength));
    Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Visual->SetGenerateOverlapEvents(false);
    Visual->SetCanEverAffectNavigation(false);
    Visual->SetCastShadow(false);
    Visual->SetHiddenInGame(false, true);
    Visual->SetVisibility(true, true);
    Visual->ComponentTags.Add(RealFallbackComponentTag);
    Weapon.AddInstanceComponent(Visual);
    Visual->RegisterComponent();

    Weapon.Tags.AddUnique(RealFallbackTag);
    UE_LOG(LogTemp, Warning,
        TEXT("PASS45_REAL_WEAPON_FALLBACK_READY weapon=%s fallback=%s exact_production=0 playable_fallback=1 primitive_visible=0 visual_root_unscaled=1 physics_root_preserved=1"),
        *Weapon.GetWeaponDisplayName(), FallbackLabel);
    return true;
}
