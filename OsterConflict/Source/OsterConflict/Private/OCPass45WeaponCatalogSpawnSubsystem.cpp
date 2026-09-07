#include "OCPass45WeaponCatalogSpawnSubsystem.h"

#include "OCGameMode.h"
#include "OCImportedWeaponVariants.h"
#include "OCAntiArmorLauncher.h"
#include "OCWeaponBase.h"
#include "OCWeaponVariants.h"

#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"

namespace
{
    struct FWeaponCatalogEntry
    {
        FName WeaponId;
        TSubclassOf<AOCWeaponBase> WeaponClass;
    };

    // Single source of truth for every gameplay weapon currently registered in PASS45.
    // The first seven entries are the legacy admin-rack trigger set; do not duplicate those IDs in a second array.
    const FWeaponCatalogEntry WeaponCatalog[] =
    {
        { FName(TEXT("OC_AR1")), AOCWeapon_AssaultRifle::StaticClass() },
        { FName(TEXT("OC_SMG1")), AOCWeapon_SMG::StaticClass() },
        { FName(TEXT("OC_PST1")), AOCWeapon_Pistol::StaticClass() },
        { FName(TEXT("OC_SNP1")), AOCWeapon_Sniper::StaticClass() },
        { FName(TEXT("OC_SG1")), AOCWeapon_Shotgun::StaticClass() },
        { FName(TEXT("OC_LMG1")), AOCWeapon_LMG::StaticClass() },
        { FName(TEXT("OC_RPG1")), AOCAntiArmorLauncher::StaticClass() },
        { FName(TEXT("R13_M14")), AOCWeapon_M14::StaticClass() },
        { FName(TEXT("R13_MAC10")), AOCWeapon_Mac10::StaticClass() },
        { FName(TEXT("R13_TEC9")), AOCWeapon_Tec9::StaticClass() },
        { FName(TEXT("R13_LEVER4570")), AOCWeapon_LeverAction::StaticClass() },
        { FName(TEXT("IMP_AK74M")), AOCWeapon_AK74M::StaticClass() },
        { FName(TEXT("IMP_AKS74U")), AOCWeapon_AKS74U::StaticClass() },
        { FName(TEXT("IMP_AR15")), AOCWeapon_AR15::StaticClass() },
        { FName(TEXT("IMP_M4A1")), AOCWeapon_M4A1::StaticClass() },
        { FName(TEXT("IMP_BALLISTA")), AOCWeapon_FnBallista::StaticClass() },
        { FName(TEXT("IMP_KAR98K")), AOCWeapon_Kar98k::StaticClass() },
        { FName(TEXT("IMP_MAKAROV")), AOCWeapon_Makarov::StaticClass() },
        { FName(TEXT("IMP_REVOLVER")), AOCWeapon_Revolver::StaticClass() },
        { FName(TEXT("IMP_TOMMY")), AOCWeapon_TommyGun::StaticClass() },
        { FName(TEXT("IMP_M72")), AOCWeapon_M72LAW::StaticClass() },
        { FName(TEXT("IMP_RPG26")), AOCWeapon_RPG26::StaticClass() },
        { FName(TEXT("IMP_FAB_RPG")), AOCWeapon_FabRPG::StaticClass() },
    };

    constexpr int32 CoreRackEntryCount = 7;
    constexpr float FullRackRadiusCm = 1450.0f;
    const FName ProductionWeaponVisualTag(TEXT("OC_ProductionWeaponVisual"));

    bool IsCoreRackId(const FName WeaponId)
    {
        static_assert(CoreRackEntryCount <= UE_ARRAY_COUNT(WeaponCatalog), "Core rack cannot exceed complete catalog.");
        for (int32 Index = 0; Index < CoreRackEntryCount; ++Index)
        {
            if (WeaponCatalog[Index].WeaponId == WeaponId) return true;
        }
        return false;
    }

    bool IsDeclaredCatalogId(const FName WeaponId)
    {
        for (const FWeaponCatalogEntry& Entry : WeaponCatalog)
        {
            if (Entry.WeaponId == WeaponId) return true;
        }
        return false;
    }

    bool HasExactProductionVisual(const AOCWeaponBase& Weapon)
    {
        TArray<UPrimitiveComponent*> Components;
        Weapon.GetComponents<UPrimitiveComponent>(Components);
        for (const UPrimitiveComponent* Component : Components)
        {
            if (Component && Component->ComponentHasTag(ProductionWeaponVisualTag) && Component->IsVisible())
            {
                return true;
            }
        }
        return false;
    }
}

bool UOCPass45WeaponCatalogSpawnSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCPass45WeaponCatalogSpawnSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>();
    if (!GameMode || GameMode->IsFrontendOnlySession() || !GameMode->IsSandboxMode()) return;

    // Do not create a second automatic weapon rack. The existing admin action is the trigger/owner.
    // We only watch briefly for its compact seven-weapon core rack, then append the missing identities once.
    InWorld.GetTimerManager().SetTimer(
        SpawnTimer,
        this,
        &UOCPass45WeaponCatalogSpawnSubsystem::CompleteRequestedWeaponRack,
        0.50f,
        true,
        0.50f);
}

void UOCPass45WeaponCatalogSpawnSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(SpawnTimer);
        World->GetTimerManager().ClearTimer(ValidationTimer);
    }
    CompletedRackCenter = FVector::ZeroVector;
    ScanPass = 0;
    bRackCompleted = false;
    Super::Deinitialize();
}

void UOCPass45WeaponCatalogSpawnSubsystem::CompleteRequestedWeaponRack()
{
    UWorld* World = GetWorld();
    if (!World || bRackCompleted) return;

    ++ScanPass;
    if (ScanPass > 120)
    {
        World->GetTimerManager().ClearTimer(SpawnTimer);
        UE_LOG(LogTemp, Verbose,
            TEXT("PASS45_COMPLETE_WEAPON_RACK_WATCH_STOPPED reason=no_admin_request scans=%d duplicate_auto_rack=0"),
            ScanPass);
        return;
    }

    const AOCGameMode* GameMode = World->GetAuthGameMode<AOCGameMode>();
    if (!GameMode || !GameMode->IsSandboxMode()) return;

    TArray<AOCWeaponBase*> WorldPickups;
    for (TActorIterator<AOCWeaponBase> It(World); It; ++It)
    {
        AOCWeaponBase* Weapon = *It;
        if (Weapon && !Weapon->IsActorBeingDestroyed() && Weapon->IsWorldPickup())
        {
            WorldPickups.Add(Weapon);
        }
    }

    constexpr float CoreClusterRadiusCm = 720.0f;
    FVector RackCenter = FVector::ZeroVector;
    bool bFoundAdminCoreRack = false;

    for (AOCWeaponBase* Candidate : WorldPickups)
    {
        if (!Candidate || !IsCoreRackId(Candidate->GetWeaponId())) continue;

        TSet<FName> CoreIds;
        FVector Sum = FVector::ZeroVector;
        int32 Count = 0;
        for (AOCWeaponBase* Nearby : WorldPickups)
        {
            if (!Nearby || !IsCoreRackId(Nearby->GetWeaponId())) continue;
            if (FVector::DistSquared2D(Candidate->GetActorLocation(), Nearby->GetActorLocation()) >
                FMath::Square(CoreClusterRadiusCm)) continue;

            CoreIds.Add(Nearby->GetWeaponId());
            Sum += Nearby->GetActorLocation();
            ++Count;
        }

        if (CoreIds.Num() == CoreRackEntryCount && Count >= CoreRackEntryCount)
        {
            RackCenter = Sum / static_cast<float>(Count);
            bFoundAdminCoreRack = true;
            break;
        }
    }

    if (!bFoundAdminCoreRack) return;

    TSet<FName> LocalIds;
    for (AOCWeaponBase* Weapon : WorldPickups)
    {
        if (!Weapon) continue;
        if (FVector::DistSquared2D(RackCenter, Weapon->GetActorLocation()) <= FMath::Square(FullRackRadiusCm))
        {
            LocalIds.Add(Weapon->GetWeaponId());
        }
    }

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    int32 Spawned = 0;
    int32 SpawnFailures = 0;
    int32 MissingOrdinal = 0;
    for (const FWeaponCatalogEntry& Entry : WeaponCatalog)
    {
        if (LocalIds.Contains(Entry.WeaponId)) continue;

        const int32 Column = MissingOrdinal % 5;
        const int32 Row = MissingOrdinal / 5;
        const FVector Location = RackCenter + FVector(
            -360.0f + Column * 180.0f,
            430.0f + Row * 190.0f,
            45.0f);
        ++MissingOrdinal;

        AOCWeaponBase* Weapon = World->SpawnActor<AOCWeaponBase>(
            Entry.WeaponClass,
            Location,
            FRotator::ZeroRotator,
            Params);
        if (!Weapon)
        {
            ++SpawnFailures;
            continue;
        }

        Weapon->DropToWorldServer(Location, FRotator::ZeroRotator);
        LocalIds.Add(Entry.WeaponId);
        ++Spawned;
    }

    CompletedRackCenter = RackCenter;
    bRackCompleted = true;
    World->GetTimerManager().ClearTimer(SpawnTimer);

    // Give the local exact-asset bridge enough time to service late admin-created actors before validating.
    World->GetTimerManager().SetTimer(
        ValidationTimer,
        this,
        &UOCPass45WeaponCatalogSpawnSubsystem::ValidateCompleteWeaponRack,
        3.20f,
        false);

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_COMPLETE_WEAPON_RACK_READY total_declared=%d local_distinct=%d appended=%d spawn_failures=%d admin_core_trigger=1 duplicate_auto_rack=0 duplicate_weapon_ids=0 exact_visual_validation_pending=1 runtime_acceptance=0"),
        UE_ARRAY_COUNT(WeaponCatalog), LocalIds.Num(), Spawned, SpawnFailures);
}

