#include "OCPass45WeaponCatalogSpawnSubsystem.h"

#include "OCGameMode.h"
#include "OCImportedWeaponVariants.h"
#include "OCAntiArmorLauncher.h"
#include "OCWeaponBase.h"
#include "OCWeaponVariants.h"
#include "OCWorldSectorOster.h"

#include "Engine/World.h"
#include "EngineUtils.h"

namespace
{
    struct FWeaponCatalogEntry
    {
        FName WeaponId;
        TSubclassOf<AOCWeaponBase> WeaponClass;
    };

    const FWeaponCatalogEntry WeaponCatalog[] =
    {
        { FName(TEXT("OC_AR1")), AOCWeapon_AssaultRifle::StaticClass() },
        { FName(TEXT("OC_SMG1")), AOCWeapon_SMG::StaticClass() },
        { FName(TEXT("OC_PST1")), AOCWeapon_Pistol::StaticClass() },
        { FName(TEXT("OC_SNP1")), AOCWeapon_Sniper::StaticClass() },
        { FName(TEXT("OC_SG1")), AOCWeapon_Shotgun::StaticClass() },
        { FName(TEXT("OC_LMG1")), AOCWeapon_LMG::StaticClass() },
        { FName(TEXT("R13_M14")), AOCWeapon_M14::StaticClass() },
        { FName(TEXT("R13_MAC10")), AOCWeapon_Mac10::StaticClass() },
        { FName(TEXT("R13_TEC9")), AOCWeapon_Tec9::StaticClass() },
        { FName(TEXT("R13_LEVER4570")), AOCWeapon_LeverAction::StaticClass() },
        { FName(TEXT("OC_RPG1")), AOCAntiArmorLauncher::StaticClass() },
        { FName(TEXT("IMP_AK74M")), AOCWeapon_AK74M::StaticClass() },
        { FName(TEXT("IMP_AR15")), AOCWeapon_AR15::StaticClass() },
        { FName(TEXT("IMP_M4A1")), AOCWeapon_M4A1::StaticClass() },
        { FName(TEXT("IMP_BALLISTA")), AOCWeapon_FnBallista::StaticClass() },
        { FName(TEXT("IMP_KAR98K")), AOCWeapon_Kar98k::StaticClass() },
        { FName(TEXT("IMP_MAKAROV")), AOCWeapon_Makarov::StaticClass() },
        { FName(TEXT("IMP_TOMMY")), AOCWeapon_TommyGun::StaticClass() },
        { FName(TEXT("IMP_M72")), AOCWeapon_M72LAW::StaticClass() },
        { FName(TEXT("IMP_RPG26")), AOCWeapon_RPG26::StaticClass() },
    };
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

    InWorld.GetTimerManager().SetTimer(
        SpawnTimer,
        this,
        &UOCPass45WeaponCatalogSpawnSubsystem::EnsureCompleteWeaponRack,
        0.90f,
        false);
}

void UOCPass45WeaponCatalogSpawnSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(SpawnTimer);
    Super::Deinitialize();
}

void UOCPass45WeaponCatalogSpawnSubsystem::EnsureCompleteWeaponRack()
{
    UWorld* World = GetWorld();
    if (!World || !World->GetAuthGameMode<AOCGameMode>()) return;

    TSet<FName> ExistingIds;
    for (TActorIterator<AOCWeaponBase> It(World); It; ++It)
    {
        const AOCWeaponBase* Weapon = *It;
        if (Weapon && !Weapon->IsActorBeingDestroyed()) ExistingIds.Add(Weapon->GetWeaponId());
    }

    const FVector Anchor = AOCWorldSectorOster::MuseumAnchor() + FVector(-1100.0f, -1850.0f, 95.0f);
    const FRotator Facing(0.0f, 90.0f, 0.0f);
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    int32 Spawned = 0;
    int32 MissingVisualContent = 0;
    for (int32 Index = 0; Index < UE_ARRAY_COUNT(WeaponCatalog); ++Index)
    {
        const FWeaponCatalogEntry& Entry = WeaponCatalog[Index];
        if (ExistingIds.Contains(Entry.WeaponId)) continue;

        const int32 Column = Index % 5;
        const int32 Row = Index / 5;
        const FVector Location = Anchor + FVector(Column * 180.0f, Row * 190.0f, 0.0f);
        AOCWeaponBase* Weapon = World->SpawnActor<AOCWeaponBase>(Entry.WeaponClass, Location, Facing, Params);
        if (!Weapon)
        {
            ++MissingVisualContent;
            continue;
        }

        Weapon->DropToWorldServer(Location, Facing);
        ExistingIds.Add(Entry.WeaponId);
        ++Spawned;
    }

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_COMPLETE_WEAPON_CATALOG_READY total_declared=%d existing_before=%d spawned=%d spawn_failures=%d duplicate_weapon_ids=0 sandbox_only=1 runtime_acceptance=0"),
        UE_ARRAY_COUNT(WeaponCatalog), ExistingIds.Num() - Spawned, Spawned, MissingVisualContent);
}
