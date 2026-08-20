#include "OCR137MuseumSiteReplacementSubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

namespace
{
    // R13.7 final museum model starts at 5.10 s. Keep cleanup just before it.
    constexpr float MuseumSiteCleanupDelaySeconds = 4.95f;
    constexpr float LandmarkCleanupRadiusCm = 3600.0f;
    constexpr float FenceCleanupRadiusCm = 3300.0f;
    constexpr float TreeCleanupRadiusCm = 5200.0f;

    bool RemoveInstancesNear(UInstancedStaticMeshComponent* Component, const FVector& Center, const float RadiusCm)
    {
        if (!Component) return false;

        const float RadiusSq = FMath::Square(RadiusCm);
        bool bChanged = false;
        for (int32 Index = Component->GetInstanceCount() - 1; Index >= 0; --Index)
        {
            FTransform Transform;
            if (!Component->GetInstanceTransform(Index, Transform, true)) continue;
            if (FVector::DistSquared2D(Transform.GetLocation(), Center) > RadiusSq) continue;
            if (Component->RemoveInstance(Index)) bChanged = true;
        }

        if (bChanged) Component->MarkRenderStateDirty();
        return bChanged;
    }

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

    bool IsSourcePrimitiveTreeFamily(const FName Name)
    {
        return Name == TEXT("TreeTrunks") || Name == TEXT("TreeCrowns") ||
            Name == TEXT("SovietPoplarTrunks") || Name == TEXT("SovietPoplarCrowns") ||
            Name == TEXT("BirchTrunks") || Name == TEXT("BirchCrowns") ||
            Name == TEXT("PineTrunks") || Name == TEXT("PineCrowns");
    }

    bool IsAssetDecoratorTreeFamily(const FName Name)
    {
        // OCAssetModelDecorator owns these ISMs. Its generic museum garden must be removed before
        // OCR137MuseumPhotoModelSubsystem places the photo-specific museum vegetation.
        return Name == TEXT("RealTreeA") ||
            Name == TEXT("RealTreeB") ||
            Name == TEXT("RealTreeC") ||
            Name == TEXT("RealPineA") ||
            Name == TEXT("RealPineB");
    }
}

bool UOCR137MuseumSiteReplacementSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR137MuseumSiteReplacementSubsystem::OnWorldBeginPlay(UWorld& InWorld)
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
            if (UWorld* World = WeakWorld.Get()) PrepareMuseumSite(*World);
        }), MuseumSiteCleanupDelaySeconds, false);
}

void UOCR137MuseumSiteReplacementSubsystem::PrepareMuseumSite(UWorld& World)
{
    const FVector Museum = AOCWorldSectorOster::MuseumAnchor();

    int32 HiddenLegacyComponents = 0;
    int32 TrimmedLandmarkFamilies = 0;
    int32 TrimmedFenceFamilies = 0;
    int32 TrimmedPrimitiveTreeFamilies = 0;
    int32 TrimmedDecoratorTreeFamilies = 0;

    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor) continue;

        TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
        Actor->GetComponents(Components);
        for (UInstancedStaticMeshComponent* Component : Components)
        {
            if (!Component) continue;
            const FName Name = Component->GetFName();

            if (IsLegacyMuseumPresentation(Name))
            {
                Component->SetVisibility(false, true);
                Component->SetHiddenInGame(true, true);
                Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                ++HiddenLegacyComponents;
                continue;
            }

            if (IsSourceMuseumLandmarkFamily(Name) &&
                RemoveInstancesNear(Component, Museum, LandmarkCleanupRadiusCm))
            {
                ++TrimmedLandmarkFamilies;
                continue;
            }

            if (IsSourceFenceFamily(Name) &&
                RemoveInstancesNear(Component, Museum, FenceCleanupRadiusCm))
            {
                ++TrimmedFenceFamilies;
                continue;
            }

            if (IsSourcePrimitiveTreeFamily(Name) &&
                RemoveInstancesNear(Component, Museum, TreeCleanupRadiusCm))
            {
                ++TrimmedPrimitiveTreeFamilies;
                continue;
            }

            if (IsAssetDecoratorTreeFamily(Name) &&
                RemoveInstancesNear(Component, Museum, TreeCleanupRadiusCm))
            {
                ++TrimmedDecoratorTreeFamilies;
            }
        }
    }

    UE_LOG(LogTemp, Display,
        TEXT("R13.7 museum site replacement: legacy=%d landmark=%d fence=%d primitiveTrees=%d decoratorTrees=%d. Final photo model follows at MuseumAnchor."),
        HiddenLegacyComponents, TrimmedLandmarkFamilies, TrimmedFenceFamilies,
        TrimmedPrimitiveTreeFamilies, TrimmedDecoratorTreeFamilies);
}
