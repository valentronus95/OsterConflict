#include "OCWeaponPalettePass37Subsystem.h"

#include "OCGameMode.h"
#include "OCWeaponBase.h"

#include "Components/MeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/UObjectGlobals.h"

namespace
{
    const FName RuntimeBaseRackTag(TEXT("OC_RuntimeBaseWeaponRack"));
    const FName ProductionVisualTag(TEXT("OC_ProductionWeaponVisual"));
    const FName RealFallbackComponentTag(TEXT("OC_RealFallbackWeaponVisual"));
    const FName PaletteAuditedTag(TEXT("OC_Pass37WeaponPaletteAudited"));
    constexpr int32 RequiredRackWeaponCountPerTeam = 11;
    constexpr int32 MaxExpectedRackWeapons = 22;
    constexpr int32 MaxAuditPasses = 12;

    bool IsAK(const FString& Name)
    {
        return Name.Contains(TEXT("AK-47"), ESearchCase::IgnoreCase) ||
            Name.Equals(TEXT("AK47"), ESearchCase::IgnoreCase);
    }

    bool IsRestoredSteinPayload(const FString& Name)
    {
        // Repository inventory for these restored folders contains mesh/WPN payloads but no separate
        // material/texture payloads beside them. That is a content gap, not permission to overwrite a
        // non-placeholder material that the imported mesh already carries internally.
        return Name.Contains(TEXT("MP5"), ESearchCase::IgnoreCase) ||
            Name.Contains(TEXT("M1911"), ESearchCase::IgnoreCase) ||
            Name.Contains(TEXT("M700"), ESearchCase::IgnoreCase) ||
            Name.Contains(TEXT("M14"), ESearchCase::IgnoreCase) ||
            Name.Contains(TEXT("MAC-10"), ESearchCase::IgnoreCase) ||
            Name.Contains(TEXT("MAC10"), ESearchCase::IgnoreCase) ||
            Name.Contains(TEXT("TEC-9"), ESearchCase::IgnoreCase) ||
            Name.Contains(TEXT("TEC9"), ESearchCase::IgnoreCase) ||
            Name.Contains(TEXT("Lever"), ESearchCase::IgnoreCase);
    }

    bool IsClearlyPlaceholderMaterial(const UMaterialInterface* Material)
    {
        if (!Material) return true;
        const FString Path = Material->GetPathName();
        return Path.Contains(TEXT("DefaultMaterial"), ESearchCase::IgnoreCase) ||
            Path.Contains(TEXT("BasicShapeMaterial"), ESearchCase::IgnoreCase) ||
            Path.Contains(TEXT("WorldGridMaterial"), ESearchCase::IgnoreCase);
    }

