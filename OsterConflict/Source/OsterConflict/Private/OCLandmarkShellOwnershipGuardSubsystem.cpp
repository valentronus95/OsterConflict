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

    // Pass 45 shell authority. One current visible shell owner per site, full stop.
    const FName MuseumReferenceLayerTag(TEXT("R137_MuseumPhotoModel"));
    const FName MuseumShellTag(TEXT("R138_MuseumHighFidelityArchitecture"));
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

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_LANDMARK_SINGLE_SHELL_CONTRACT_READY museum=R138 silpo=R140 culture=R146 museum_r137_role=reference_detail_interactivity shell_owners_per_site=1"));
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

    // SpawnActor delegates execute before the caller appends runtime identity tags. Re-check next tick.
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

    if (Actor->ActorHasTag(MuseumReferenceLayerTag))
    {
        // R13.7 is retained only as the single reference/detail/interactivity parent. It is not a shell owner.
        DuplicateRepairs += RepairTaggedOwners(*World, MuseumReferenceLayerTag, TEXT("Museum-R13.7-reference"), false);
    }
    else if (Actor->ActorHasTag(MuseumShellTag))
    {
        // R13.8 is the one current Museum shell.
        DuplicateRepairs += RepairTaggedOwners(*World, MuseumShellTag, TEXT("Museum-shell-R13.8"), false);
    }
    else if (Actor->ActorHasTag(SilpoShellTag))
    {
        // R14.0 is the one current Silpo shell. Detail actors do not share this tag.
        DuplicateRepairs += RepairTaggedOwners(*World, SilpoShellTag, TEXT("Silpo-shell-R14.0"), true);
    }
    else if (Actor->ActorHasTag(CultureHouseShellTag))
    {
        // R14.6 is the one current Culture House shell.
        DuplicateRepairs += RepairTaggedOwners(*World, CultureHouseShellTag, TEXT("CultureHouse-shell-R14.6"), false);
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

    // One final repair after the historical startup window. No periodic full-world owner scan follows.
    DuplicateRepairs += RepairTaggedOwners(*World, MuseumReferenceLayerTag, TEXT("Museum-R13.7-reference-final"), false);
    DuplicateRepairs += RepairTaggedOwners(*World, MuseumShellTag, TEXT("Museum-shell-R13.8-final"), false);
    DuplicateRepairs += RepairTaggedOwners(*World, SilpoShellTag, TEXT("Silpo-shell-R14.0-final"), true);
    DuplicateRepairs += RepairTaggedOwners(*World, CultureHouseShellTag, TEXT("CultureHouse-shell-R14.6-final"), false);
    if (World->GetNetMode() != NM_Client)
    {
        DuplicateRepairs += RepairTaggedOwners(*World, SilpoEntranceDoorTag, TEXT("SilpoEntranceDoor-final"), true);
    }

    const int32 MuseumReferenceLayerCount = CountTaggedActors(*World, MuseumReferenceLayerTag);
    const int32 MuseumShellCount = CountTaggedActors(*World, MuseumShellTag);
    const int32 SilpoShellCount = CountTaggedActors(*World, SilpoShellTag);
    const int32 CultureShellCount = CountTaggedActors(*World, CultureHouseShellTag);
    const int32 SilpoDoorCount = CountTaggedActors(*World, SilpoEntranceDoorTag);

    const FVector Museum = GeoToWorld(FOCGeoReference::Museum());
    const FVector Silpo = GeoToWorld(FOCGeoReference::Silpo());
    const FVector Culture = GeoToWorld(FOCGeoReference::CultureHouse());

    const bool bMuseumAtSite = MuseumShellCount == 1 &&
        HasInstanceGeometryNear(FindTaggedActor(*World, MuseumShellTag), Museum, SiteGeometryRadiusCm);
    const bool bSilpoAtSite = SilpoShellCount == 1 &&
        HasInstanceGeometryNear(FindTaggedActor(*World, SilpoShellTag), Silpo, SiteGeometryRadiusCm);
    const bool bCultureAtSite = CultureShellCount == 1 &&
        HasInstanceGeometryNear(FindTaggedActor(*World, CultureHouseShellTag), Culture, SiteGeometryRadiusCm);
    const bool bSilpoDoorReady = World->GetNetMode() == NM_Client || SilpoDoorCount == 1;

    // R13.7 reference actor is still required once because R13.8 interaction actors use it as their parent,
    // but it no longer contributes a second shell count.
    const bool bReady = MuseumReferenceLayerCount == 1 && MuseumShellCount == 1 &&
        SilpoShellCount == 1 && CultureShellCount == 1 && bSilpoDoorReady &&
        bMuseumAtSite && bSilpoAtSite && bCultureAtSite;

    const float MuseumToSilpoM = FVector::Dist2D(Museum, Silpo) / 100.0f;
    const float MuseumToCultureM = FVector::Dist2D(Museum, Culture) / 100.0f;
    const float SilpoToCultureM = FVector::Dist2D(Silpo, Culture) / 100.0f;

    if (bReady)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_SINGLE_LANDMARK_SHELL_OWNERS_READY museumShell=%d museumReferenceLayer=%d silpoShell=%d cultureShell=%d silpoDoor=%d repaired=%d siteGeometry=1 periodic_owner_scan=0 distancesM=%.1f/%.1f/%.1f"),
            MuseumShellCount,
            MuseumReferenceLayerCount,
            SilpoShellCount,
            CultureShellCount,
            SilpoDoorCount,
            DuplicateRepairs,
            MuseumToSilpoM,
            MuseumToCultureM,
            SilpoToCultureM);
        // Keep the historical marker for source/acceptance tooling, but its semantics are now one shell per site.
        UE_LOG(LogTemp, Display,
            TEXT("PASS21_LANDMARK_OWNERSHIP_READY museumPrototype=%d museumArchitecture=%d silpoShell=%d cultureShell=%d silpoDoor=%d repaired=%d siteGeometry=1 distancesM=%.1f/%.1f/%.1f"),
            MuseumReferenceLayerCount,
            MuseumShellCount,
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
            TEXT("PASS45_SINGLE_LANDMARK_SHELL_OWNERS_FAIL museumShell=%d museumReferenceLayer=%d silpoShell=%d cultureShell=%d silpoDoor=%d siteMuseum=%d siteSilpo=%d siteCulture=%d repaired=%d"),
            MuseumShellCount,
            MuseumReferenceLayerCount,
            SilpoShellCount,
            CultureShellCount,
            SilpoDoorCount,
            bMuseumAtSite ? 1 : 0,
            bSilpoAtSite ? 1 : 0,
            bCultureAtSite ? 1 : 0,
            DuplicateRepairs);
        UE_LOG(LogTemp, Error,
            TEXT("PASS21_LANDMARK_OWNERSHIP_FAIL museumPrototype=%d museumArchitecture=%d silpoShell=%d cultureShell=%d silpoDoor=%d siteMuseumArchitecture=%d siteSilpo=%d siteCulture=%d repaired=%d"),
            MuseumReferenceLayerCount,
            MuseumShellCount,
            SilpoShellCount,
            CultureShellCount,
            SilpoDoorCount,
            bMuseumAtSite ? 1 : 0,
            bSilpoAtSite ? 1 : 0,
            bCultureAtSite ? 1 : 0,
            DuplicateRepairs);
    }
}