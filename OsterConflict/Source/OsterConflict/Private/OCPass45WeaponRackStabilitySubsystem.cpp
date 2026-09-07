#include "OCPass45WeaponRackStabilitySubsystem.h"

#include "OCGameMode.h"
#include "OCWeaponBase.h"

#include "Components/MeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"

namespace
{
    const FName ProductionVisualTag(TEXT("OC_ProductionWeaponVisual"));
    const FName ExactImportedVisualTag(TEXT("OC_PASS45_LOCAL_IMPORTED_WEAPON"));
    const FName LocalInboxVisualTag(TEXT("OC_LocalInboxWeaponVisual"));
    const FName RealFallbackVisualTag(TEXT("OC_RealFallbackWeaponVisual"));
    const FName StableRackTag(TEXT("OC_StableWeaponRackPickup"));
    const FName RetiredCompetingVisualTag(TEXT("OC_RetiredCompetingWeaponVisual"));

    const FName CoreRackIds[] =
    {
        FName(TEXT("OC_AR1")),
        FName(TEXT("OC_SMG1")),
        FName(TEXT("OC_PST1")),
        FName(TEXT("OC_SNP1")),
        FName(TEXT("OC_SG1")),
        FName(TEXT("OC_LMG1")),
        FName(TEXT("OC_RPG1"))
    };

    constexpr float CoreClusterRadiusCm = 720.0f;
    constexpr float FullRackRadiusCm = 1450.0f;
    constexpr float RefreshIntervalSeconds = 0.40f;
    constexpr int32 MaxRefreshPasses = 18;

    bool IsCoreRackId(const FName WeaponId)
    {
        for (const FName& CoreId : CoreRackIds)
        {
            if (CoreId == WeaponId) return true;
        }
        return false;
    }

    bool IsEngineBasicShape(const UStaticMeshComponent* Component)
    {
        if (!IsValid(Component) || !IsValid(Component->GetStaticMesh())) return false;
        return Component->GetStaticMesh()->GetPathName().Contains(
            TEXT("/Engine/BasicShapes/"), ESearchCase::IgnoreCase);
    }
}

bool UOCPass45WeaponRackStabilitySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCPass45WeaponRackStabilitySubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    if (InWorld.GetNetMode() == NM_DedicatedServer) return;
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>();
    if (GameMode)
    {
        if (GameMode->IsFrontendOnlySession() || !GameMode->IsSandboxMode()) return;
    }
    else if (InWorld.GetNetMode() != NM_Client)
    {
        return;
    }

    bSandboxActive = true;
    ActorSpawnedHandle = InWorld.AddOnActorSpawnedHandler(
        FOnActorSpawned::FDelegate::CreateUObject(this, &UOCPass45WeaponRackStabilitySubsystem::HandleActorSpawned));

    // Initial map weapons are not automatically treated as an arsenal. The bounded scan only activates
    // if the compact seven-identity admin rack is actually present.
    ScheduleRefresh(0.05f);
}

void UOCPass45WeaponRackStabilitySubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(RefreshTimer);
        if (ActorSpawnedHandle.IsValid()) World->RemoveOnActorSpawnedHandler(ActorSpawnedHandle);
    }

    ActorSpawnedHandle.Reset();
    RefreshPass = 0;
    bSandboxActive = false;
    Super::Deinitialize();
}

void UOCPass45WeaponRackStabilitySubsystem::HandleActorSpawned(AActor* Actor)
{
    if (!bSandboxActive || !Cast<AOCWeaponBase>(Actor)) return;

    // SpawnWeaponRack creates actors synchronously and calls DropToWorldServer immediately afterwards.
    // Debouncing the burst to the next frame preserves the ordinary DropToWorldServer contract while preventing
    // showcase actors from spending meaningful time under random rigid-body motion.
    ScheduleRefresh(0.01f);
}

void UOCPass45WeaponRackStabilitySubsystem::ScheduleRefresh(float FirstDelaySeconds)
{
    UWorld* World = GetWorld();
    if (!World || !bSandboxActive) return;

    RefreshPass = 0;
    World->GetTimerManager().ClearTimer(RefreshTimer);
    World->GetTimerManager().SetTimer(
        RefreshTimer,
        this,
        &UOCPass45WeaponRackStabilitySubsystem::RefreshRack,
        RefreshIntervalSeconds,
        true,
        FMath::Max(0.01f, FirstDelaySeconds));
}

