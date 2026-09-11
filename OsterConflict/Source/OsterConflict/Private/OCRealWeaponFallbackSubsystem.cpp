#include "OCRealWeaponFallbackSubsystem.h"

#include "OCWeaponBase.h"

#include "Components/MeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StreamableManager.h"
#include "Engine/Texture.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Materials/MaterialInterface.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

namespace
{
    const FName ProductionVisualTag(TEXT("OC_ProductionWeaponVisual"));
    const FName RealFallbackComponentTag(TEXT("OC_RealFallbackWeaponVisual"));
    const FName MaterialAuditCompleteTag(TEXT("OC_WeaponMaterialAuditComplete"));
    const FName AuthoredMaterialGapTag(TEXT("OC_WeaponAuthoredMaterialGap"));
    const FName RuntimeBaseRackTag(TEXT("OC_RuntimeBaseWeaponRack"));
    const FName PrimitiveVisualRetiredTag(TEXT("OC_PrimitiveWeaponVisualRetired"));
    constexpr int32 RequiredRackWeaponCountPerTeam = 11;
    constexpr int32 MaxExpectedRackWeapons = 22;
    constexpr int32 MaxRefreshPasses = 6;

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
            if (IsRejectedPrimitiveMesh(Component) && Component->IsVisible()) return true;
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

    // Normal gameplay needs primitive retirement once, not a material/texture dependency audit every 0.5 s.
    // The expensive GetUsedTextures audit is acceptance-only and remains available behind ValidateProductionWeapons.
    if (!FParse::Param(FCommandLine::Get(), TEXT("ValidateProductionWeapons")))
    {
        int32 WeaponsScanned = 0;
        int32 HiddenPrimitiveComponents = 0;
        for (TActorIterator<AOCWeaponBase> It(&InWorld); It; ++It)
        {
            AOCWeaponBase* Weapon = *It;
            if (!IsValid(Weapon) || Weapon->IsActorBeingDestroyed()) continue;
            ++WeaponsScanned;
            HiddenPrimitiveComponents += HideRejectedPrimitiveVisuals(*Weapon);
        }

        UE_LOG(LogTemp, Display,
            TEXT("GAME_RECOVERY_WEAPON_FALLBACK_GAMEPLAY_READY weapons_scanned=%d hidden_basicshape_components=%d material_audit=0 repeated_timer=0 generic_substitution=0"),
            WeaponsScanned, HiddenPrimitiveComponents);
        return;
    }

    InWorld.GetTimerManager().SetTimer(
        RefreshTimer,
        this,
        &UOCRealWeaponFallbackSubsystem::RefreshWeaponFallbacks,
        0.50f,
        true,
        0.0f);

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_GENERIC_WEAPON_FALLBACK_RETIRED validation=1 generic_substitution=0 exact_visual_owner=imported_bridge primitive_cleanup=1 material_audit=1"));
}

void UOCRealWeaponFallbackSubsystem::CompleteFallbackPreload()
{
    // Retained only for ABI/source compatibility with the existing header. No fallback assets are preloaded.
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
            if (IsMissingOrDefaultMaterial(Component->GetMaterial(Slot))) ++MissingAuthoredSlots;
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
    }

    const bool bRackCountValid = RackWeapons >= RequiredRackWeaponCountPerTeam && RackWeapons <= MaxExpectedRackWeapons;
    if (bRackCountValid && RackVisiblePrimitiveWeapons == 0)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_PRIMITIVE_WEAPON_RUNTIME_READY rack_weapons=%d visible_basicshape_weapons=0 content_readiness_separate=1 generic_substitution=0"),
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
        }
        else
        {
            UE_LOG(LogTemp, Error,
                TEXT("PASS44_WEAPON_RACK_AUTHORED_MATERIAL_GAP rack_weapons=%d audited=%d gap_weapons=%d exact_material_ready=0 basicshape_repair=0"),
                RackWeapons, RackAudited, RackGapWeapons);
        }
        return;
    }

    if (RefreshPassCount >= MaxRefreshPasses)
    {
        World->GetTimerManager().ClearTimer(RefreshTimer);
        UE_LOG(LogTemp, Error,
            TEXT("PASS38_WEAPON_FALLBACK_SCAN_BOUNDED_STOP passes=%d max_passes=%d rack_weapons=%d audited=%d permanent_scan=0 generic_substitution=0"),
            RefreshPassCount, MaxRefreshPasses, RackWeapons, RackAudited);
    }
}

bool UOCRealWeaponFallbackSubsystem::ApplyRealFallback(
    AOCWeaponBase& Weapon,
    UStaticMesh* Mesh,
    float DesiredLengthCm,
    const TCHAR* FallbackLabel)
{
    // Kept as a no-op for source compatibility. Wrong-identity replacement is intentionally forbidden.
    (void)Weapon;
    (void)Mesh;
    (void)DesiredLengthCm;
    (void)FallbackLabel;
    return false;
}