    FLinearColor ResolvePaletteColor(const FString& Name, const int32 Slot)
    {
        const bool bAlternate = (Slot % 2) != 0;

        if (Name.Contains(TEXT("M14"), ESearchCase::IgnoreCase) ||
            Name.Contains(TEXT("Lever"), ESearchCase::IgnoreCase))
        {
            return bAlternate
                ? FLinearColor(0.055f, 0.065f, 0.075f, 1.0f)
                : FLinearColor(0.16f, 0.075f, 0.025f, 1.0f);
        }
        if (Name.Contains(TEXT("M700"), ESearchCase::IgnoreCase))
        {
            return bAlternate
                ? FLinearColor(0.050f, 0.058f, 0.065f, 1.0f)
                : FLinearColor(0.09f, 0.11f, 0.055f, 1.0f);
        }
        if (Name.Contains(TEXT("M1911"), ESearchCase::IgnoreCase))
        {
            return bAlternate
                ? FLinearColor(0.12f, 0.045f, 0.020f, 1.0f)
                : FLinearColor(0.045f, 0.052f, 0.060f, 1.0f);
        }
        if (Name.Contains(TEXT("MP5"), ESearchCase::IgnoreCase) ||
            Name.Contains(TEXT("MAC"), ESearchCase::IgnoreCase) ||
            Name.Contains(TEXT("TEC"), ESearchCase::IgnoreCase))
        {
            return bAlternate
                ? FLinearColor(0.025f, 0.028f, 0.032f, 1.0f)
                : FLinearColor(0.055f, 0.062f, 0.070f, 1.0f);
        }
        if (Name.Contains(TEXT("Remington"), ESearchCase::IgnoreCase))
        {
            return bAlternate
                ? FLinearColor(0.15f, 0.065f, 0.022f, 1.0f)
                : FLinearColor(0.050f, 0.058f, 0.065f, 1.0f);
        }
        if (Name.Contains(TEXT("M249"), ESearchCase::IgnoreCase) ||
            Name.Contains(TEXT("Launcher"), ESearchCase::IgnoreCase) ||
            Name.Contains(TEXT("RPG"), ESearchCase::IgnoreCase))
        {
            return bAlternate
                ? FLinearColor(0.040f, 0.045f, 0.045f, 1.0f)
                : FLinearColor(0.10f, 0.13f, 0.060f, 1.0f);
        }
        return bAlternate
            ? FLinearColor(0.030f, 0.035f, 0.040f, 1.0f)
            : FLinearColor(0.060f, 0.070f, 0.080f, 1.0f);
    }

    int32 ApplyPalette(
        AOCWeaponBase& Weapon,
        UMaterialInterface* BaseMaterial,
        int32& OutRestoredPlaceholderSlots,
        int32& OutOtherPlaceholderSlots)
    {
        OutRestoredPlaceholderSlots = 0;
        OutOtherPlaceholderSlots = 0;
        if (!BaseMaterial) return 0;

        const FString Name = Weapon.GetWeaponDisplayName();
        const bool bKnownRestoredPayload = IsRestoredSteinPayload(Name);
        TInlineComponentArray<UMeshComponent*> Components;
        Weapon.GetComponents(Components);

        int32 RelevantVisuals = 0;
        for (UMeshComponent* Component : Components)
        {
            if (!IsValid(Component)) continue;
            if (!Component->ComponentHasTag(ProductionVisualTag) &&
                !Component->ComponentHasTag(RealFallbackComponentTag))
            {
                continue;
            }

            ++RelevantVisuals;
            Component->SetCastShadow(false);
            Component->SetCanEverAffectNavigation(false);

            const int32 SlotCount = FMath::Max(1, Component->GetNumMaterials());
            for (int32 Slot = 0; Slot < SlotCount; ++Slot)
            {
                UMaterialInterface* Current = Component->GetMaterial(Slot);
                const bool bPlaceholder = IsClearlyPlaceholderMaterial(Current);

                // Pass 38: never repaint a real non-placeholder material. The Pass 37 forced path caused
                // the flat orange Lever Action and other toy-like screenshots by replacing valid imported
                // assignments merely because the asset came from a known incomplete restored folder.
                if (!bPlaceholder) continue;

                UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(
                    BaseMaterial,
                    &Weapon,
                    MakeUniqueObjectName(
                        &Weapon,
                        UMaterialInstanceDynamic::StaticClass(),
                        FName(*FString::Printf(TEXT("PASS38_WeaponPlaceholder_%d"), Slot))));
                if (!MID) continue;

                MID->SetVectorParameterValue(TEXT("Color"), ResolvePaletteColor(Name, Slot));
                Component->SetMaterial(Slot, MID);
                if (bKnownRestoredPayload) ++OutRestoredPlaceholderSlots;
                else ++OutOtherPlaceholderSlots;
            }
        }
        return RelevantVisuals;
    }
}

