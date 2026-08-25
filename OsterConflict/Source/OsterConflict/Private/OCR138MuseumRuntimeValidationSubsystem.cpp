#include "OCR138MuseumRuntimeValidationSubsystem.h"

#include "OCBreakableWindow.h"
#include "OCGameMode.h"
#include "OCInteractableDoor.h"
#include "OCMuseumBreakableWindow.h"
#include "OCMuseumDoubleDoor.h"
#include "OCMuseumServiceDoubleDoor.h"
#include "OCWorldSectorOster.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

namespace
{
    constexpr float R138MuseumValidationDelaySeconds = 6.75f;
    constexpr float MuseumInteractionRadiusCm = 2800.0f;
}

bool UOCR138MuseumRuntimeValidationSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR138MuseumRuntimeValidationSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (InWorld.GetNetMode() == NM_Client) return;

    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle Timer;
    InWorld.GetTimerManager().SetTimer(Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) ValidateMuseum(*World);
        }), R138MuseumValidationDelaySeconds, false);
}

void UOCR138MuseumRuntimeValidationSubsystem::ValidateMuseum(UWorld& World) const
{
    const FVector Museum = AOCWorldSectorOster::MuseumAnchor();
    const float RadiusSq = FMath::Square(MuseumInteractionRadiusCm);

    int32 R137VisibleExteriorActors = 0;
    int32 CollisionActors = 0;
    int32 CollisionSections = 0;
    int32 VisibleCollisionSections = 0;
    int32 FacadeDetailActors = 0;
    int32 EntranceDetailActors = 0;
    int32 SiteVegetationActors = 0;
    int32 RearExteriorActors = 0;
    int32 TreeLayoutActors = 0;
    int32 MainDoorActors = 0;
    int32 ServiceDoorActors = 0;
    int32 PrototypeServiceDoors = 0;
    int32 PrototypeMainDoorLeaves = 0;
    int32 BreakableWindows = 0;
    int32 StyledMuseumWindows = 0;
    int32 PrototypeMuseumWindows = 0;
    int32 UpperGableWindows = 0;
    int32 InitiallyBrokenWindows = 0;

    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor || Actor->IsActorBeingDestroyed()) continue;

        if (Actor->ActorHasTag(TEXT("R137_MuseumPhotoModel")))
        {
            ++R137VisibleExteriorActors;
        }

        if (Actor->ActorHasTag(TEXT("R138_MuseumInteractionCollision")))
        {
            ++CollisionActors;
            TInlineComponentArray<UStaticMeshComponent*> Components;
            Actor->GetComponents(Components);
            for (UStaticMeshComponent* Component : Components)
            {
                if (!Component || !Component->ComponentHasTag(TEXT("MuseumInteractionCollision"))) continue;
                ++CollisionSections;
                if (Component->IsVisible()) ++VisibleCollisionSections;
            }
        }

        if (Actor->ActorHasTag(TEXT("R140_MuseumFacadeDetail"))) ++FacadeDetailActors;
        if (Actor->ActorHasTag(TEXT("R142_MuseumEntranceDetail"))) ++EntranceDetailActors;
        if (Actor->ActorHasTag(TEXT("R143_MuseumSiteVegetation"))) ++SiteVegetationActors;
        if (Actor->ActorHasTag(TEXT("R144_MuseumRearExteriorDetail"))) ++RearExteriorActors;
        if (Actor->ActorHasTag(TEXT("R145_MuseumPhotoTreeLayout"))) ++TreeLayoutActors;

        if (FVector::DistSquared2D(Actor->GetActorLocation(), Museum) > RadiusSq)
        {
            continue;
        }

        if (AOCMuseumDoubleDoor* MainDoor = Cast<AOCMuseumDoubleDoor>(Actor))
        {
            if (MainDoor->ActorHasTag(TEXT("MuseumMainDoubleDoor"))) ++MainDoorActors;
            continue;
        }

        if (AOCMuseumServiceDoubleDoor* ServiceDoor = Cast<AOCMuseumServiceDoubleDoor>(Actor))
        {
            if (ServiceDoor->ActorHasTag(TEXT("MuseumServiceDoubleDoor"))) ++ServiceDoorActors;
            continue;
        }

        if (AOCInteractableDoor* Door = Cast<AOCInteractableDoor>(Actor))
        {
            if (Door->ActorHasTag(TEXT("MuseumServiceDoor"))) ++PrototypeServiceDoors;
            if (Door->ActorHasTag(TEXT("MuseumMainDoorLeft")) || Door->ActorHasTag(TEXT("MuseumMainDoorRight")))
            {
                ++PrototypeMainDoorLeaves;
            }
            continue;
        }

        if (AOCBreakableWindow* Window = Cast<AOCBreakableWindow>(Actor))
        {
            if (!Actor->ActorHasTag(TEXT("R138_MuseumInteractive")) &&
                !Actor->ActorHasTag(TEXT("R140_MuseumInteractive")))
            {
                continue;
            }

            ++BreakableWindows;
            if (Cast<AOCMuseumBreakableWindow>(Window)) ++StyledMuseumWindows;
            else ++PrototypeMuseumWindows;
            if (Window->ActorHasTag(TEXT("MuseumUpperGableWindow"))) ++UpperGableWindows;
            if (Window->IsBroken()) ++InitiallyBrokenWindows;
        }
    }

    const bool bPass = R137VisibleExteriorActors == 1 &&
        CollisionActors == 1 &&
        CollisionSections >= 30 &&
        VisibleCollisionSections == 0 &&
        FacadeDetailActors == 1 &&
        EntranceDetailActors == 1 &&
        SiteVegetationActors == 1 &&
        RearExteriorActors == 1 &&
        TreeLayoutActors == 1 &&
        MainDoorActors == 1 &&
        ServiceDoorActors == 1 &&
        PrototypeServiceDoors == 0 &&
        PrototypeMainDoorLeaves == 0 &&
        BreakableWindows >= 21 &&
        StyledMuseumWindows == BreakableWindows &&
        PrototypeMuseumWindows == 0 &&
        UpperGableWindows == 1 &&
        InitiallyBrokenWindows == 0;

    if (bPass)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_MUSEUM_RUNTIME_SINGLE_OWNER_READY r137_visible=%d collision_owners=%d collision_sections=%d visible_collision_sections=%d facade=%d entrance=%d vegetation=%d rearExterior=%d trees=%d mainDoor=%d serviceDoor=%d styledWindows=%d prototypeDoors=0 prototypeWindows=0 upperGable=%d"),
            R137VisibleExteriorActors, CollisionActors, CollisionSections, VisibleCollisionSections,
            FacadeDetailActors, EntranceDetailActors, SiteVegetationActors, RearExteriorActors,
            TreeLayoutActors, MainDoorActors, ServiceDoorActors, StyledMuseumWindows, UpperGableWindows);
        UE_LOG(LogTemp, Display,
            TEXT("R14.5 museum validation PASS: single visible R13.7 exterior + hidden R13.8 interaction collision + final doors/windows/details."));
        return;
    }

    UE_LOG(LogTemp, Warning,
        TEXT("PASS45_MUSEUM_RUNTIME_SINGLE_OWNER_FAIL r137_visible=%d collision_owners=%d collision_sections=%d visible_collision_sections=%d facade=%d entrance=%d vegetation=%d rearExterior=%d trees=%d mainDoor=%d serviceDoor=%d prototypeService=%d prototypeMainLeaves=%d windows=%d styled=%d prototypeWindows=%d upperGable=%d initiallyBroken=%d"),
        R137VisibleExteriorActors, CollisionActors, CollisionSections, VisibleCollisionSections,
        FacadeDetailActors, EntranceDetailActors, SiteVegetationActors, RearExteriorActors,
        TreeLayoutActors, MainDoorActors, ServiceDoorActors, PrototypeServiceDoors,
        PrototypeMainDoorLeaves, BreakableWindows, StyledMuseumWindows, PrototypeMuseumWindows,
        UpperGableWindows, InitiallyBrokenWindows);
    UE_LOG(LogTemp, Warning,
        TEXT("R14.5 museum validation FAILED: current Pass45 single-owner runtime contract was not satisfied."));
}
