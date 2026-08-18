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
    int32 TouchedComponents = 0;

    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor) continue;

        TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
        Actor->GetComponents(Components);
        for (UInstancedStaticMeshComponent* Component : Components)
        {
            if (!Component || !IsGenericDressingComponent(Component->GetFName())) continue;

            int32 RemovedFromComponent = 0;
            for (int32 Index = Component->GetInstanceCount() - 1; Index >= 0; --Index)
            {
                FTransform Transform;
                if (!Component->GetInstanceTransform(Index, Transform, true)) continue;
                if (!IsInsideMuseumProtection(Transform.GetLocation())) continue;
                if (Component->RemoveInstance(Index))
                {
                    ++RemovedFromComponent;
                    ++RemovedInstances;
                }
            }
            if (RemovedFromComponent > 0) ++TouchedComponents;
        }
    }

    bApplied = true;
    UE_LOG(LogTemp, Display,
        TEXT("R13.5 museum protection: removed %d generic dressing instances across %d components from the historic-garden / entrance-view corridor; dedicated museum/civic art untouched."),
        RemovedInstances, TouchedComponents);
}
