#include "OCMuseumLayerPerformanceGuardSubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"

namespace
{
    constexpr float MuseumValidationRadiusCm = 5000.0f;
    constexpr float ValidationDelaySeconds = 1.35f;

    const FName MuseumVisibleOwnerTag(TEXT("R137_MuseumPhotoModel"));
    const FName MuseumCollisionOwnerTag(TEXT("R138_MuseumInteractionCollision"));

    const TSet<FName> ForbiddenSourceFamilies =
    {
        TEXT("LandmarkBlocks"),
        TEXT("LandmarkRoofs"),
        TEXT("LandmarkWindows"),
        TEXT("LandmarkDetails"),
        TEXT("Buildings"),
        TEXT("ResidentialRoofs"),
        TEXT("ResidentialDetails"),
        TEXT("Fences"),
        TEXT("WoodFences"),
        TEXT("MetalFences"),
        TEXT("LightSheetFences"),
        TEXT("TreeTrunks"),
        TEXT("TreeCrowns"),
        TEXT("SovietPoplarTrunks"),
        TEXT("SovietPoplarCrowns"),
        TEXT("BirchTrunks"),
        TEXT("BirchCrowns"),
        TEXT("PineTrunks"),
        TEXT("PineCrowns")
    };

    const TSet<FName> RequiredVisibleR137Components =
    {
        TEXT("R137Museum_Plinth"),
        TEXT("R137Museum_BrickBody"),
        TEXT("R137Museum_BlueGreyTimber"),
        TEXT("R137Museum_SheetMetalRoof"),
        TEXT("R137Museum_CarvedPaleTrim"),
        TEXT("R137Museum_WindowGrilles")
    };

    int32 CountInstancesNear(
        const UInstancedStaticMeshComponent& Component,
        const FVector& Center,
        const float RadiusCm)
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
        ValidationTimer,
        this,
        &UOCMuseumLayerPerformanceGuardSubsystem::RunValidation,
        ValidationDelaySeconds,
        false);

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_MUSEUM_LAYER_VALIDATION_SCHEDULED delay=%.2f mutation=0 repair_pass=0"),
        ValidationDelaySeconds);
}

void UOCMuseumLayerPerformanceGuardSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ValidationTimer);
    }
    Super::Deinitialize();
}

void UOCMuseumLayerPerformanceGuardSubsystem::RunValidation()
{
    UWorld* World = GetWorld();
    if (!World) return;

    const FVector Museum = AOCWorldSectorOster::MuseumAnchor();

    int32 ForbiddenSourceInstances = 0;
    int32 VisibleOwners = 0;
    int32 CollisionOwners = 0;
    int32 MissingVisibleComponents = 0;
    int32 HiddenVisibleComponents = 0;
    int32 CollisionSections = 0;
    int32 VisibleCollisionSections = 0;
    int32 NonCollidingCollisionSections = 0;

    for (TActorIterator<AOCWorldSectorOster> It(World); It; ++It)
    {
        AOCWorldSectorOster* Sector = *It;
        if (!Sector) continue;

        TInlineComponentArray<UInstancedStaticMeshComponent*> SourceComponents;
        Sector->GetComponents(SourceComponents);
        for (UInstancedStaticMeshComponent* Component : SourceComponents)
        {
            if (!Component || !ForbiddenSourceFamilies.Contains(Component->GetFName())) continue;
            ForbiddenSourceInstances += CountInstancesNear(*Component, Museum, MuseumValidationRadiusCm);
        }
    }

    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor || Actor->IsActorBeingDestroyed()) continue;

        if (Actor->ActorHasTag(MuseumVisibleOwnerTag))
        {
            ++VisibleOwners;
            TInlineComponentArray<UPrimitiveComponent*> Components;
            Actor->GetComponents(Components);

            for (const FName RequiredName : RequiredVisibleR137Components)
            {
                UPrimitiveComponent* Found = nullptr;
                for (UPrimitiveComponent* Component : Components)
                {
                    if (Component && Component->GetFName() == RequiredName)
                    {
                        Found = Component;
                        break;
                    }
                }

                if (!Found)
                {
                    ++MissingVisibleComponents;
                    continue;
                }

                if (!Found->IsVisible() || Found->bHiddenInGame)
                {
                    ++HiddenVisibleComponents;
                }
            }
        }

        if (Actor->ActorHasTag(MuseumCollisionOwnerTag))
        {
            ++CollisionOwners;
            TInlineComponentArray<UStaticMeshComponent*> Components;
            Actor->GetComponents(Components);
            for (UStaticMeshComponent* Component : Components)
            {
                if (!Component || !Component->ComponentHasTag(TEXT("MuseumInteractionCollision"))) continue;
                ++CollisionSections;
                if (Component->IsVisible() && !Component->bHiddenInGame) ++VisibleCollisionSections;
                if (Component->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
                {
                    ++NonCollidingCollisionSections;
                }
            }
        }
    }

    const bool bReady =
        ForbiddenSourceInstances == 0 &&
        VisibleOwners == 1 &&
        CollisionOwners == 1 &&
        MissingVisibleComponents == 0 &&
        HiddenVisibleComponents == 0 &&
        CollisionSections > 0 &&
        VisibleCollisionSections == 0 &&
        NonCollidingCollisionSections == 0;

    if (bReady)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_MUSEUM_LAYER_VALIDATION_READY source_overlap=0 visible_owner=1 collision_owner=1 required_visible_missing=0 required_visible_hidden=0 collision_sections=%d collision_visible=0 collision_disabled=0 mutation=0"),
            CollisionSections);
    }
    else
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_MUSEUM_LAYER_VALIDATION_FAIL source_overlap=%d visible_owner=%d collision_owner=%d required_visible_missing=%d required_visible_hidden=%d collision_sections=%d collision_visible=%d collision_disabled=%d mutation=0 primary_authoring_fix_required=1"),
            ForbiddenSourceInstances,
            VisibleOwners,
            CollisionOwners,
            MissingVisibleComponents,
            HiddenVisibleComponents,
            CollisionSections,
            VisibleCollisionSections,
            NonCollidingCollisionSections);
    }
}
