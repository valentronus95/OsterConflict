#include "OCRealWeaponFallbackSubsystem.h"

#include "OCWeaponBase.h"

#include "Components/MeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/UObjectGlobals.h"

namespace
{
    const FName RealFallbackTag(TEXT("OC_RealMeshFallbackApplied"));
    const FName ProductionVisualTag(TEXT("OC_ProductionWeaponVisual"));
    const FName RealFallbackComponentTag(TEXT("OC_RealFallbackWeaponVisual"));
    const FName MaterialAuditCompleteTag(TEXT("OC_WeaponMaterialAuditComplete"));
    const FName RuntimeBaseRackTag(TEXT("OC_RuntimeBaseWeaponRack"));
    constexpr int32 RequiredRackWeaponCountPerTeam = 11;
    constexpr int32 MaxExpectedRackWeapons = 22;
    constexpr int32 MaxRefreshPasses = 12;

    bool HasProductionVisual(const AOCWeaponBase& Weapon)
    {
        TArray<UActorComponent*> Components;
        Weapon.GetComponents(Components);
        for (const UActorComponent* Component : Components)
        {
            if (IsValid(Component) && Component->ComponentHasTag(ProductionVisualTag)) return true;
        }
        return false;
    }

    bool IsMissingOrDefaultMaterial(const UMaterialInterface* Material)
    {
        if (!Material) return true;
        const FString Path = Material->GetPathName();
        return Path.Contains(TEXT("/Engine/EngineMaterials/DefaultMaterial"), ESearchCase::IgnoreCase) ||
            Material->GetName().Equals(TEXT("DefaultMaterial"), ESearchCase::IgnoreCase);
    }

    FLinearColor ResolveRecoveryColor(const FString& WeaponName)
    {
        if (WeaponName.Contains(TEXT("M14"), ESearchCase::IgnoreCase) ||
            WeaponName.Contains(TEXT("Lever"), ESearchCase::IgnoreCase) ||
            WeaponName.Contains(TEXT("Remington"), ESearchCase::IgnoreCase) ||
            WeaponName.Contains(TEXT("M700"), ESearchCase::IgnoreCase))
        {
            return FLinearColor(0.20f, 0.095f, 0.035f, 1.0f);
        }
        if (WeaponName.Contains(TEXT("Launcher"), ESearchCase::IgnoreCase) ||
            WeaponName.Contains(TEXT("RPG"), ESearchCase::IgnoreCase))
        {
            return FLinearColor(0.10f, 0.15f, 0.075f, 1.0f);
        }
        return FLinearColor(0.055f, 0.065f, 0.075f, 1.0f);
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

    GenericMachineGun = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/R13/Weapons/machinegun.machinegun"));
    GenericPistol = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/R13/Weapons/pistol.pistol"));
    GenericSMG = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/R13/Weapons/uzi.uzi"));
    GenericShotgun = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/R13/Weapons/shotgun.shotgun"));
    MaterialRecoveryBase = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

    // Pass 38: startup warm-up only. The old timer ran forever at 4 Hz even after every BASE weapon had
    // converged. Twelve half-second passes are ample for delayed component registration while placing a hard
    // ceiling on world-wide actor/material scanning if some unrelated weapon never becomes auditable.
    InWorld.GetTimerManager().SetTimer(
        RefreshTimer,
        this,
        &UOCRealWeaponFallbackSubsystem::RefreshWeaponFallbacks,
        0.50f,
        true,
        0.05f);
}

void UOCRealWeaponFallbackSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(RefreshTimer);
    GenericMachineGun = nullptr;
    GenericPistol = nullptr;
    GenericSMG = nullptr;
    GenericShotgun = nullptr;
    MaterialRecoveryBase = nullptr;
    bRackMaterialAuditReadyLogged = false;
    RefreshPassCount = 0;
    Super::Deinitialize();
}

int32 UOCRealWeaponFallbackSubsystem::AuditAndRepairWeaponMaterials(AOCWeaponBase& Weapon)
{
    if (Weapon.ActorHasTag(MaterialAuditCompleteTag)) return 0;

    TInlineComponentArray<UMeshComponent*> MeshComponents;
    Weapon.GetComponents(MeshComponents);

    int32 VisualComponents = 0;
    int32 RepairedSlots = 0;
    UMaterialInstanceDynamic* RecoveryMaterial = nullptr;

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
            UMaterialInterface* Current = Component->GetMaterial(Slot);
            if (!IsMissingOrDefaultMaterial(Current)) continue;
            if (!MaterialRecoveryBase) continue;

            if (!RecoveryMaterial)
            {
                RecoveryMaterial = UMaterialInstanceDynamic::Create(
                    MaterialRecoveryBase,
                    &Weapon,
                    MakeUniqueObjectName(
                        &Weapon,
                        UMaterialInstanceDynamic::StaticClass(),
                        FName(TEXT("PASS36_WeaponMaterialRecovery"))));
                if (RecoveryMaterial)
                {
                    RecoveryMaterial->SetVectorParameterValue(
                        TEXT("Color"), ResolveRecoveryColor(Weapon.GetWeaponDisplayName()));
                }
            }
            if (!RecoveryMaterial) continue;

            Component->SetMaterial(Slot, RecoveryMaterial);
            ++RepairedSlots;
        }
    }

    if (VisualComponents <= 0) return 0;

    Weapon.Tags.Add(MaterialAuditCompleteTag);
    if (RepairedSlots > 0)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("PASS36_WEAPON_MATERIAL_RECOVERED weapon=%s repaired_slots=%d authored_materials_preserved=1"),
            *Weapon.GetWeaponDisplayName(), RepairedSlots);
    }
    return RepairedSlots;
}

