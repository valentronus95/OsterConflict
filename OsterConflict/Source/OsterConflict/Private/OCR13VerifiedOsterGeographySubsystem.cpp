#include "OCR13VerifiedOsterGeographySubsystem.h"

#include "OCGameMode.h"
#include "OCGeoReference.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

namespace
{
    constexpr float LegacySlicePurgeDelaySeconds = 1.82f;
    constexpr float GeographyDelaySeconds = 3.35f;
    constexpr float LegacySliceCenterX = -3400.0f;
    constexpr float LegacySliceHalfWidthCm = 7200.0f;
    constexpr float LegacySliceMinY = -14500.0f;
    constexpr float LegacySliceMaxY = 17800.0f;
    constexpr float LegacyStadiumSearchRadiusCm = 8400.0f;
    const FVector LegacyStadiumAnchor(15000.0f, -1500.0f, 0.0f);

    bool IsLegacySliceComponent(const FName Name)
    {
        const FString Value = Name.ToString();
        return Value.StartsWith(TEXT("R12_House")) ||
            Value.StartsWith(TEXT("R12_Fence")) ||
            Value.StartsWith(TEXT("R12_Tree")) ||
            Value.StartsWith(TEXT("R12_Plants")) ||
            Value.StartsWith(TEXT("R12_Barrels")) ||
            Value.StartsWith(TEXT("R12_Crates")) ||
            Value.StartsWith(TEXT("R12_StreetLights")) ||
            Value.StartsWith(TEXT("R13_KrushelnytskaGrass")) ||
            Value.StartsWith(TEXT("R13_KrushelnytskaPine")) ||
            Value.StartsWith(TEXT("R13_KrushelnytskaUtility")) ||
            Value.StartsWith(TEXT("R13_KrushelnytskaPole"));
    }

    bool IsInsideLegacySlice(const FVector& Location)
    {
        return FMath::Abs(Location.X - LegacySliceCenterX) <= LegacySliceHalfWidthCm &&
            Location.Y >= LegacySliceMinY && Location.Y <= LegacySliceMaxY;
    }

    bool IsOsterResidentialPresentationComponent(const FName Name)
    {
        const FString Value = Name.ToString();
        return Value.StartsWith(TEXT("R13_OsterBrickHouse")) ||
            Value.StartsWith(TEXT("R13_OsterHousePlinth")) ||
            Value.StartsWith(TEXT("R13_OsterHouseWindow")) ||
            Value.StartsWith(TEXT("R13_OsterGreyPitchedRoofs")) ||
            Value.StartsWith(TEXT("R13_OsterHousePorches")) ||
            Value.StartsWith(TEXT("R13_OsterHouseDoors"));
    }

    bool IsStadiumPresentationComponent(const FName Name)
    {
        const FString Value = Name.ToString();
        return Name == TEXT("StadiumGeometry") || Name == TEXT("StadiumDetails") || Name == TEXT("Fences") ||
            Value.StartsWith(TEXT("R13_Stadium")) ||
            Value.StartsWith(TEXT("R13_CivicTree")) ||
            Value.StartsWith(TEXT("R13_CivicShrub"));
    }
}

bool UOCR13VerifiedOsterGeographySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13VerifiedOsterGeographySubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);

    // The old R12 art shortcut is purged BEFORE the 1.95 s Oster residential pass. This prevents its fake houses
    // from consuming replacement slots or surviving invisibly as duplicate map content.
    FTimerHandle LegacyPurgeTimer;
    InWorld.GetTimerManager().SetTimer(LegacyPurgeTimer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) SuppressLegacyNearSpawnSlice(*World);
        }), LegacySlicePurgeDelaySeconds, false);

    FTimerHandle GeographyTimer;
    InWorld.GetTimerManager().SetTimer(GeographyTimer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) ApplyVerifiedGeography(*World);
        }), GeographyDelaySeconds, false);
}

FVector UOCR13VerifiedOsterGeographySubsystem::VerifiedStadiumAnchor()
{
    const FOCGeoReferencePoint Ref = FOCGeoReference::Stadium();
    return FOCGeoReference::ToLocalCm(Ref.Latitude, Ref.Longitude, 0.0);
}

