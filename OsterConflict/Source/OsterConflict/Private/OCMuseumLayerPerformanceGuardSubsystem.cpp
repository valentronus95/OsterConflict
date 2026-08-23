#include "OCMuseumLayerPerformanceGuardSubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"

namespace
{
    constexpr float MuseumCleanupRadiusCm = 5000.0f;
    constexpr float RepairDelaySeconds = 0.35f;
    constexpr float FinalValidationDelaySeconds = 1.25f;

    const FName MuseumPrototypeTag(TEXT("R137_MuseumPhotoModel"));
    const FName MuseumArchitectureTag(TEXT("R138_MuseumHighFidelityArchitecture"));

    const TSet<FName> SourceFamiliesToRemove =
    {
        // Historical museum shell families.
        TEXT("LandmarkBlocks"),
        TEXT("LandmarkRoofs"),
        TEXT("LandmarkWindows"),
        TEXT("LandmarkDetails"),

        // Defensive cleanup: generic procedural city geometry must never occupy the dedicated museum site.
        TEXT("Buildings"),
        TEXT("ResidentialRoofs"),
        TEXT("ResidentialDetails"),
        TEXT("Fences"),
        TEXT("WoodFences"),
        TEXT("MetalFences"),
        TEXT("LightSheetFences"),

        // The R14.5 photo-oriented tree layout is authoritative at the museum. Remove source proxy vegetation.
        TEXT("TreeTrunks"),
        TEXT("TreeCrowns"),
        TEXT("SovietPoplarTrunks"),
        TEXT("SovietPoplarCrowns"),
        TEXT("BirchTrunks"),
        TEXT("BirchCrowns"),
        TEXT("PineTrunks"),
        TEXT("PineCrowns")
    };

    const TSet<FName> ObsoleteR137Components =
    {
        // R13.8 owns the enterable shell and interactive openings.
        TEXT("R137Museum_BrickBody"),
        TEXT("R137Museum_BlueGreyTimber"),
        TEXT("R137Museum_WindowGlass"),
        TEXT("R137Museum_WindowGrilles"),
        TEXT("R137Museum_CarvedPaleTrim"),
        TEXT("R137Museum_GreyDoors"),

        // R14.5 owns the final photo-oriented tree layout.
        TEXT("R137Museum_Pine01"),
        TEXT("R137Museum_Pine03"),
        TEXT("R137Museum_Deciduous01")
    };

    bool HasAnyTag(const AActor& Actor, std::initializer_list<FName> Tags)
    {
        for (const FName Tag : Tags)
        {
            if (Actor.ActorHasTag(Tag)) return true;
        }
        return false;
    }

    int32 ResolveDecorCullDistance(const AActor& Actor)
    {
        if (Actor.ActorHasTag(TEXT("R143_MuseumSiteVegetation"))) return 15000;
        if (Actor.ActorHasTag(TEXT("R145_MuseumPhotoTreeLayout"))) return 45000;
        if (Actor.ActorHasTag(MuseumPrototypeTag)) return 35000;
        return 30000;
    }

    int32 RemoveInstancesNear(UInstancedStaticMeshComponent& Component, const FVector& Center, const float RadiusCm)
    {
        const float RadiusSq = FMath::Square(RadiusCm);
        int32 Removed = 0;
        for (int32 Index = Component.GetInstanceCount() - 1; Index >= 0; --Index)
        {
            FTransform Transform;
            if (!Component.GetInstanceTransform(Index, Transform, true)) continue;
            if (FVector::DistSquared2D(Transform.GetLocation(), Center) > RadiusSq) continue;
            if (Component.RemoveInstance(Index)) ++Removed;
        }
        if (Removed > 0) Component.MarkRenderStateDirty();
        return Removed;
    }

