#include "OCR139MuseumMainDoorReplacementSubsystem.h"

#include "OCGameMode.h"
#include "OCInteractableDoor.h"
#include "OCMuseumDoubleDoor.h"
#include "OCWorldSectorOster.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

namespace
{
    constexpr float R139MuseumMainDoorDelaySeconds = 5.55f;
    constexpr float MuseumDoorRadiusCm = 2500.0f;
}

bool UOCR139MuseumMainDoorReplacementSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR139MuseumMainDoorReplacementSubsystem::OnWorldBeginPlay(UWorld& InWorld)
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
            if (UWorld* World = WeakWorld.Get()) ReplaceMainDoor(*World);
        }), R139MuseumMainDoorDelaySeconds, false);
}

void UOCR139MuseumMainDoorReplacementSubsystem::ReplaceMainDoor(UWorld& World) const
{
    const FVector Museum = AOCWorldSectorOster::MuseumAnchor();
    const float RadiusSq = FMath::Square(MuseumDoorRadiusCm);

    for (TActorIterator<AOCMuseumDoubleDoor> It(&World); It; ++It)
    {
        AOCMuseumDoubleDoor* Door = *It;
        if (Door && Door->ActorHasTag(TEXT("MuseumMainDoubleDoor")) &&
            FVector::DistSquared2D(Door->GetActorLocation(), Museum) <= RadiusSq)
        {
            return;
        }
    }

    TArray<TWeakObjectPtr<AOCInteractableDoor>> PrototypeLeaves;
    AActor* MuseumOwner = nullptr;
    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor) continue;
        if (Actor->ActorHasTag(TEXT("R137_MuseumPhotoModel"))) MuseumOwner = Actor;

        AOCInteractableDoor* Door = Cast<AOCInteractableDoor>(Actor);
        if (!Door || FVector::DistSquared2D(Door->GetActorLocation(), Museum) > RadiusSq) continue;
        if (Door->ActorHasTag(TEXT("MuseumMainDoorLeft")) || Door->ActorHasTag(TEXT("MuseumMainDoorRight")))
        {
            PrototypeLeaves.Add(Door);
        }
    }

    FActorSpawnParameters Params;
    Params.Owner = MuseumOwner;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AOCMuseumDoubleDoor* FinalDoor = World.SpawnActor<AOCMuseumDoubleDoor>(
        AOCMuseumDoubleDoor::StaticClass(),
        FTransform(FRotator::ZeroRotator, Museum + FVector(0.0f, -678.0f, 70.0f)), Params);
    if (!FinalDoor)
    {
        UE_LOG(LogTemp, Warning, TEXT("R13.9 museum main door: photo door spawn failed; generic R13.8 leaves preserved."));
        return;
    }

    FinalDoor->Tags.Add(TEXT("R138_MuseumInteractive"));
    FinalDoor->Tags.Add(TEXT("R139_MuseumInteractive"));
    FinalDoor->Tags.Add(TEXT("MuseumMainDoubleDoor"));

    int32 RemovedPrototypeLeaves = 0;
    for (const TWeakObjectPtr<AOCInteractableDoor>& WeakDoor : PrototypeLeaves)
    {
        if (AOCInteractableDoor* Door = WeakDoor.Get())
        {
            Door->Destroy();
            ++RemovedPrototypeLeaves;
        }
    }

    UE_LOG(LogTemp, Display,
        TEXT("R13.9 museum main door: photo-proportioned replicated double door active; generic leaves removed=%d."),
        RemovedPrototypeLeaves);
}
