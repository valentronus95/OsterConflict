#include "OCR146LandmarkSeparationSubsystem.h"

#include "OCGameMode.h"
#include "OCGeoReference.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"

namespace
{
    // Pass 45: all current landmark construction delays finish before this one-shot reconciliation.
    // The old 0.20 s x 40 startup loop rescanned the whole world for ~8 seconds and repeatedly dirtied
    // render state. Keep one delayed reconciliation plus the cheap actor-spawn legacy guard instead.
    constexpr float SeparationValidationDelaySeconds = 6.25f;
    constexpr float MuseumCleanupRadiusCm = 2600.0f;
    constexpr float SilpoCleanupRadiusCm = 2600.0f;
    constexpr float CultureCleanupRadiusCm = 2600.0f;

    FVector GeoToWorld(const FOCGeoReferencePoint& Point)
    {
        return FOCGeoReference::ToLocalCm(Point.Latitude, Point.Longitude, 0.0);
    }

    bool IsGenericBuildingFamily(const FName Name)
    {
        return Name == TEXT("Buildings") || Name == TEXT("ResidentialRoofs") ||
            Name == TEXT("ResidentialDetails") || Name == TEXT("LandmarkBlocks") ||
            Name == TEXT("LandmarkRoofs") || Name == TEXT("LandmarkWindows") ||
            Name == TEXT("LandmarkDetails");
    }

    int32 RemoveInstancesNear(UInstancedStaticMeshComponent* Component, const FVector& Center, const float RadiusCm)
    {
        if (!Component) return 0;
        const float RadiusSq = FMath::Square(RadiusCm);
        int32 Removed = 0;
        for (int32 Index = Component->GetInstanceCount() - 1; Index >= 0; --Index)
        {
            FTransform Transform;
            if (!Component->GetInstanceTransform(Index, Transform, true)) continue;
            if (FVector::DistSquared2D(Transform.GetLocation(), Center) > RadiusSq) continue;
            if (Component->RemoveInstance(Index)) ++Removed;
        }
        if (Removed > 0) Component->MarkRenderStateDirty();
        return Removed;
    }
}

bool UOCR146LandmarkSeparationSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR146LandmarkSeparationSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    GuardWorld = &InWorld;
    StartupGuardPass = 0;

    // Actor-level late legacy owners are rejected for the whole lifetime of the runtime world.
    ActorSpawnedHandle = InWorld.AddOnActorSpawnedHandler(
        FOnActorSpawned::FDelegate::CreateUObject(this, &UOCR146LandmarkSeparationSubsystem::HandleActorSpawned));

    InWorld.GetTimerManager().SetTimer(
        StartupGuardTimer,
        this,
        &UOCR146LandmarkSeparationSubsystem::RunStartupGuardPass,
        SeparationValidationDelaySeconds,
        false);

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_LANDMARK_RECONCILIATION_BUDGET_READY startup_full_world_scans=1 legacy_spawn_guard=1 delay_s=%.2f"),
        SeparationValidationDelaySeconds);
}

void UOCR146LandmarkSeparationSubsystem::Deinitialize()
{
    if (UWorld* World = GuardWorld.Get())
    {
        World->GetTimerManager().ClearTimer(StartupGuardTimer);
        if (ActorSpawnedHandle.IsValid())
        {
            World->RemoveOnActorSpawnedHandler(ActorSpawnedHandle);
        }
    }
    ActorSpawnedHandle.Reset();
    GuardWorld.Reset();
    StartupGuardPass = 0;
    Super::Deinitialize();
}

void UOCR146LandmarkSeparationSubsystem::RunStartupGuardPass()
{
    UWorld* World = GuardWorld.Get();
    if (!World) return;

    ++StartupGuardPass;
    EnforceSeparation(*World, true);

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_LANDMARK_RECONCILIATION_COMPLETE full_world_scan_passes=%d further_periodic_scan=0"),
        StartupGuardPass);
}

bool UOCR146LandmarkSeparationSubsystem::IsForbiddenLegacyLandmarkActor(const AActor* Actor)
{
    if (!Actor) return false;
    if (Actor->ActorHasTag(TEXT("R13_CultureHousePhotoModel")) ||
        Actor->ActorHasTag(TEXT("R13_SilpoPhotoModel")))
    {
        return true;
    }

    const FString Name = Actor->GetName();
    return Name.Contains(TEXT("R13_CultureHousePhotoModel"), ESearchCase::IgnoreCase) ||
        Name.Contains(TEXT("R13_SilpoPhotoModel"), ESearchCase::IgnoreCase);
}

void UOCR146LandmarkSeparationSubsystem::HandleActorSpawned(AActor* Actor)
{
    UWorld* World = GuardWorld.Get();
    if (!World || !Actor || Actor->GetWorld() != World) return;

    // SpawnActor delegates run before a caller may append its runtime tags. Re-check on the next tick,
    // when a late legacy subsystem has finished initialising the actor identity.
    TWeakObjectPtr<AActor> WeakActor(Actor);
    World->GetTimerManager().SetTimerForNextTick(
        FTimerDelegate::CreateWeakLambda(this, [WeakActor]()
        {
            AActor* Spawned = WeakActor.Get();
            if (!Spawned || !IsForbiddenLegacyLandmarkActor(Spawned)) return;

            UE_LOG(LogTemp, Error,
                TEXT("Landmark ownership violation: destroying late legacy actor '%s'. Museum/Silpo/Culture each have one current owner."),
                *Spawned->GetName());
            Spawned->Destroy();
        }));
}