bool UOCPass45WeaponRackStabilitySubsystem::FindAdminRack(
    TArray<AOCWeaponBase*>& OutRackWeapons,
    FVector& OutRackCenter,
    float& OutRackYaw) const
{
    UWorld* World = GetWorld();
    if (!World) return false;

    TArray<AOCWeaponBase*> WorldPickups;
    for (TActorIterator<AOCWeaponBase> It(World); It; ++It)
    {
        AOCWeaponBase* Weapon = *It;
        if (Weapon && !Weapon->IsActorBeingDestroyed() && Weapon->IsWorldPickup())
        {
            WorldPickups.Add(Weapon);
        }
    }

    bool bFoundCore = false;
    for (AOCWeaponBase* Candidate : WorldPickups)
    {
        if (!Candidate || !IsCoreRackId(Candidate->GetWeaponId())) continue;

        TSet<FName> DistinctCoreIds;
        FVector CoreLocationSum = FVector::ZeroVector;
        int32 CoreActorCount = 0;
        for (AOCWeaponBase* Nearby : WorldPickups)
        {
            if (!Nearby || !IsCoreRackId(Nearby->GetWeaponId())) continue;
            if (FVector::DistSquared2D(Candidate->GetActorLocation(), Nearby->GetActorLocation()) >
                FMath::Square(CoreClusterRadiusCm))
            {
                continue;
            }

            DistinctCoreIds.Add(Nearby->GetWeaponId());
            CoreLocationSum += Nearby->GetActorLocation();
            ++CoreActorCount;
        }

        if (DistinctCoreIds.Num() == UE_ARRAY_COUNT(CoreRackIds) &&
            CoreActorCount >= UE_ARRAY_COUNT(CoreRackIds))
        {
            OutRackCenter = CoreLocationSum / static_cast<float>(CoreActorCount);
            OutRackYaw = Candidate->GetActorRotation().Yaw;
            bFoundCore = true;
            break;
        }
    }

    if (!bFoundCore) return false;

    for (AOCWeaponBase* Weapon : WorldPickups)
    {
        if (!Weapon) continue;
        if (FVector::DistSquared2D(OutRackCenter, Weapon->GetActorLocation()) <= FMath::Square(FullRackRadiusCm))
        {
            OutRackWeapons.Add(Weapon);
        }
    }

    return OutRackWeapons.Num() >= UE_ARRAY_COUNT(CoreRackIds);
}

int32 UOCPass45WeaponRackStabilitySubsystem::StabilizeRackWeapon(
    AOCWeaponBase& Weapon,
    const FVector& Location,
    const FRotator& Rotation,
    int32& OutHiddenBasicShapes,
    int32& OutRetiredCompetingVisuals,
    bool& bOutExactVisual) const
{
    OutHiddenBasicShapes = 0;
    OutRetiredCompetingVisuals = 0;
    bOutExactVisual = false;

    Weapon.SetActorLocationAndRotation(Location, Rotation, false, nullptr, ETeleportType::TeleportPhysics);

    if (UPrimitiveComponent* PhysicsRoot = Cast<UPrimitiveComponent>(Weapon.GetRootComponent()))
    {
        PhysicsRoot->SetPhysicsLinearVelocity(FVector::ZeroVector);
        PhysicsRoot->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
        PhysicsRoot->SetSimulatePhysics(false);
        PhysicsRoot->SetEnableGravity(false);
        PhysicsRoot->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        PhysicsRoot->SetCollisionResponseToAllChannels(ECR_Block);
        PhysicsRoot->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
        PhysicsRoot->SetCanEverAffectNavigation(false);
    }

    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Weapon.GetComponents<UPrimitiveComponent>(PrimitiveComponents);

    UPrimitiveComponent* ExactVisual = nullptr;
    for (UPrimitiveComponent* Component : PrimitiveComponents)
    {
        if (IsValid(Component) && Component->ComponentHasTag(ExactImportedVisualTag))
        {
            ExactVisual = Component;
            bOutExactVisual = true;
            break;
        }
    }

    TArray<UStaticMeshComponent*> StaticComponents;
    Weapon.GetComponents<UStaticMeshComponent>(StaticComponents);
    for (UStaticMeshComponent* Component : StaticComponents)
    {
        if (!IsEngineBasicShape(Component)) continue;
        if (Component->IsVisible()) ++OutHiddenBasicShapes;
        Component->SetVisibility(false, false);
        Component->SetHiddenInGame(true, false);
        Component->SetCastShadow(false);
        Component->SetCanEverAffectNavigation(false);
        // Root collision remains the invisible pickup/interaction authority. Never turn it into visible art.
    }

    if (ExactVisual)
    {
        ExactVisual->SetVisibility(true, true);
        ExactVisual->SetHiddenInGame(false, true);
        ExactVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        ExactVisual->SetGenerateOverlapEvents(false);
        ExactVisual->SetCanEverAffectNavigation(false);

        for (UPrimitiveComponent* Component : PrimitiveComponents)
        {
            if (!IsValid(Component) || Component == ExactVisual || Component == Weapon.GetRootComponent()) continue;

            const bool bCompetingVisual =
                Component->ComponentHasTag(LocalInboxVisualTag) ||
                Component->ComponentHasTag(RealFallbackVisualTag) ||
                Component->ComponentHasTag(ProductionVisualTag);
            if (!bCompetingVisual) continue;

            if (Component->IsVisible()) ++OutRetiredCompetingVisuals;
            Component->SetVisibility(false, false);
            Component->SetHiddenInGame(true, false);
            Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            Component->SetGenerateOverlapEvents(false);
            Component->SetCastShadow(false);
            Component->SetCanEverAffectNavigation(false);
            Component->ComponentTags.AddUnique(RetiredCompetingVisualTag);
        }
    }

    Weapon.Tags.AddUnique(StableRackTag);
    return 1;
}