    int32 CountInstancesNear(const UInstancedStaticMeshComponent& Component, const FVector& Center, const float RadiusCm)
    {
        const float RadiusSq = FMath::Square(RadiusCm);
        int32 Count = 0;
        for (int32 Index = 0; Index < Component.GetInstanceCount(); ++Index)
        {
            FTransform Transform;
            if (!Component.GetInstanceTransform(Index, Transform, true)) continue;
            if (FVector::DistSquared2D(Transform.GetLocation(), Center) <= RadiusSq) ++Count;
        }
        return Count;
    }
}

bool UOCMuseumLayerPerformanceGuardSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCMuseumLayerPerformanceGuardSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    InWorld.GetTimerManager().SetTimer(
        RepairTimer,
        this,
        &UOCMuseumLayerPerformanceGuardSubsystem::RunRepairPass,
        RepairDelaySeconds,
        false);

    InWorld.GetTimerManager().SetTimer(
        ValidationTimer,
        this,
        &UOCMuseumLayerPerformanceGuardSubsystem::RunFinalValidation,
        FinalValidationDelaySeconds,
        false);
}

void UOCMuseumLayerPerformanceGuardSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(RepairTimer);
        World->GetTimerManager().ClearTimer(ValidationTimer);
    }
    Super::Deinitialize();
}

void UOCMuseumLayerPerformanceGuardSubsystem::RunRepairPass()
{
    UWorld* World = GetWorld();
    if (!World) return;

    const FVector Museum = AOCWorldSectorOster::MuseumAnchor();
    int32 RemovedSourceInstances = 0;
    int32 HiddenPrototypeComponents = 0;
    int32 TunedDecorativeComponents = 0;

    // The source sector used to contain a large generic museum shell plus proxy fences/trees. R13.7-R14.5
    // replace those families. Strip every remaining source instance inside the dedicated 50 m museum site so
    // a future source-world edit cannot silently recreate the "building inside building" failure.
    for (TActorIterator<AOCWorldSectorOster> It(World); It; ++It)
    {
        AOCWorldSectorOster* Sector = *It;
        if (!Sector) continue;

        TInlineComponentArray<UInstancedStaticMeshComponent*> SourceComponents;
        Sector->GetComponents(SourceComponents);
        for (UInstancedStaticMeshComponent* Component : SourceComponents)
        {
            if (!Component || !SourceFamiliesToRemove.Contains(Component->GetFName())) continue;
            RemovedSourceInstances += RemoveInstancesNear(*Component, Museum, MuseumCleanupRadiusCm);
        }
    }

    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor || Actor->IsActorBeingDestroyed()) continue;

        if (Actor->ActorHasTag(MuseumPrototypeTag))
        {
            TInlineComponentArray<UPrimitiveComponent*> PrototypeComponents;
            Actor->GetComponents(PrototypeComponents);
            for (UPrimitiveComponent* Component : PrototypeComponents)
            {
                if (!Component) continue;

                if (ObsoleteR137Components.Contains(Component->GetFName()))
                {
                    Component->SetVisibility(false, true);
                    Component->SetHiddenInGame(true, true);
                    Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                    Component->SetGenerateOverlapEvents(false);
                    Component->SetCanEverAffectNavigation(false);
                    Component->SetCastShadow(false);
                    Component->MarkRenderStateDirty();
                    ++HiddenPrototypeComponents;
                }
            }
        }

        const bool bMuseumDecorActor = HasAnyTag(*Actor,
        {
            MuseumPrototypeTag,
            MuseumArchitectureTag,
            TEXT("R140_MuseumFacadeDetail"),
            TEXT("R142_MuseumEntranceDetail"),
            TEXT("R143_MuseumSiteVegetation"),
            TEXT("R144_MuseumRearExteriorDetail"),
            TEXT("R145_MuseumPhotoTreeLayout")
        });
        if (!bMuseumDecorActor) continue;

        const int32 CullEndCm = ResolveDecorCullDistance(*Actor);
        TInlineComponentArray<UInstancedStaticMeshComponent*> DetailComponents;
        Actor->GetComponents(DetailComponents);
        for (UInstancedStaticMeshComponent* Component : DetailComponents)
        {
            if (!Component || ObsoleteR137Components.Contains(Component->GetFName())) continue;
            Component->SetCastShadow(false);
            Component->SetCullDistances(0, CullEndCm);
            if (Component->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
            {
                Component->SetCanEverAffectNavigation(false);
            }
            Component->MarkRenderStateDirty();
            ++TunedDecorativeComponents;
        }
    }

    TotalSourceInstancesRemoved += RemovedSourceInstances;
    TotalObsoletePrototypeComponentsHidden += HiddenPrototypeComponents;
    TotalDecorativeComponentsTuned += TunedDecorativeComponents;

    UE_LOG(LogTemp, Display,
        TEXT("PASS32_MUSEUM_LAYER_REPAIR source_removed=%d prototype_hidden=%d decor_tuned=%d radius_m=%.1f"),
        RemovedSourceInstances,
        HiddenPrototypeComponents,
        TunedDecorativeComponents,
        MuseumCleanupRadiusCm / 100.0f);
}