void UOCR146LandmarkSeparationSubsystem::EnforceSeparation(UWorld& World, const bool bFinalValidation) const
{
    const FVector Museum = GeoToWorld(FOCGeoReference::Museum());
    const FVector Silpo = GeoToWorld(FOCGeoReference::Silpo());
    const FVector CultureHouse = GeoToWorld(FOCGeoReference::CultureHouse());
    const FVector CentralPark = GeoToWorld(FOCGeoReference::CentralPark());
    const FVector NorthCivicReference = GeoToWorld(FOCGeoReference::CultureParkNorth());
    const FVector SyntheticParkLinkMid = (CentralPark + NorthCivicReference) * 0.5f;

    int32 LegacyActorsRemoved = 0;
    TArray<TWeakObjectPtr<AActor>> LegacyActors;
    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        if (AActor* Actor = *It; IsForbiddenLegacyLandmarkActor(Actor))
        {
            LegacyActors.Add(Actor);
        }
    }
    for (const TWeakObjectPtr<AActor>& WeakActor : LegacyActors)
    {
        if (AActor* Actor = WeakActor.Get())
        {
            Actor->Destroy();
            ++LegacyActorsRemoved;
        }
    }

    int32 MuseumRemoved = 0;
    int32 SilpoRemoved = 0;
    int32 CultureRemoved = 0;
    int32 SyntheticNorthCivicRemoved = 0;
    int32 SyntheticParkLinkRemoved = 0;
    int32 FamiliesTouched = 0;

    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor) continue;

        TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
        Actor->GetComponents(Components);
        for (UInstancedStaticMeshComponent* Component : Components)
        {
            if (!Component) continue;
            const FName Name = Component->GetFName();

            if (IsGenericBuildingFamily(Name))
            {
                const int32 Before = Component->GetInstanceCount();
                MuseumRemoved += RemoveInstancesNear(Component, Museum, MuseumCleanupRadiusCm);
                SilpoRemoved += RemoveInstancesNear(Component, Silpo, SilpoCleanupRadiusCm);
                CultureRemoved += RemoveInstancesNear(Component, CultureHouse, CultureCleanupRadiusCm);
                if (Component->GetInstanceCount() != Before) ++FamiliesTouched;
                continue;
            }

            if (Name == TEXT("ParkGeometry"))
            {
                SyntheticNorthCivicRemoved += RemoveInstancesNear(Component, NorthCivicReference, 1000.0f);
            }
            else if (Name == TEXT("Sidewalks"))
            {
                SyntheticParkLinkRemoved += RemoveInstancesNear(Component, SyntheticParkLinkMid, 1000.0f);
            }
        }
    }

    const int32 OwnershipViolationsRemoved = LegacyActorsRemoved + MuseumRemoved + SilpoRemoved + CultureRemoved;
    if (OwnershipViolationsRemoved > 0)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("Pass45 landmark reconciliation removed foreign geometry: legacyActors=%d Museum=%d Silpo=%d CultureHouse=%d families=%d."),
            LegacyActorsRemoved, MuseumRemoved, SilpoRemoved, CultureRemoved, FamiliesTouched);
    }

    if (SyntheticNorthCivicRemoved > 0 || SyntheticParkLinkRemoved > 0)
    {
        UE_LOG(LogTemp, Display,
            TEXT("Pass45 map cleanup removed synthetic north-civic grove=%d and unmapped park-link sidewalk=%d."),
            SyntheticNorthCivicRemoved, SyntheticParkLinkRemoved);
    }

    if (bFinalValidation)
    {
        const float MuseumToSilpoM = FVector::Dist2D(Museum, Silpo) / 100.0f;
        const float MuseumToCultureM = FVector::Dist2D(Museum, CultureHouse) / 100.0f;
        const float SilpoToCultureM = FVector::Dist2D(Silpo, CultureHouse) / 100.0f;

        if (OwnershipViolationsRemoved > 0)
        {
            UE_LOG(LogTemp, Warning,
                TEXT("PASS45_LANDMARK_OWNERSHIP_RECONCILED late_violations=%d periodic_rescan=0"),
                OwnershipViolationsRemoved);
        }
        else
        {
            UE_LOG(LogTemp, Display,
                TEXT("PASS45_LANDMARK_OWNERSHIP_CLEAN late_violations=0 periodic_rescan=0"));
        }

        UE_LOG(LogTemp, Display,
            TEXT("PASS45_LANDMARK_SITE_DISTANCES Museum-Silpo=%.1fm Museum-CultureHouse=%.1fm Silpo-CultureHouse=%.1fm"),
            MuseumToSilpoM, MuseumToCultureM, SilpoToCultureM);
    }
}
