#include "OCR138MuseumRuntimeValidationSubsystem.h"

#include "OCBreakableWindow.h"
#include "OCGameMode.h"
#include "OCInteractableDoor.h"
#include "OCMuseumDoubleDoor.h"
#include "OCWorldSectorOster.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

namespace
{
    constexpr float R138MuseumValidationDelaySeconds = 5.95f;
    constexpr float MuseumInteractionRadiusCm = 2500.0f;
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
    int32 StructuralSections = 0;
    int32 MainDoorActors = 0;
    int32 ServiceDoors = 0;
    int32 BreakableWindows = 0;
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

        if (!Actor->ActorHasTag(TEXT("R138_MuseumInteractive")) ||
            FVector::DistSquared2D(Actor->GetActorLocation(), Museum) > RadiusSq)
        {
            continue;
        }

        if (AOCMuseumDoubleDoor* MainDoor = Cast<AOCMuseumDoubleDoor>(Actor))
        {
            if (MainDoor->ActorHasTag(TEXT("MuseumMainDoubleDoor"))) ++MainDoorActors;
            continue;
        }

        if (AOCInteractableDoor* Door = Cast<AOCInteractableDoor>(Actor))
        {
            if (Door->ActorHasTag(TEXT("MuseumServiceDoor"))) ++ServiceDoors;
            continue;
        }

        if (AOCBreakableWindow* Window = Cast<AOCBreakableWindow>(Actor))
        {
            ++BreakableWindows;
            if (Window->IsBroken()) ++InitiallyBrokenWindows;
        }
    }

    const bool bPass = ArchitectureActors == 1 &&
        StructuralSections >= 30 &&
        MainDoorActors == 1 &&
        ServiceDoors == 1 &&
        BreakableWindows >= 20 &&
        InitiallyBrokenWindows == 0;

    if (bPass)
    {
        UE_LOG(LogTemp, Display,
            TEXT("R13.8 museum validation PASS: architectureActors=%d structuralSections=%d mainDoorActors=%d serviceDoors=%d breakableWindows=%d."),
            ArchitectureActors, StructuralSections, MainDoorActors, ServiceDoors, BreakableWindows);
        return;
    }

    UE_LOG(LogTemp, Warning,
        TEXT("R13.8 museum validation FAILED: architectureActors=%d structuralSections=%d mainDoorActors=%d serviceDoors=%d breakableWindows=%d initiallyBrokenWindows=%d."),
        ArchitectureActors, StructuralSections, MainDoorActors, ServiceDoors, BreakableWindows, InitiallyBrokenWindows);
}
