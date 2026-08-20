#include "OCR146LandmarkSeparationSubsystem.h"

#include "OCGameMode.h"
#include "OCGeoReference.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

namespace
{
    constexpr float SeparationStartupDelaySeconds = 0.15f;
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

    void RemoveLegacyCompositeActors(UWorld& World)
    {
        TArray<TWeakObjectPtr<AActor>> ToDestroy;
        for (TActorIterator<AActor> It(&World); It; ++It)
        {
            AActor* Actor = *It;
            if (!Actor) continue;
            if (Actor->ActorHasTag(TEXT("R13_CultureHousePhotoModel")) ||
                Actor->ActorHasTag(TEXT("R13_SilpoPhotoModel")))
            {
                ToDestroy.Add(Actor);
            }
        }
        for (const TWeakObjectPtr<AActor>& WeakActor : ToDestroy)
        {
            if (AActor* Actor = WeakActor.Get()) Actor->Destroy();
        }
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

    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle Timer;
    InWorld.GetTimerManager().SetTimer(Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) EnforceSeparation(*World);
        }), SeparationStartupDelaySeconds, false);
}

void UOCR146LandmarkSeparationSubsystem::EnforceSeparation(UWorld& World) const
{
    const FVector Museum = GeoToWorld(FOCGeoReference::Museum());
    const FVector Silpo = GeoToWorld(FOCGeoReference::Silpo());
    const FVector CultureHouse = GeoToWorld(FOCGeoReference::CultureHouse());

    RemoveLegacyCompositeActors(World);

    int32 MuseumRemoved = 0;
    int32 SilpoRemoved = 0;
    int32 CultureRemoved = 0;
    int32 FamiliesTouched = 0;

    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor) continue;

        TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
        Actor->GetComponents(Components);
        for (UInstancedStaticMeshComponent* Component : Components)
        {
            if (!Component || !IsGenericBuildingFamily(Component->GetFName())) continue;
            const int32 Before = Component->GetInstanceCount();
            MuseumRemoved += RemoveInstancesNear(Component, Museum, MuseumCleanupRadiusCm);
            SilpoRemoved += RemoveInstancesNear(Component, Silpo, SilpoCleanupRadiusCm);
            CultureRemoved += RemoveInstancesNear(Component, CultureHouse, CultureCleanupRadiusCm);
            if (Component->GetInstanceCount() != Before) ++FamiliesTouched;
        }
    }

    const float MuseumToSilpoM = FVector::Dist2D(Museum, Silpo) / 100.0f;
    const float MuseumToCultureM = FVector::Dist2D(Museum, CultureHouse) / 100.0f;
    const float SilpoToCultureM = FVector::Dist2D(Silpo, CultureHouse) / 100.0f;

    UE_LOG(LogTemp, Display,
        TEXT("R14.6 landmark separation: Museum removed=%d, Silpo removed=%d, CultureHouse removed=%d, sourceFamiliesTouched=%d."),
        MuseumRemoved, SilpoRemoved, CultureRemoved, FamiliesTouched);
    UE_LOG(LogTemp, Display,
        TEXT("R14.6 canonical site distances: Museum-Silpo=%.1fm Museum-CultureHouse=%.1fm Silpo-CultureHouse=%.1fm. Each landmark remains on its own geo owner."),
        MuseumToSilpoM, MuseumToCultureM, SilpoToCultureM);
}
