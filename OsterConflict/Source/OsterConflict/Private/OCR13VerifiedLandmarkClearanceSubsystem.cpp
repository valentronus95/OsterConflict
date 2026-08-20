#include "OCR13VerifiedLandmarkClearanceSubsystem.h"

#include "OCGameMode.h"
#include "OCGeoReference.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

namespace
{
    constexpr float ClearanceDelaySeconds = 2.12f;

    struct FLandmarkZone
    {
        FVector Center = FVector::ZeroVector;
        FVector2D HalfExtentsCm = FVector2D::ZeroVector;
        bool bAllowLandmarkFamilies = false;
    };

    FVector Geo(const FOCGeoReferencePoint& Ref)
    {
        return FOCGeoReference::ToLocalCm(Ref.Latitude, Ref.Longitude, 0.0);
    }

    bool IsGenericResidentialFamily(const FName Name)
    {
        const FString Value = Name.ToString();
        return Name == TEXT("Buildings") || Name == TEXT("ResidentialRoofs") || Name == TEXT("ResidentialDetails") ||
            Name == TEXT("R13_House01") || Name == TEXT("R13_House02") ||
            Name == TEXT("R12_House01") || Name == TEXT("R12_House02") ||
            Value.StartsWith(TEXT("R13_OsterBrickHouse")) ||
            Value.StartsWith(TEXT("R13_OsterHouse")) ||
            Value.StartsWith(TEXT("R13_OsterGreyPitchedRoofs"));
    }

    bool IsLandmarkPresentationFamily(const FName Name)
    {
        return Name == TEXT("LandmarkBlocks") || Name == TEXT("LandmarkRoofs") ||
            Name == TEXT("LandmarkWindows") || Name == TEXT("LandmarkDetails");
    }

    bool IsInside(const FVector& WorldLocation, const FLandmarkZone& Zone)
    {
        const FVector Delta = WorldLocation - Zone.Center;
        return FMath::Abs(Delta.X) <= Zone.HalfExtentsCm.X &&
            FMath::Abs(Delta.Y) <= Zone.HalfExtentsCm.Y;
    }

    int32 RemoveInstances(UInstancedStaticMeshComponent* Component, const FLandmarkZone& Zone)
    {
        if (!Component) return 0;
        int32 Removed = 0;
        for (int32 Index = Component->GetInstanceCount() - 1; Index >= 0; --Index)
        {
            FTransform Transform;
            if (!Component->GetInstanceTransform(Index, Transform, true)) continue;
            if (!IsInside(Transform.GetLocation(), Zone)) continue;
            if (Component->RemoveInstance(Index)) ++Removed;
        }
        if (Removed > 0) Component->MarkRenderStateDirty();
        return Removed;
    }
}

bool UOCR13VerifiedLandmarkClearanceSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13VerifiedLandmarkClearanceSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    // The removed source structures can own blocking collision. Apply the same deterministic clearance on every
    // authoritative world, including a headless dedicated server, to prevent invisible server-only blockers.
    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle Timer;
    InWorld.GetTimerManager().SetTimer(Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) ApplyClearance(*World);
        }), ClearanceDelaySeconds, false);
}

void UOCR13VerifiedLandmarkClearanceSubsystem::ApplyClearance(UWorld& World)
{
    const FLandmarkZone Zones[] =
    {
        { Geo(FOCGeoReference::Silpo()), FVector2D(2200.0f, 1550.0f), true },
        { Geo(FOCGeoReference::BusStation()), FVector2D(1750.0f, 1300.0f), false },
        { Geo(FOCGeoReference::CityCouncil()), FVector2D(2300.0f, 1650.0f), true },
    };

    int32 RemovedGeneric = 0;
    int32 RemovedLandmark = 0;

    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor ||
            Actor->ActorHasTag(TEXT("R13_SilpoPhotoModel")) ||
            Actor->ActorHasTag(TEXT("R13_BusStationPhotoModel")) ||
            Actor->ActorHasTag(TEXT("R13_CityCouncilPhotoModel")))
        {
            continue;
        }

        TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
        Actor->GetComponents(Components);
        for (UInstancedStaticMeshComponent* Component : Components)
        {
            if (!Component) continue;
            const FName Name = Component->GetFName();
            const bool bGeneric = IsGenericResidentialFamily(Name);
            const bool bLandmark = IsLandmarkPresentationFamily(Name);
            if (!bGeneric && !bLandmark) continue;

            for (const FLandmarkZone& Zone : Zones)
            {
                if (bLandmark && !Zone.bAllowLandmarkFamilies) continue;
                const int32 Removed = RemoveInstances(Component, Zone);
                if (bGeneric) RemovedGeneric += Removed;
                else RemovedLandmark += Removed;
            }
        }
    }

    UE_LOG(LogTemp, Display,
        TEXT("R13 verified landmark clearance: generic=%d landmark=%d instances removed from Silpo/bus-station/city-council zones; roads/sidewalks preserved and server collision kept deterministic."),
        RemovedGeneric, RemovedLandmark);
}