bool UOCWeaponPalettePass37Subsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCWeaponPalettePass37Subsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    if (InWorld.GetNetMode() == NM_DedicatedServer) return;
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    PaletteBaseMaterial = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

    InWorld.GetTimerManager().SetTimer(
        AuditTimer,
        this,
        &UOCWeaponPalettePass37Subsystem::AuditRackWeapons,
        0.50f,
        true,
        0.35f);
}

void UOCWeaponPalettePass37Subsystem::Deinitialize()
{
    if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(AuditTimer);
    PaletteBaseMaterial = nullptr;
    AuditPassCount = 0;
    Super::Deinitialize();
}

void UOCWeaponPalettePass37Subsystem::AuditRackWeapons()
{
    UWorld* World = GetWorld();
    if (!World || !PaletteBaseMaterial) return;

    ++AuditPassCount;

    int32 RackWeapons = 0;
    int32 AuditedWeapons = 0;
    int32 RestoredPlaceholderSlots = 0;
    int32 OtherPlaceholderSlots = 0;

    for (TActorIterator<AOCWeaponBase> It(World); It; ++It)
    {
        AOCWeaponBase* Weapon = *It;
        if (!IsValid(Weapon) || Weapon->IsActorBeingDestroyed() || !Weapon->ActorHasTag(RuntimeBaseRackTag)) continue;
        ++RackWeapons;

        if (Weapon->ActorHasTag(PaletteAuditedTag))
        {
            ++AuditedWeapons;
            continue;
        }

        const FString Name = Weapon->GetWeaponDisplayName();
        if (IsAK(Name))
        {
            Weapon->Tags.Add(PaletteAuditedTag);
            ++AuditedWeapons;
            continue;
        }

        int32 RestoredSlots = 0;
        int32 OtherSlots = 0;
        const int32 Visuals = ApplyPalette(*Weapon, PaletteBaseMaterial, RestoredSlots, OtherSlots);
        if (Visuals <= 0) continue;

        RestoredPlaceholderSlots += RestoredSlots;
        OtherPlaceholderSlots += OtherSlots;
        Weapon->Tags.Add(PaletteAuditedTag);
        ++AuditedWeapons;

        if (RestoredSlots > 0 || OtherSlots > 0)
        {
            UE_LOG(LogTemp, Warning,
                TEXT("PASS37_WEAPON_VISIBLE_PALETTE_APPLIED weapon=%s restored_placeholder_slots=%d other_placeholder_slots=%d authored_materials_preserved=1"),
                *Name, RestoredSlots, OtherSlots);
        }
    }

    const bool bRackReady = RackWeapons >= RequiredRackWeaponCountPerTeam &&
        RackWeapons <= MaxExpectedRackWeapons && AuditedWeapons == RackWeapons;

    if (bRackReady)
    {
        World->GetTimerManager().ClearTimer(AuditTimer);
        UE_LOG(LogTemp, Display,
            TEXT("PASS37_WEAPON_VISIBLE_PALETTE_READY rack_weapons=%d audited=%d restored_placeholder_slots_last_pass=%d other_placeholder_slots_last_pass=%d ak_authored_preserved=1 authored_materials_preserved=1"),
            RackWeapons, AuditedWeapons, RestoredPlaceholderSlots, OtherPlaceholderSlots);
        UE_LOG(LogTemp, Display,
            TEXT("PASS38_WEAPON_PALETTE_SCAN_STOPPED reason=ready passes=%d rack_weapons=%d audited=%d permanent_scan=0"),
            AuditPassCount, RackWeapons, AuditedWeapons);
        return;
    }

    if (AuditPassCount >= MaxAuditPasses)
    {
        World->GetTimerManager().ClearTimer(AuditTimer);
        UE_LOG(LogTemp, Error,
            TEXT("PASS38_WEAPON_PALETTE_SCAN_BOUNDED_STOP passes=%d max_passes=%d rack_weapons=%d audited=%d permanent_scan=0"),
            AuditPassCount, MaxAuditPasses, RackWeapons, AuditedWeapons);
    }
}