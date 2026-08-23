#include "OCLandmarkShellOwnershipGuardSubsystem.h"

#include "OCGameMode.h"
#include "OCGeoReference.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"

namespace
{
    constexpr float FinalValidationDelaySeconds = 8.75f;
    constexpr float SiteGeometryRadiusCm = 5000.0f;

    const FName MuseumPrototypeTag(TEXT("R137_MuseumPhotoModel"));
    const FName MuseumArchitectureTag(TEXT("R138_MuseumHighFidelityArchitecture"));
    const FName SilpoShellTag(TEXT("R140_SilpoPhotoModel"));
    const FName SilpoEntranceDoorTag(TEXT("R140_SilpoEntranceDoor"));
    const FName CultureHouseShellTag(TEXT("R146_CultureHouseAuthoritative"));

    FVector GeoToWorld(const FOCGeoReferencePoint& Point)
    {
        return FOCGeoReference::ToLocalCm(Point.Latitude, Point.Longitude, 0.0);
    }
}

bool UOCLandmarkShellOwnershipGuardSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCLandmarkShellOwnershipGuardSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    GuardWorld = &InWorld;
    DuplicateRepairs = 0;

    ActorSpawnedHandle = InWorld.AddOnActorSpawnedHandler(
        FOnActorSpawned::FDelegate::CreateUObject(this, &UOCLandmarkShellOwnershipGuardSubsystem::HandleActorSpawned));

    InWorld.GetTimerManager().SetTimer(
        FinalValidationTimer,
        this,
        &UOCLandmarkShellOwnershipGuardSubsystem::RunFinalValidation,
        FinalValidationDelaySeconds,
        false);
}

void UOCLandmarkShellOwnershipGuardSubsystem::Deinitialize()
{
    if (UWorld* World = GuardWorld.Get())
    {
        World->GetTimerManager().ClearTimer(FinalValidationTimer);
        if (ActorSpawnedHandle.IsValid())
        {
            World->RemoveOnActorSpawnedHandler(ActorSpawnedHandle);
        }
    }

    ActorSpawnedHandle.Reset();
    GuardWorld.Reset();
    DuplicateRepairs = 0;
    Super::Deinitialize();
}

void UOCLandmarkShellOwnershipGuardSubsystem::HandleActorSpawned(AActor* Actor)
{
    UWorld* World = GuardWorld.Get();
    if (!World || !Actor || Actor->GetWorld() != World) return;

    // SpawnActor delegates execute before the caller appends runtime identity tags. Re-check on the next tick,
    // after the landmark stage has finished assigning tags/components, but before a duplicate can persist.
    TWeakObjectPtr<AActor> WeakActor(Actor);
    World->GetTimerManager().SetTimerForNextTick(
        FTimerDelegate::CreateWeakLambda(this, [this, WeakActor]()
        {
            EvaluateSpawnedActor(WeakActor);
        }));
}

void UOCLandmarkShellOwnershipGuardSubsystem::EvaluateSpawnedActor(TWeakObjectPtr<AActor> WeakActor)
{
    UWorld* World = GuardWorld.Get();
    AActor* Actor = WeakActor.Get();
    if (!World || !Actor || Actor->GetWorld() != World || Actor->IsActorBeingDestroyed()) return;

    if (Actor->ActorHasTag(MuseumPrototypeTag))
    {
        // Keep the older R13.7 owner: R13.8 has already suppressed/upgraded that exact prototype.
        DuplicateRepairs += RepairTaggedOwners(*World, MuseumPrototypeTag, TEXT("Museum-R13.7"), false);
    }
    else if (Actor->ActorHasTag(MuseumArchitectureTag))
    {
        // R13.8 architecture is deterministic; keep the first authoritative build and reject a late rebuild.
        DuplicateRepairs += RepairTaggedOwners(*World, MuseumArchitectureTag, TEXT("Museum-R13.8"), false);
    }
    else if (Actor->ActorHasTag(SilpoShellTag))
    {
        // R14.0 Silpo cleanup can hide the previous R140Silpo_* components before a delayed rebuild.
        // Keep the newest shell, then discard the hidden older owner.
        DuplicateRepairs += RepairTaggedOwners(*World, SilpoShellTag, TEXT("Silpo-R14.0"), true);
    }
    else if (Actor->ActorHasTag(CultureHouseShellTag))
    {
        DuplicateRepairs += RepairTaggedOwners(*World, CultureHouseShellTag, TEXT("CultureHouse-R14.6"), false);
    }

    if (World->GetNetMode() != NM_Client && Actor->ActorHasTag(SilpoEntranceDoorTag))
    {
        DuplicateRepairs += RepairTaggedOwners(*World, SilpoEntranceDoorTag, TEXT("SilpoEntranceDoor"), true);
    }
}