void UOCMuseumLayerPerformanceGuardSubsystem::RunFinalValidation()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Repeat once after the authoritative startup window. This is intentionally idempotent and catches a late
    // actor/component registration without adding a permanent tick.
    RunRepairPass();

    const FVector Museum = AOCWorldSectorOster::MuseumAnchor();
    int32 ForbiddenSourceInstances = 0;
    int32 VisibleObsoletePrototypeComponents = 0;
    int32 PrototypeOwners = 0;
    int32 ArchitectureOwners = 0;

    for (TActorIterator<AOCWorldSectorOster> It(World); It; ++It)
    {
        AOCWorldSectorOster* Sector = *It;
        if (!Sector) continue;
        TInlineComponentArray<UInstancedStaticMeshComponent*> SourceComponents;
        Sector->GetComponents(SourceComponents);
        for (UInstancedStaticMeshComponent* Component : SourceComponents)
        {
            if (!Component || !SourceFamiliesToRemove.Contains(Component->GetFName())) continue;
            ForbiddenSourceInstances += CountInstancesNear(*Component, Museum, MuseumCleanupRadiusCm);
        }
    }

    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor || Actor->IsActorBeingDestroyed()) continue;
        if (Actor->ActorHasTag(MuseumPrototypeTag))
        {
            ++PrototypeOwners;
            TInlineComponentArray<UPrimitiveComponent*> Components;
            Actor->GetComponents(Components);
            for (UPrimitiveComponent* Component : Components)
            {
                if (!Component || !ObsoleteR137Components.Contains(Component->GetFName())) continue;
                if (Component->IsVisible() && !Component->bHiddenInGame) ++VisibleObsoletePrototypeComponents;
            }
        }
        if (Actor->ActorHasTag(MuseumArchitectureTag)) ++ArchitectureOwners;
    }

    const bool bReady = ForbiddenSourceInstances == 0 &&
        VisibleObsoletePrototypeComponents == 0 &&
        PrototypeOwners == 1 && ArchitectureOwners == 1;

    if (bReady)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS32_MUSEUM_LAYER_BUDGET_READY source_near_site=0 obsolete_visible=0 prototypeOwners=%d architectureOwners=%d total_source_removed=%d total_prototype_hidden=%d total_decor_tuned=%d"),
            PrototypeOwners,
            ArchitectureOwners,
            TotalSourceInstancesRemoved,
            TotalObsoletePrototypeComponentsHidden,
            TotalDecorativeComponentsTuned);
    }
    else
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS32_MUSEUM_LAYER_BUDGET_FAIL source_near_site=%d obsolete_visible=%d prototypeOwners=%d architectureOwners=%d total_source_removed=%d total_prototype_hidden=%d total_decor_tuned=%d"),
            ForbiddenSourceInstances,
            VisibleObsoletePrototypeComponents,
            PrototypeOwners,
            ArchitectureOwners,
            TotalSourceInstancesRemoved,
            TotalObsoletePrototypeComponentsHidden,
            TotalDecorativeComponentsTuned);
    }
}
