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
    constexpr float R138MuseumValidationDelaySeconds = 6.55f;
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

    int32 ArchitectureActors = 0;
    int32 FacadeDetailActors = 0;
    int32 EntranceDetailActors = 0;
    int32 SiteVegetationActors = 0;
    int32 StructuralSections = 0;
    int32 MainDoorActors = 0;
    int32 ServiceDoorActors = 0;
    int32 PrototypeServiceDoors = 0;
    int32 BreakableWindows = 0;
    int32 StyledMuseumWindows = 0;
    int32 PrototypeMuseumWindows = 0;
    int32 UpperGableWindows = 0;
    int32 InitiallyBrokenWindows = 0;

    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor) continue;

        if (Actor->ActorHasTag(TEXT("R138_MuseumHighFidelityArchitecture")))
        {
            ++ArchitectureActors;
            TInlineComponentArray<UStaticMeshComponent*> Components;
            Actor->GetComponents(Components);
            for (UStaticMeshComponent* Component : Components)
            {
                if (Component && Component->ComponentHasTag(TEXT("MuseumStructural")))
                {
                    ++StructuralSections;
                }
            }
        }

        if (Actor->ActorHasTag(TEXT("R140_MuseumFacadeDetail"))) ++FacadeDetailActors;
        if (Actor->ActorHasTag(TEXT("R142_MuseumEntranceDetail"))) ++EntranceDetailActors;
        if (Actor->ActorHasTag(TEXT("R143_MuseumSiteVegetation"))) ++SiteVegetationActors;

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

    const bool bPass = ArchitectureActors == 1 &&
        FacadeDetailActors == 1 &&
        EntranceDetailActors == 1 &&
        SiteVegetationActors == 1 &&
        StructuralSections >= 30 &&
        MainDoorActors == 1 &&
        ServiceDoorActors == 1 &&
        PrototypeServiceDoors == 0 &&
        BreakableWindows >= 21 &&
        StyledMuseumWindows == BreakableWindows &&
        PrototypeMuseumWindows == 0 &&
        UpperGableWindows == 1 &&
        InitiallyBrokenWindows == 0;

    if (bPass)
    {
        UE_LOG(LogTemp, Display,
            TEXT("R14.3 museum validation PASS: architecture=%d facade=%d entrance=%d vegetation=%d structural=%d mainDoor=%d serviceDoor=%d styledWindows=%d upperGable=%d."),
            ArchitectureActors, FacadeDetailActors, EntranceDetailActors, SiteVegetationActors,
            StructuralSections, MainDoorActors, ServiceDoorActors, StyledMuseumWindows, UpperGableWindows);
        return;
    }

    UE_LOG(LogTemp, Warning,
        TEXT("R14.3 museum validation FAILED: architecture=%d facade=%d entrance=%d vegetation=%d structural=%d mainDoor=%d serviceDoor=%d prototypeService=%d windows=%d styled=%d prototypeWindows=%d upperGable=%d initiallyBroken=%d."),
        ArchitectureActors, FacadeDetailActors, EntranceDetailActors, SiteVegetationActors,
        StructuralSections, MainDoorActors, ServiceDoorActors, PrototypeServiceDoors,
        BreakableWindows, StyledMuseumWindows, PrototypeMuseumWindows, UpperGableWindows, InitiallyBrokenWindows);
}