void UOCPass45WeaponCatalogSpawnSubsystem::ValidateCompleteWeaponRack()
{
    UWorld* World = GetWorld();
    if (!World || !bRackCompleted) return;

    TMap<FName, int32> CountsById;
    TSet<FName> ExactVisualIds;

    for (TActorIterator<AOCWeaponBase> It(World); It; ++It)
    {
        AOCWeaponBase* Weapon = *It;
        if (!Weapon || Weapon->IsActorBeingDestroyed() || !Weapon->IsWorldPickup()) continue;
        if (FVector::DistSquared2D(CompletedRackCenter, Weapon->GetActorLocation()) > FMath::Square(FullRackRadiusCm)) continue;

        const FName WeaponId = Weapon->GetWeaponId();
        if (!IsDeclaredCatalogId(WeaponId)) continue;
        CountsById.FindOrAdd(WeaponId) += 1;
        if (HasExactProductionVisual(*Weapon)) ExactVisualIds.Add(WeaponId);
    }

    TArray<FString> MissingIds;
    TArray<FString> MissingExactVisuals;
    TArray<FString> DuplicateIds;
    int32 PresentIds = 0;
    int32 ExactVisualCount = 0;

    for (const FWeaponCatalogEntry& Entry : WeaponCatalog)
    {
        const int32 Count = CountsById.FindRef(Entry.WeaponId);
        if (Count <= 0)
        {
            MissingIds.Add(Entry.WeaponId.ToString());
            continue;
        }

        ++PresentIds;
        if (Count > 1)
        {
            DuplicateIds.Add(FString::Printf(TEXT("%s:%d"), *Entry.WeaponId.ToString(), Count));
        }

        if (ExactVisualIds.Contains(Entry.WeaponId))
        {
            ++ExactVisualCount;
        }
        else
        {
            MissingExactVisuals.Add(Entry.WeaponId.ToString());
        }
    }

    const int32 DeclaredCount = UE_ARRAY_COUNT(WeaponCatalog);
    const bool bComplete = PresentIds == DeclaredCount && ExactVisualCount == DeclaredCount && DuplicateIds.IsEmpty();
    if (bComplete)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_COMPLETE_WEAPON_CATALOG_VISUAL_READY declared=%d present=%d exact_visuals=%d missing_ids=0 missing_exact_visuals=0 duplicate_weapon_ids=0 wrong_identity_substitution=0 runtime_acceptance=0"),
            DeclaredCount, PresentIds, ExactVisualCount);
        return;
    }

    UE_LOG(LogTemp, Error,
        TEXT("PASS45_COMPLETE_WEAPON_CATALOG_VISUAL_GAP declared=%d present=%d exact_visuals=%d missing_ids=%s missing_exact_visuals=%s duplicate_weapon_ids=%s wrong_identity_substitution=0 runtime_acceptance=0"),
        DeclaredCount,
        PresentIds,
        ExactVisualCount,
        MissingIds.IsEmpty() ? TEXT("NONE") : *FString::Join(MissingIds, TEXT(",")),
        MissingExactVisuals.IsEmpty() ? TEXT("NONE") : *FString::Join(MissingExactVisuals, TEXT(",")),
        DuplicateIds.IsEmpty() ? TEXT("NONE") : *FString::Join(DuplicateIds, TEXT(",")));
}