void UOCRealWeaponFallbackSubsystem::RefreshWeaponFallbacks()
{
    UWorld* World = GetWorld();
    if (!World) return;

    ++RefreshPassCount;

    int32 RackWeapons = 0;
    int32 RackAudited = 0;
    int32 RepairedSlotsThisPass = 0;

    for (TActorIterator<AOCWeaponBase> It(World); It; ++It)
    {
        AOCWeaponBase* Weapon = *It;
        if (!IsValid(Weapon) || Weapon->IsActorBeingDestroyed()) continue;

        if (Weapon->ActorHasTag(RuntimeBaseRackTag)) ++RackWeapons;

        RepairedSlotsThisPass += AuditAndRepairWeaponMaterials(*Weapon);
        if (Weapon->ActorHasTag(RuntimeBaseRackTag) && Weapon->ActorHasTag(MaterialAuditCompleteTag)) ++RackAudited;

        if (Weapon->ActorHasTag(RealFallbackTag) || HasProductionVisual(*Weapon)) continue;

        const FString Name = Weapon->GetWeaponDisplayName();
        if (Name.Equals(TEXT("M249"), ESearchCase::IgnoreCase))
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

    const bool bRackReady = RackWeapons >= RequiredRackWeaponCountPerTeam &&
        RackWeapons <= MaxExpectedRackWeapons && RackAudited == RackWeapons;

    if (bRackReady)
    {
        if (!bRackMaterialAuditReadyLogged)
        {
            bRackMaterialAuditReadyLogged = true;
            UE_LOG(LogTemp, Display,
                TEXT("PASS36_WEAPON_MATERIAL_AUDIT_READY rack_weapons=%d audited=%d repaired_slots_last_pass=%d authored_materials_preserved=1"),
                RackWeapons, RackAudited, RepairedSlotsThisPass);
        }

        World->GetTimerManager().ClearTimer(RefreshTimer);
        UE_LOG(LogTemp, Display,
            TEXT("PASS38_WEAPON_FALLBACK_SCAN_STOPPED reason=ready passes=%d rack_weapons=%d audited=%d permanent_scan=0"),
            RefreshPassCount, RackWeapons, RackAudited);
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
    USceneComponent* RootComponent = Weapon.GetRootComponent();
    if (!IsValid(&Weapon) || Weapon.IsActorBeingDestroyed() || !IsValid(Mesh) || !IsValid(RootComponent))
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
        Existing->SetVisibility(false, true);
        Existing->SetHiddenInGame(true, true);
        Existing->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    const FName ComponentName = MakeUniqueObjectName(
        &Weapon,
        UStaticMeshComponent::StaticClass(),
        FName(TEXT("OC_RealWeaponFallback")));
    UStaticMeshComponent* Visual = NewObject<UStaticMeshComponent>(&Weapon, ComponentName);
    if (!IsValid(Visual)) return false;

    Visual->SetupAttachment(RootComponent);
    Visual->SetStaticMesh(Mesh);
    Visual->SetRelativeLocation(FVector::ZeroVector);
    Visual->SetRelativeRotation(FRotator::ZeroRotator);
    Visual->SetRelativeScale3D(FVector(DesiredLengthCm / NativeLength));
    Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Visual->SetGenerateOverlapEvents(false);
    Visual->SetCanEverAffectNavigation(false);
    Visual->SetCastShadow(false);
    Visual->ComponentTags.Add(RealFallbackComponentTag);
    Weapon.AddInstanceComponent(Visual);
    Visual->RegisterComponent();

    Weapon.Tags.Add(RealFallbackTag);
    UE_LOG(LogTemp, Warning,
        TEXT("Weapon '%s' exact production visual is unavailable; primitive hidden and %s real-mesh fallback applied. exact_production=0 playable_fallback=1"),
        *Weapon.GetWeaponDisplayName(), FallbackLabel);
    return true;
}