void UOCR13VerifiedOsterGeographySubsystem::ApplyVerifiedGeography(UWorld& World)
{
    SuppressLegacyNearSpawnSlice(World);
    RemoveLegacySliceResidentialPresentation(World);
    RelocateStadiumPresentation(World);
}

void UOCR13VerifiedOsterGeographySubsystem::SuppressLegacyNearSpawnSlice(UWorld& World)
{
    int32 RemovedComponents = 0;
    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor) continue;

        TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
        Actor->GetComponents(Components);
        for (UInstancedStaticMeshComponent* Component : Components)
        {
            if (!Component || !IsLegacySliceComponent(Component->GetFName())) continue;

            // These components belong only to the obsolete visual shortcut, not to source map collision. Destroying
            // them before residential styling is safer than merely hiding them and accidentally letting later scans
            // treat their transforms as real Oster houses/trees/fences.
            Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            Component->SetCanEverAffectNavigation(false);
            Component->DestroyComponent();
            ++RemovedComponents;
        }
    }

    if (RemovedComponents > 0)
    {
        UE_LOG(LogTemp, Display,
            TEXT("R13.6 verified geography: purged %d legacy near-spawn Krushelnytska visual components before Oster residential styling; source geo corridor remains authoritative."),
            RemovedComponents);
    }
}

void UOCR13VerifiedOsterGeographySubsystem::RemoveLegacySliceResidentialPresentation(UWorld& World)
{
    // Defensive cleanup for hot-reload/late-order cases where the architecture pass may have seen an obsolete slice
    // before its components were purged. Normal fresh launches should remove zero instances here.
    int32 RemovedInstances = 0;
    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor) continue;

        TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
        Actor->GetComponents(Components);
        for (UInstancedStaticMeshComponent* Component : Components)
        {
            if (!Component || !IsOsterResidentialPresentationComponent(Component->GetFName())) continue;

            bool bChanged = false;
            for (int32 Index = Component->GetInstanceCount() - 1; Index >= 0; --Index)
            {
                FTransform Transform;
                if (!Component->GetInstanceTransform(Index, Transform, true)) continue;
                if (!IsInsideLegacySlice(Transform.GetLocation())) continue;
                if (Component->RemoveInstance(Index))
                {
                    ++RemovedInstances;
                    bChanged = true;
                }
            }
            if (bChanged) Component->MarkRenderStateDirty();
        }
    }

    if (RemovedInstances > 0)
    {
        UE_LOG(LogTemp, Display,
            TEXT("R13.6 verified geography: removed %d residential-art instances inherited from a late deprecated near-spawn slice."),
            RemovedInstances);
    }
}

void UOCR13VerifiedOsterGeographySubsystem::RelocateStadiumPresentation(UWorld& World)
{
    const FVector Verified = VerifiedStadiumAnchor();
    const FVector Delta = Verified - LegacyStadiumAnchor;
    const float RadiusSq = FMath::Square(LegacyStadiumSearchRadiusCm);
    int32 MovedInstances = 0;
    int32 TouchedComponents = 0;

    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor) continue;

        TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
        Actor->GetComponents(Components);
        for (UInstancedStaticMeshComponent* Component : Components)
        {
            if (!Component || !IsStadiumPresentationComponent(Component->GetFName())) continue;
            bool bChanged = false;

            for (int32 Index = 0; Index < Component->GetInstanceCount(); ++Index)
            {
                FTransform Transform;
                if (!Component->GetInstanceTransform(Index, Transform, true)) continue;
                const FVector Location = Transform.GetLocation();
                if (FVector::DistSquared2D(Location, LegacyStadiumAnchor) > RadiusSq) continue;

                Transform.SetLocation(Location + Delta);
                if (Component->UpdateInstanceTransform(Index, Transform, true, false, true))
                {
                    ++MovedInstances;
                    bChanged = true;
                }
            }

            if (bChanged)
            {
                Component->MarkRenderStateDirty();
                ++TouchedComponents;
            }
        }
    }

    UE_LOG(LogTemp, Display,
        TEXT("R13.6 verified geography: stadium presentation moved by X %.0f / Y %.0f cm to canonical geo anchor; instances=%d components=%d."),
        Delta.X, Delta.Y, MovedInstances, TouchedComponents);
}