void UOCPass45WeaponRackStabilitySubsystem::RefreshRack()
{
    UWorld* World = GetWorld();
    if (!World || !bSandboxActive) return;

    ++RefreshPass;

    TArray<AOCWeaponBase*> RackWeapons;
    FVector RackCenter = FVector::ZeroVector;
    float RackYaw = 0.0f;
    if (!FindAdminRack(RackWeapons, RackCenter, RackYaw))
    {
        if (RefreshPass >= MaxRefreshPasses)
        {
            World->GetTimerManager().ClearTimer(RefreshTimer);
            UE_LOG(LogTemp, Verbose,
                TEXT("PASS45_WEAPON_RACK_STABILITY_WATCH_STOPPED reason=no_admin_rack passes=%d permanent_scan=0"),
                RefreshPass);
        }
        return;
    }

    int32 Stabilized = 0;
    int32 ExactVisuals = 0;
    int32 HiddenBasicShapes = 0;
    int32 RetiredCompetingVisuals = 0;
    int32 SimulatingPhysicsAfter = 0;

    const FRotator StableRotation(0.0f, RackYaw, 0.0f);
    for (AOCWeaponBase* Weapon : RackWeapons)
    {
        if (!Weapon || Weapon->IsActorBeingDestroyed() || !Weapon->IsWorldPickup()) continue;

        int32 HiddenForWeapon = 0;
        int32 RetiredForWeapon = 0;
        bool bExactVisual = false;
        Stabilized += StabilizeRackWeapon(
            *Weapon,
            Weapon->GetActorLocation(),
            StableRotation,
            HiddenForWeapon,
            RetiredForWeapon,
            bExactVisual);
        HiddenBasicShapes += HiddenForWeapon;
        RetiredCompetingVisuals += RetiredForWeapon;
        ExactVisuals += bExactVisual ? 1 : 0;

        if (const UPrimitiveComponent* PhysicsRoot = Cast<UPrimitiveComponent>(Weapon->GetRootComponent()))
        {
            if (PhysicsRoot->IsSimulatingPhysics()) ++SimulatingPhysicsAfter;
        }
    }

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_WEAPON_RACK_STABILITY_READY pass=%d rack_weapons=%d stabilized=%d simulating_physics_after=%d hidden_basicshape_components=%d exact_visuals=%d competing_visuals_retired=%d stable_pickup_collision=query_only runtime_acceptance=0"),
        RefreshPass,
        RackWeapons.Num(),
        Stabilized,
        SimulatingPhysicsAfter,
        HiddenBasicShapes,
        ExactVisuals,
        RetiredCompetingVisuals);

    if (RefreshPass >= MaxRefreshPasses)
    {
        World->GetTimerManager().ClearTimer(RefreshTimer);
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_WEAPON_RACK_STABILITY_WATCH_STOPPED reason=bounded_complete passes=%d rack_weapons=%d permanent_scan=0 runtime_acceptance=0"),
            RefreshPass, RackWeapons.Num());
    }
}