int32 UOCLandmarkShellOwnershipGuardSubsystem::RepairTaggedOwners(
    UWorld& World,
    FName OwnerTag,
    const TCHAR* SiteLabel,
    bool bKeepNewest)
{
    TArray<AActor*> Owners;
    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        AActor* Candidate = *It;
        if (!Candidate || Candidate->IsActorBeingDestroyed() || !Candidate->ActorHasTag(OwnerTag)) continue;
        Owners.Add(Candidate);
    }

    if (Owners.Num() <= 1) return 0;

    AActor* Keep = Owners[0];
    float KeepAge = Keep->GetGameTimeSinceCreation();
    for (int32 Index = 1; Index < Owners.Num(); ++Index)
    {
        AActor* Candidate = Owners[Index];
        const float CandidateAge = Candidate->GetGameTimeSinceCreation();
        const bool bPreferCandidate = bKeepNewest
            ? (CandidateAge < KeepAge)
            : (CandidateAge > KeepAge);
        if (bPreferCandidate)
        {
            Keep = Candidate;
            KeepAge = CandidateAge;
        }
    }

    int32 Removed = 0;
    for (AActor* Candidate : Owners)
    {
        if (!Candidate || Candidate == Keep || Candidate->IsActorBeingDestroyed()) continue;
        UE_LOG(LogTemp, Warning,
            TEXT("PASS21_LANDMARK_DUPLICATE_REPAIRED site=%s tag=%s kept=%s removed=%s policy=%s"),
            SiteLabel,
            *OwnerTag.ToString(),
            *Keep->GetName(),
            *Candidate->GetName(),
            bKeepNewest ? TEXT("keep_newest") : TEXT("keep_oldest"));
        Candidate->Destroy();
        ++Removed;
    }

    return Removed;
}

int32 UOCLandmarkShellOwnershipGuardSubsystem::CountTaggedActors(UWorld& World, FName OwnerTag)
{
    int32 Count = 0;
    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        const AActor* Actor = *It;
        if (Actor && !Actor->IsActorBeingDestroyed() && Actor->ActorHasTag(OwnerTag)) ++Count;
    }
    return Count;
}

AActor* UOCLandmarkShellOwnershipGuardSubsystem::FindTaggedActor(UWorld& World, FName OwnerTag)
{
    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        AActor* Actor = *It;
        if (Actor && !Actor->IsActorBeingDestroyed() && Actor->ActorHasTag(OwnerTag)) return Actor;
    }
    return nullptr;
}

bool UOCLandmarkShellOwnershipGuardSubsystem::HasInstanceGeometryNear(
    AActor* Actor,
    const FVector& SiteCenter,
    float RadiusCm)
{
    if (!Actor) return false;

    const float RadiusSq = FMath::Square(RadiusCm);
    TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
    Actor->GetComponents(Components);
    for (UInstancedStaticMeshComponent* Component : Components)
    {
        if (!Component || !Component->GetStaticMesh() || Component->GetInstanceCount() <= 0) continue;
        for (int32 InstanceIndex = 0; InstanceIndex < Component->GetInstanceCount(); ++InstanceIndex)
        {
            FTransform InstanceTransform;
            if (!Component->GetInstanceTransform(InstanceIndex, InstanceTransform, true)) continue;
            if (FVector::DistSquared2D(InstanceTransform.GetLocation(), SiteCenter) <= RadiusSq) return true;
        }
    }
    return false;
}

