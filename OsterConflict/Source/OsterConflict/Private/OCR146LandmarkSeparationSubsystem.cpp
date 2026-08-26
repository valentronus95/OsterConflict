#include "OCR146LandmarkSeparationSubsystem.h"

#include "OCGameMode.h"
#include "OCGeoReference.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"

namespace
{
    constexpr float ValidationDelaySeconds = 0.90f;
    constexpr float MuseumRadiusCm = 2600.0f;
    constexpr float SilpoRadiusCm = 2600.0f;
    constexpr float CultureRadiusCm = 2600.0f;
    constexpr float MinimumMuseumCultureSeparationCm = 10000.0f;

    const FName MuseumOwnerTag(TEXT("R137_MuseumPhotoModel"));
    const FName SilpoOwnerTag(TEXT("R140_SilpoPhotoModel"));
    const FName CultureOwnerTag(TEXT("R146_CultureHouseAuthoritative"));

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

    bool IsForbiddenLegacyLandmarkActor(const AActor* Actor)
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

    int32 CountInstancesNear(UInstancedStaticMeshComponent* Component, const FVector& Center, const float RadiusCm)
    {
        if (!Component) return 0;
        const float RadiusSq = FMath::Square(RadiusCm);
        int32 Count = 0;
        for (int32 Index = 0; Index < Component->GetInstanceCount(); ++Index)
        {
            FTransform Transform;
            if (!Component->GetInstanceTransform(Index, Transform, true)) continue;
            if (FVector::DistSquared2D(Transform.GetLocation(), Center) <= RadiusSq) ++Count;
        }
        return Count;
    }

    int32 CountActorInstancesNear(AActor* Actor, const FVector& Center, const float RadiusCm)
    {
        if (!Actor) return 0;
        int32 Count = 0;
        TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
        Actor->GetComponents(Components);
        for (UInstancedStaticMeshComponent* Component : Components)
        {
            Count += CountInstancesNear(Component, Center, RadiusCm);
        }
        return Count;
    }

    int32 CountCultureColumnShafts(AActor* Actor)
    {
        if (!Actor) return 0;
        int32 Shafts = 0;
        TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
        Actor->GetComponents(Components);
        for (UInstancedStaticMeshComponent* Component : Components)
        {
            if (!Component || !Component->GetFName().ToString().StartsWith(TEXT("R146Culture_Columns"))) continue;
            for (int32 Index = 0; Index < Component->GetInstanceCount(); ++Index)
            {
                FTransform Transform;
                if (!Component->GetInstanceTransform(Index, Transform, false)) continue;
                // The six reference facade shafts are 610 cm high on the engine Cylinder (Z scale 6.1).
                // Bases/caps use sub-1.0 Z scale, so this distinguishes six columns from their trim pieces.
                if (Transform.GetScale3D().Z > 4.0f) ++Shafts;
            }
        }
        return Shafts;
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

    ValidationWorld = &InWorld;
    InWorld.GetTimerManager().SetTimer(
        ValidationTimer,
        this,
        &UOCR146LandmarkSeparationSubsystem::ValidateSeparation,
        ValidationDelaySeconds,
        false);

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_LANDMARK_SEPARATION_VALIDATION_SCHEDULED delay_s=%.2f mutation=0 actor_spawn_guard=0 periodic_scan=0"),
        ValidationDelaySeconds);
}

void UOCR146LandmarkSeparationSubsystem::Deinitialize()
{
    if (UWorld* World = ValidationWorld.Get())
    {
        World->GetTimerManager().ClearTimer(ValidationTimer);
    }
    ValidationWorld.Reset();
    Super::Deinitialize();
}

