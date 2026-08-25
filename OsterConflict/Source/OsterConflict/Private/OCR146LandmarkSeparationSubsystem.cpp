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

    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor || Actor->IsActorBeingDestroyed()) continue;
        if (IsForbiddenLegacyLandmarkActor(Actor)) ++ForbiddenLegacyActors;

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
    const float MuseumToCultureM = FVector::Dist2D(Museum, CultureHouse) / 100.0f;
    const float SilpoToCultureM = FVector::Dist2D(Silpo, CultureHouse) / 100.0f;

    const bool bReady = ForbiddenLegacyActors == 0 &&
        MuseumGenericInstances == 0 && SilpoGenericInstances == 0 && CultureGenericInstances == 0;

    if (bReady)
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
}
