#include "OCPass45WeaponRackStabilitySubsystem.h"

#include "OCGameMode.h"
#include "OCWeaponBase.h"

#include "CollisionQueryParams.h"
#include "Components/MeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/HitResult.h"
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

    constexpr float FullRackRadiusCm = 1450.0f;
    constexpr int32 MinimumRackActorCount = 3;
    constexpr float RefreshIntervalSeconds = 0.45f;
    constexpr int32 MaxRefreshPasses = 8;
    constexpr float MinimumRackLiftCm = 25.0f;
    constexpr float VisualGroundClearanceCm = 4.0f;
    constexpr float GroundTraceUpCm = 800.0f;
    constexpr float GroundTraceDownCm = 2400.0f;

    bool IsEngineBasicShape(const UStaticMeshComponent* Component)
    {
        if (!IsValid(Component) || !IsValid(Component->GetStaticMesh())) return false;
        return Component->GetStaticMesh()->GetPathName().Contains(
            TEXT("/Engine/BasicShapes/"), ESearchCase::IgnoreCase);
    }

    UPrimitiveComponent* FindAuthoritativeVisual(const TArray<UPrimitiveComponent*>& Components)
    {
        auto FindTagged = [&Components](const FName Tag) -> UPrimitiveComponent*
        {
            for (UPrimitiveComponent* Component : Components)
            {
                if (IsValid(Component) && Component->ComponentHasTag(Tag)) return Component;
            }
            return nullptr;
        };

        if (UPrimitiveComponent* Exact = FindTagged(ExactImportedVisualTag)) return Exact;
        if (UPrimitiveComponent* Local = FindTagged(LocalInboxVisualTag)) return Local;
        if (UPrimitiveComponent* Production = FindTagged(ProductionVisualTag)) return Production;
        return FindTagged(RealFallbackVisualTag);
    }

    bool IsManagedWeaponVisual(const UPrimitiveComponent* Component)
    {
        return IsValid(Component) &&
            (Component->ComponentHasTag(ExactImportedVisualTag) ||
             Component->ComponentHasTag(LocalInboxVisualTag) ||
             Component->ComponentHasTag(ProductionVisualTag) ||
             Component->ComponentHasTag(RealFallbackVisualTag));
    }

    bool GroundRenderedVisual(AOCWeaponBase& Weapon, UPrimitiveComponent* Visual)
    {
        UWorld* World = Weapon.GetWorld();
        if (!World) return false;

        FVector ActorLocation = Weapon.GetActorLocation();
        FCollisionQueryParams Params(SCENE_QUERY_STAT(OCWeaponRackGround), false, &Weapon);
        Params.AddIgnoredActor(&Weapon);

        FHitResult Hit;
        const FVector TraceStart(ActorLocation.X, ActorLocation.Y, ActorLocation.Z + GroundTraceUpCm);
        const FVector TraceEnd(ActorLocation.X, ActorLocation.Y, ActorLocation.Z - GroundTraceDownCm);
        if (!World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params)) return false;

        float VisualBottomZ = ActorLocation.Z;
        if (IsValid(Visual))
        {
            Visual->UpdateComponentToWorld();
            VisualBottomZ = Visual->Bounds.Origin.Z - Visual->Bounds.BoxExtent.Z;
        }
        else
        {
            FVector BoundsOrigin = FVector::ZeroVector;
            FVector BoundsExtent = FVector::ZeroVector;
            Weapon.GetActorBounds(false, BoundsOrigin, BoundsExtent, true);
            VisualBottomZ = BoundsOrigin.Z - BoundsExtent.Z;
        }

        const float DeltaZ = (Hit.ImpactPoint.Z + VisualGroundClearanceCm) - VisualBottomZ;
        if (!FMath::IsNearlyZero(DeltaZ, 0.5f))
        {
            ActorLocation.Z += DeltaZ;
            Weapon.SetActorLocation(ActorLocation, false, nullptr, ETeleportType::TeleportPhysics);
            if (IsValid(Visual)) Visual->UpdateComponentToWorld();
        }
        return true;
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

    // Bounded startup scan. Any compact sandbox weapon cluster is an arsenal; exact legacy IDs are irrelevant.
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

    // Catalog actors are spawned synchronously and then dropped. Debounce the burst and freeze/ground the whole rack.
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

    // GAME_RECOVERY: the previous implementation required seven exact legacy identities. If one was absent or
    // underground, none of the weapons were frozen, deduplicated or grounded. Pick the densest sandbox cluster instead.
    int32 BestClusterCount = 0;
    FVector BestClusterCenter = FVector::ZeroVector;
    float BestClusterYaw = 0.0f;
    for (AOCWeaponBase* Candidate : WorldPickups)
    {
        if (!Candidate) continue;

        FVector LocationSum = FVector::ZeroVector;
        int32 ClusterCount = 0;
        for (AOCWeaponBase* Nearby : WorldPickups)
        {
            if (!Nearby) continue;
            if (FVector::DistSquared2D(Candidate->GetActorLocation(), Nearby->GetActorLocation()) >
                FMath::Square(FullRackRadiusCm))
            {
                continue;
            }

            LocationSum += Nearby->GetActorLocation();
            ++ClusterCount;
        }

        if (ClusterCount > BestClusterCount)
        {
            BestClusterCount = ClusterCount;
            BestClusterCenter = LocationSum / static_cast<float>(ClusterCount);
            BestClusterYaw = Candidate->GetActorRotation().Yaw;
        }
    }

    if (BestClusterCount < MinimumRackActorCount) return false;

    OutRackCenter = BestClusterCenter;
    OutRackYaw = BestClusterYaw;
    for (AOCWeaponBase* Weapon : WorldPickups)
    {
        if (!Weapon) continue;
        if (FVector::DistSquared2D(OutRackCenter, Weapon->GetActorLocation()) <= FMath::Square(FullRackRadiusCm))
        {
            OutRackWeapons.Add(Weapon);
        }
    }

    return OutRackWeapons.Num() >= MinimumRackActorCount;
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

    UPrimitiveComponent* AuthoritativeVisual = FindAuthoritativeVisual(PrimitiveComponents);
    bOutExactVisual = IsValid(AuthoritativeVisual);

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
    }

    if (AuthoritativeVisual)
    {
        AuthoritativeVisual->SetVisibility(true, true);
        AuthoritativeVisual->SetHiddenInGame(false, true);
        AuthoritativeVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        AuthoritativeVisual->SetGenerateOverlapEvents(false);
        AuthoritativeVisual->SetCanEverAffectNavigation(false);

        for (UPrimitiveComponent* Component : PrimitiveComponents)
        {
            if (!IsValid(Component) || Component == AuthoritativeVisual || Component == Weapon.GetRootComponent()) continue;
            if (!IsManagedWeaponVisual(Component)) continue;

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

    GroundRenderedVisual(Weapon, AuthoritativeVisual);
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
                TEXT("PASS45_WEAPON_RACK_STABILITY_WATCH_STOPPED reason=no_sandbox_weapon_cluster passes=%d permanent_scan=0"),
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
    const float MinimumStableZ = RackCenter.Z + MinimumRackLiftCm;
    for (AOCWeaponBase* Weapon : RackWeapons)
    {
        if (!Weapon || Weapon->IsActorBeingDestroyed() || !Weapon->IsWorldPickup()) continue;

        FVector StableLocation = Weapon->GetActorLocation();
        StableLocation.Z = FMath::Max(StableLocation.Z, MinimumStableZ);

        int32 HiddenForWeapon = 0;
        int32 RetiredForWeapon = 0;
        bool bExactVisual = false;
        Stabilized += StabilizeRackWeapon(
            *Weapon,
            StableLocation,
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
        TEXT("PASS45_WEAPON_RACK_STABILITY_READY pass=%d rack_weapons=%d stabilized=%d simulating_physics_after=%d hidden_basicshape_components=%d authoritative_visuals=%d competing_visuals_retired=%d stable_pickup_collision=query_only grounding=rendered_visual_bottom legacy_core_requirement=0 runtime_acceptance=0"),
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