void UOCR146LandmarkSeparationSubsystem::ValidateSeparation()
{
    UWorld* World = ValidationWorld.Get();
    if (!World) return;

    const FVector Museum = GeoToWorld(FOCGeoReference::Museum());
    const FVector Silpo = GeoToWorld(FOCGeoReference::Silpo());
    const FVector CultureHouse = GeoToWorld(FOCGeoReference::CultureHouse());

    int32 ForbiddenLegacyActors = 0;
    int32 MuseumGenericInstances = 0;
    int32 SilpoGenericInstances = 0;
    int32 CultureGenericInstances = 0;
    int32 MuseumOwnerCount = 0;
    int32 SilpoOwnerCount = 0;
    int32 CultureOwnerCount = 0;
    int32 MuseumIdentityInstancesAtMuseum = 0;
    int32 SilpoIdentityInstancesAtSilpo = 0;
    int32 CultureIdentityInstancesAtCulture = 0;
    int32 CultureIdentityInstancesAtMuseum = 0;
    int32 MuseumIdentityInstancesAtCulture = 0;
    int32 SilpoIdentityInstancesAtMuseum = 0;
    int32 SilpoIdentityInstancesAtCulture = 0;
    int32 MuseumIdentityInstancesAtSilpo = 0;
    int32 CultureIdentityInstancesAtSilpo = 0;
    int32 CultureColumnShafts = 0;
    float SilpoOwnerAnchorErrorCm = TNumericLimits<float>::Max();
    float CultureOwnerAnchorErrorCm = TNumericLimits<float>::Max();

    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor || Actor->IsActorBeingDestroyed()) continue;
        if (IsForbiddenLegacyLandmarkActor(Actor)) ++ForbiddenLegacyActors;

        const bool bMuseumOwner = Actor->ActorHasTag(MuseumOwnerTag);
        const bool bSilpoOwner = Actor->ActorHasTag(SilpoOwnerTag);
        const bool bCultureOwner = Actor->ActorHasTag(CultureOwnerTag);
        if (bMuseumOwner)
        {
            ++MuseumOwnerCount;
            MuseumIdentityInstancesAtMuseum += CountActorInstancesNear(Actor, Museum, MuseumRadiusCm);
            MuseumIdentityInstancesAtSilpo += CountActorInstancesNear(Actor, Silpo, SilpoRadiusCm);
            MuseumIdentityInstancesAtCulture += CountActorInstancesNear(Actor, CultureHouse, CultureRadiusCm);
        }
        if (bSilpoOwner)
        {
            ++SilpoOwnerCount;
            SilpoIdentityInstancesAtSilpo += CountActorInstancesNear(Actor, Silpo, SilpoRadiusCm);
            SilpoIdentityInstancesAtMuseum += CountActorInstancesNear(Actor, Museum, MuseumRadiusCm);
            SilpoIdentityInstancesAtCulture += CountActorInstancesNear(Actor, CultureHouse, CultureRadiusCm);
            SilpoOwnerAnchorErrorCm = FMath::Min(
                SilpoOwnerAnchorErrorCm,
                FVector::Dist2D(Actor->GetActorLocation(), Silpo));
        }
        if (bCultureOwner)
        {
            ++CultureOwnerCount;
            CultureIdentityInstancesAtCulture += CountActorInstancesNear(Actor, CultureHouse, CultureRadiusCm);
            CultureIdentityInstancesAtMuseum += CountActorInstancesNear(Actor, Museum, MuseumRadiusCm);
            CultureIdentityInstancesAtSilpo += CountActorInstancesNear(Actor, Silpo, SilpoRadiusCm);
            CultureColumnShafts += CountCultureColumnShafts(Actor);
            CultureOwnerAnchorErrorCm = FMath::Min(
                CultureOwnerAnchorErrorCm,
                FVector::Dist2D(Actor->GetActorLocation(), CultureHouse));
        }

        TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
        Actor->GetComponents(Components);
        for (UInstancedStaticMeshComponent* Component : Components)
        {
            if (!Component || !IsGenericBuildingFamily(Component->GetFName())) continue;
            MuseumGenericInstances += CountInstancesNear(Component, Museum, MuseumRadiusCm);
            SilpoGenericInstances += CountInstancesNear(Component, Silpo, SilpoRadiusCm);
            CultureGenericInstances += CountInstancesNear(Component, CultureHouse, CultureRadiusCm);
        }
    }

    const float MuseumToSilpoM = FVector::Dist2D(Museum, Silpo) / 100.0f;
    const float MuseumToCultureCm = FVector::Dist2D(Museum, CultureHouse);
    const float MuseumToCultureM = MuseumToCultureCm / 100.0f;
    const float SilpoToCultureM = FVector::Dist2D(Silpo, CultureHouse) / 100.0f;

    const bool bParcelReady = ForbiddenLegacyActors == 0 &&
        MuseumGenericInstances == 0 && SilpoGenericInstances == 0 && CultureGenericInstances == 0;

    const bool bIdentityReady = MuseumOwnerCount == 1 && CultureOwnerCount == 1 &&
        MuseumIdentityInstancesAtMuseum > 0 && CultureIdentityInstancesAtCulture > 0 &&
        CultureIdentityInstancesAtMuseum == 0 && MuseumIdentityInstancesAtCulture == 0 &&
        CultureColumnShafts == 6 && MuseumToCultureCm >= MinimumMuseumCultureSeparationCm &&
        CultureOwnerAnchorErrorCm <= 100.0f;

    const bool bSilpoIdentityReady = SilpoOwnerCount == 1 && SilpoIdentityInstancesAtSilpo > 0 &&
        SilpoIdentityInstancesAtMuseum == 0 && SilpoIdentityInstancesAtCulture == 0 &&
        MuseumIdentityInstancesAtSilpo == 0 && CultureIdentityInstancesAtSilpo == 0 &&
        SilpoOwnerAnchorErrorCm <= 100.0f;

    if (bParcelReady)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_LANDMARK_SEPARATION_VALIDATION_READY legacyActors=0 genericMuseum=0 genericSilpo=0 genericCulture=0 mutation=0 periodic_scan=0 distancesM=%.1f/%.1f/%.1f"),
            MuseumToSilpoM, MuseumToCultureM, SilpoToCultureM);
    }
    else
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_LANDMARK_SEPARATION_VALIDATION_FAIL legacyActors=%d genericMuseum=%d genericSilpo=%d genericCulture=%d mutation=0 primary_authoring_fix_required=1 distancesM=%.1f/%.1f/%.1f"),
            ForbiddenLegacyActors,
            MuseumGenericInstances,
            SilpoGenericInstances,
            CultureGenericInstances,
            MuseumToSilpoM, MuseumToCultureM, SilpoToCultureM);
    }

    if (bIdentityReady)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_LANDMARK_IDENTITY_VALIDATION_READY museumOwners=1 cultureOwners=1 museumAtMuseum=%d cultureAtCulture=%d cultureAtMuseum=0 museumAtCulture=0 cultureColumnShafts=6 cultureAnchorErrorCm=%.1f museumCultureDistanceM=%.1f mutation=0"),
            MuseumIdentityInstancesAtMuseum, CultureIdentityInstancesAtCulture,
            CultureOwnerAnchorErrorCm, MuseumToCultureM);
    }
    else
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_LANDMARK_IDENTITY_VALIDATION_FAIL museumOwners=%d cultureOwners=%d museumAtMuseum=%d cultureAtCulture=%d cultureAtMuseum=%d museumAtCulture=%d cultureColumnShafts=%d cultureAnchorErrorCm=%.1f museumCultureDistanceM=%.1f mutation=0 primary_authoring_fix_required=1"),
            MuseumOwnerCount, CultureOwnerCount,
            MuseumIdentityInstancesAtMuseum, CultureIdentityInstancesAtCulture,
            CultureIdentityInstancesAtMuseum, MuseumIdentityInstancesAtCulture,
            CultureColumnShafts, CultureOwnerAnchorErrorCm, MuseumToCultureM);
    }

    if (bSilpoIdentityReady)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_SILPO_IDENTITY_VALIDATION_READY silpoOwners=1 silpoAtSilpo=%d silpoAtMuseum=0 silpoAtCulture=0 museumAtSilpo=0 cultureAtSilpo=0 silpoAnchorErrorCm=%.1f mutation=0"),
            SilpoIdentityInstancesAtSilpo, SilpoOwnerAnchorErrorCm);
    }
    else
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_SILPO_IDENTITY_VALIDATION_FAIL silpoOwners=%d silpoAtSilpo=%d silpoAtMuseum=%d silpoAtCulture=%d museumAtSilpo=%d cultureAtSilpo=%d silpoAnchorErrorCm=%.1f mutation=0 primary_authoring_fix_required=1"),
            SilpoOwnerCount, SilpoIdentityInstancesAtSilpo,
            SilpoIdentityInstancesAtMuseum, SilpoIdentityInstancesAtCulture,
            MuseumIdentityInstancesAtSilpo, CultureIdentityInstancesAtSilpo,
            SilpoOwnerAnchorErrorCm);
    }
}
