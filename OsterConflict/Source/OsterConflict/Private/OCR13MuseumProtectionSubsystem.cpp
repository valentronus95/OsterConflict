#include "OCR13MuseumProtectionSubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

namespace
{
    constexpr float ProtectionDelaySeconds = 2.65f;
    constexpr float LegacyMuseumWindowRadiusCm = 7200.0f;

    bool IsGenericDressingComponent(const FName Name)
    {
        const FString Text = Name.ToString();
        return Text.StartsWith(TEXT("R13_DenseGrass")) ||
            Text.StartsWith(TEXT("R13_GroundPlant")) ||
            Text.StartsWith(TEXT("R13_CompanionTree")) ||
            Text.StartsWith(TEXT("R13_TreeStump")) ||
            Text.StartsWith(TEXT("R13_ExplicitPine")) ||
            Text.StartsWith(TEXT("R13_Shrub")) ||
            Text.StartsWith(TEXT("R13_WetlandReed")) ||
            Text.StartsWith(TEXT("R13_Yard"));
    }

    bool IsSharedLandmarkWindowBridge(const FName Name)
    {
        return Name == TEXT("R13_LandmarkWindowGlass") || Name == TEXT("R13_LandmarkWindowFrames");
    }

    bool IsInsideMuseumProtection(const FVector& Location)
    {
        const FVector Delta = Location - AOCWorldSectorOster::MuseumAnchor();

        // Inner historic garden / facade envelope. Generic city dressing stays out; dedicated museum pines and
        // CivicLandscaping are intentionally different component families and therefore remain untouched.
        const float Ellipse = FMath::Square(Delta.X / 6500.0f) + FMath::Square(Delta.Y / 5200.0f);
        if (Ellipse <= 1.0f) return true;

        // Preserve the photo-driven visual corridor toward the main entrance and long museum approach.
        return FMath::Abs(Delta.X - 1180.0f) <= 1750.0f &&
            Delta.Y <= -900.0f && Delta.Y >= -9200.0f;
    }

    bool IsInsidePhotoStadiumProtection(const FVector& Location)
    {
        const FVector Delta = Location - AOCWorldSectorOster::StadiumAnchor();
        // Supplied stadium photos show an open community grass field. Keep random city dressing outside the field
        // and immediate goal/exercise-bar perimeter; dedicated final stadium photo art is a different component family.
        const float Ellipse = FMath::Square(Delta.X / 7200.0f) + FMath::Square(Delta.Y / 5200.0f);
        return Ellipse <= 1.0f;
    }

    bool IsInsideLegacyMuseumWindowZone(const FVector& Location)
    {
        return FVector::DistSquared2D(Location, AOCWorldSectorOster::MuseumAnchor()) <=
            FMath::Square(LegacyMuseumWindowRadiusCm);
    }
}

bool UOCR13MuseumProtectionSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13MuseumProtectionSubsystem::OnWorldBeginPlay(UWorld& InWorld)
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
            if (UWorld* World = WeakWorld.Get()) ApplyMuseumProtection(*World);
        }), ProtectionDelaySeconds, false);
}

void UOCR13MuseumProtectionSubsystem::ApplyMuseumProtection(UWorld& World)
{
    if (bApplied) return;

    int32 RemovedInstances = 0;
    int32 RemovedLegacyWindowInstances = 0;
    int32 RemovedStadiumDressingInstances = 0;
    int32 TouchedComponents = 0;

    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor) continue;

        TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
        Actor->GetComponents(Components);
        for (UInstancedStaticMeshComponent* Component : Components)
        {
            if (!Component) continue;

            const bool bGenericDressing = IsGenericDressingComponent(Component->GetFName());
            const bool bLegacySharedWindow = IsSharedLandmarkWindowBridge(Component->GetFName());
            if (!bGenericDressing && !bLegacySharedWindow) continue;

            int32 RemovedFromComponent = 0;
            for (int32 Index = Component->GetInstanceCount() - 1; Index >= 0; --Index)
            {
                FTransform Transform;
                if (!Component->GetInstanceTransform(Index, Transform, true)) continue;

                bool bRemove = false;
                bool bStadiumRemoval = false;
                if (bGenericDressing)
                {
                    bRemove = IsInsideMuseumProtection(Transform.GetLocation());
                    if (!bRemove)
                    {
                        bStadiumRemoval = IsInsidePhotoStadiumProtection(Transform.GetLocation());
                        bRemove = bStadiumRemoval;
                    }
                }
                else
                {
                    bRemove = IsInsideLegacyMuseumWindowZone(Transform.GetLocation());
                }
                if (!bRemove) continue;

                if (Component->RemoveInstance(Index))
                {
                    ++RemovedFromComponent;
                    ++RemovedInstances;
                    if (bLegacySharedWindow) ++RemovedLegacyWindowInstances;
                    if (bStadiumRemoval) ++RemovedStadiumDressingInstances;
                }
            }
            if (RemovedFromComponent > 0)
            {
                Component->MarkRenderStateDirty();
                ++TouchedComponents;
            }
        }
    }

    bApplied = true;
    UE_LOG(LogTemp, Display,
        TEXT("R13.6 museum/stadium protection: removed %d instances across %d generic/shared components; museum legacy windows=%d, random dressing inside adjacent photo stadium=%d; college windows and dedicated final museum/stadium/civic art untouched."),
        RemovedInstances, TouchedComponents, RemovedLegacyWindowInstances, RemovedStadiumDressingInstances);
}