void UOCLandmarkShellOwnershipGuardSubsystem::RunFinalValidation()
{
    UWorld* World = GuardWorld.Get();
    if (!World) return;

    // Repair anything that existed before our actor-spawn callback registered or escaped a per-spawn next-tick check.
    DuplicateRepairs += RepairTaggedOwners(*World, MuseumPrototypeTag, TEXT("Museum-R13.7-final"), false);
    DuplicateRepairs += RepairTaggedOwners(*World, MuseumArchitectureTag, TEXT("Museum-R13.8-final"), false);
    DuplicateRepairs += RepairTaggedOwners(*World, SilpoShellTag, TEXT("Silpo-R14.0-final"), true);
    DuplicateRepairs += RepairTaggedOwners(*World, CultureHouseShellTag, TEXT("CultureHouse-R14.6-final"), false);
    if (World->GetNetMode() != NM_Client)
    {
        DuplicateRepairs += RepairTaggedOwners(*World, SilpoEntranceDoorTag, TEXT("SilpoEntranceDoor-final"), true);
    }

    const int32 MuseumPrototypeCount = CountTaggedActors(*World, MuseumPrototypeTag);
    const int32 MuseumArchitectureCount = CountTaggedActors(*World, MuseumArchitectureTag);
    const int32 SilpoShellCount = CountTaggedActors(*World, SilpoShellTag);
    const int32 CultureShellCount = CountTaggedActors(*World, CultureHouseShellTag);
    const int32 SilpoDoorCount = CountTaggedActors(*World, SilpoEntranceDoorTag);

    const FVector Museum = GeoToWorld(FOCGeoReference::Museum());
    const FVector Silpo = GeoToWorld(FOCGeoReference::Silpo());
    const FVector Culture = GeoToWorld(FOCGeoReference::CultureHouse());

    const bool bMuseumPrototypeAtSite = MuseumPrototypeCount == 1 &&
        HasInstanceGeometryNear(FindTaggedActor(*World, MuseumPrototypeTag), Museum, SiteGeometryRadiusCm);
    const bool bMuseumArchitectureAtSite = MuseumArchitectureCount == 1 &&
        HasInstanceGeometryNear(FindTaggedActor(*World, MuseumArchitectureTag), Museum, SiteGeometryRadiusCm);
    const bool bSilpoAtSite = SilpoShellCount == 1 &&
        HasInstanceGeometryNear(FindTaggedActor(*World, SilpoShellTag), Silpo, SiteGeometryRadiusCm);
    const bool bCultureAtSite = CultureShellCount == 1 &&
        HasInstanceGeometryNear(FindTaggedActor(*World, CultureHouseShellTag), Culture, SiteGeometryRadiusCm);
    const bool bSilpoDoorReady = World->GetNetMode() == NM_Client || SilpoDoorCount == 1;

    const bool bReady = MuseumPrototypeCount == 1 && MuseumArchitectureCount == 1 &&
        SilpoShellCount == 1 && CultureShellCount == 1 && bSilpoDoorReady &&
        bMuseumPrototypeAtSite && bMuseumArchitectureAtSite && bSilpoAtSite && bCultureAtSite;

    const float MuseumToSilpoM = FVector::Dist2D(Museum, Silpo) / 100.0f;
    const float MuseumToCultureM = FVector::Dist2D(Museum, Culture) / 100.0f;
    const float SilpoToCultureM = FVector::Dist2D(Silpo, Culture) / 100.0f;

    if (bReady)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS21_LANDMARK_OWNERSHIP_READY museumPrototype=%d museumArchitecture=%d silpoShell=%d cultureShell=%d silpoDoor=%d repaired=%d siteGeometry=1 distancesM=%.1f/%.1f/%.1f"),
            MuseumPrototypeCount,
            MuseumArchitectureCount,
            SilpoShellCount,
            CultureShellCount,
            SilpoDoorCount,
            DuplicateRepairs,
            MuseumToSilpoM,
            MuseumToCultureM,
            SilpoToCultureM);
    }
    else
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS21_LANDMARK_OWNERSHIP_FAIL museumPrototype=%d museumArchitecture=%d silpoShell=%d cultureShell=%d silpoDoor=%d siteMuseumPrototype=%d siteMuseumArchitecture=%d siteSilpo=%d siteCulture=%d repaired=%d"),
            MuseumPrototypeCount,
            MuseumArchitectureCount,
            SilpoShellCount,
            CultureShellCount,
            SilpoDoorCount,
            bMuseumPrototypeAtSite ? 1 : 0,
            bMuseumArchitectureAtSite ? 1 : 0,
            bSilpoAtSite ? 1 : 0,
            bCultureAtSite ? 1 : 0,
            DuplicateRepairs);
    }
}
