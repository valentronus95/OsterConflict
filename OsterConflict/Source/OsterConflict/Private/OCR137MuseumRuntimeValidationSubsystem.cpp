#include "OCR137MuseumRuntimeValidationSubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

namespace
{
    constexpr float MuseumValidationDelaySeconds = 5.65f;
    constexpr float SourceLandmarkCheckRadiusCm = 3600.0f;
    constexpr float SourceFenceCheckRadiusCm = 3300.0f;
    constexpr float TreeCheckRadiusCm = 5200.0f;
    const FName FinalMuseumTag(TEXT("R137_MuseumPhotoModel"));

    bool IsLegacyMuseumPresentation(const FName Name)
    {
        return Name.ToString().StartsWith(TEXT("R13_Museum"));
    }

    bool IsSourceMuseumLandmarkFamily(const FName Name)
    {
        return Name == TEXT("LandmarkBlocks") ||
            Name == TEXT("LandmarkRoofs") ||
            Name == TEXT("LandmarkWindows") ||
            Name == TEXT("LandmarkDetails");
    }

    bool IsSourceFenceFamily(const FName Name)
    {
        return Name == TEXT("Fences") ||
            Name == TEXT("WoodFences") ||
            Name == TEXT("MetalFences") ||
            Name == TEXT("LightSheetFences");
    }

    bool IsAssetDecoratorTreeFamily(const FName Name)
    {
        return Name == TEXT("RealTreeA") ||
            Name == TEXT("RealTreeB") ||
            Name == TEXT("RealTreeC") ||
            Name == TEXT("RealPineA") ||
            Name == TEXT("RealPineB");
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

bool UOCR137MuseumRuntimeValidationSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR137MuseumRuntimeValidationSubsystem::OnWorldBeginPlay(UWorld& InWorld)
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
            if (UWorld* World = WeakWorld.Get()) ValidateMuseumReplacement(*World);
        }), MuseumValidationDelaySeconds, false);
}

void UOCR137MuseumRuntimeValidationSubsystem::ValidateMuseumReplacement(UWorld& World)
{
    const FVector Museum = AOCWorldSectorOster::MuseumAnchor();

    int32 FinalModelActors = 0;
    int32 FinalModelComponents = 0;
    int32 FinalModelInstances = 0;
    int32 CollidableFinalComponents = 0;
    int32 FinalInstancesNearAnchor = 0;
    int32 VisibleLegacyComponents = 0;
    int32 SourceLandmarkInstancesNearMuseum = 0;
    int32 SourceFenceInstancesNearMuseum = 0;
    int32 DecoratorTreeInstancesNearMuseum = 0;

    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor) continue;

        const bool bFinalMuseumActor = Actor->ActorHasTag(FinalMuseumTag);
        if (bFinalMuseumActor) ++FinalModelActors;

        TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
        Actor->GetComponents(Components);
        for (UInstancedStaticMeshComponent* Component : Components)
        {
            if (!Component) continue;
            const FName Name = Component->GetFName();

            if (bFinalMuseumActor)
            {
                ++FinalModelComponents;
                FinalModelInstances += Component->GetInstanceCount();
                if (Component->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
                {
                    ++CollidableFinalComponents;
                }
                FinalInstancesNearAnchor += CountInstancesNear(Component, Museum, SourceLandmarkCheckRadiusCm);
            }

            if (IsLegacyMuseumPresentation(Name) && Component->IsVisible())
            {
                ++VisibleLegacyComponents;
            }

            if (IsSourceMuseumLandmarkFamily(Name))
            {
                SourceLandmarkInstancesNearMuseum +=
                    CountInstancesNear(Component, Museum, SourceLandmarkCheckRadiusCm);
            }
            else if (IsSourceFenceFamily(Name))
            {
                SourceFenceInstancesNearMuseum +=
                    CountInstancesNear(Component, Museum, SourceFenceCheckRadiusCm);
            }
            else if (IsAssetDecoratorTreeFamily(Name))
            {
                DecoratorTreeInstancesNearMuseum +=
                    CountInstancesNear(Component, Museum, TreeCheckRadiusCm);
            }
        }
    }

    const bool bPass =
        FinalModelActors == 1 &&
        FinalModelComponents > 0 &&
        FinalModelInstances > 0 &&
        CollidableFinalComponents > 0 &&
        FinalInstancesNearAnchor > 0 &&
        VisibleLegacyComponents == 0 &&
        SourceLandmarkInstancesNearMuseum == 0 &&
        SourceFenceInstancesNearMuseum == 0 &&
        DecoratorTreeInstancesNearMuseum == 0;

    if (bPass)
    {
        UE_LOG(LogTemp, Display,
            TEXT("R13.7 museum validation PASS: finalActors=%d components=%d instances=%d collidableComponents=%d nearAnchor=%d; no legacy/source/decorator residue."),
            FinalModelActors, FinalModelComponents, FinalModelInstances,
            CollidableFinalComponents, FinalInstancesNearAnchor);
        return;
    }

    UE_LOG(LogTemp, Warning,
        TEXT("R13.7 museum validation FAILED: finalActors=%d components=%d instances=%d collidableComponents=%d nearAnchor=%d visibleLegacy=%d sourceLandmarkResidue=%d sourceFenceResidue=%d decoratorTreeResidue=%d."),
        FinalModelActors, FinalModelComponents, FinalModelInstances,
        CollidableFinalComponents, FinalInstancesNearAnchor, VisibleLegacyComponents,
        SourceLandmarkInstancesNearMuseum, SourceFenceInstancesNearMuseum,
        DecoratorTreeInstancesNearMuseum);
}
