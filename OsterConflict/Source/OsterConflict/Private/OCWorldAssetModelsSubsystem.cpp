#include "OCWorldAssetModelsSubsystem.h"

#include "OCAssetModelDecorator.h"
#include "OCWorldSectorOster.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"

bool UOCWorldAssetModelsSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;

    const UWorld* World = Cast<UWorld>(Outer);
    if (!World) return false;

    return World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE;
}

void UOCWorldAssetModelsSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    // Dedicated servers do not need any of the visual-only imported meshes.
    if (InWorld.GetNetMode() == NM_DedicatedServer) return;

    AttachAttempts = 0;

    // UWorldSubsystem::OnWorldBeginPlay runs before actor BeginPlay. The game mode
    // creates the Oster sector from BeginPlay, and network clients can receive that
    // replicated actor later still, so start on the next tick and retry briefly.
    InWorld.GetTimerManager().SetTimerForNextTick(
        FTimerDelegate::CreateUObject(this, &UOCWorldAssetModelsSubsystem::AttachToOsterSector, &InWorld));
}

void UOCWorldAssetModelsSubsystem::AttachToOsterSector(UWorld* World)
{
    if (!World || DecoratorActor.IsValid()) return;

    AOCWorldSectorOster* OsterSector = nullptr;
    for (TActorIterator<AOCWorldSectorOster> It(World); It; ++It)
    {
        OsterSector = *It;
        break;
    }

    if (!OsterSector)
    {
        ++AttachAttempts;
        if (AttachAttempts < 40)
        {
            FTimerHandle RetryHandle;
            World->GetTimerManager().SetTimer(
                RetryHandle,
                FTimerDelegate::CreateUObject(this, &UOCWorldAssetModelsSubsystem::AttachToOsterSector, World),
                0.25f,
                false);
        }
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    SpawnParams.ObjectFlags |= RF_Transient;

    AOCAssetModelDecorator* Decorator = World->SpawnActor<AOCAssetModelDecorator>(
        OsterSector->GetActorLocation(), OsterSector->GetActorRotation(), SpawnParams);
    if (!Decorator) return;

    Decorator->SetActorScale3D(OsterSector->GetActorScale3D());
    Decorator->PopulateForSector(OsterSector);
    DecoratorActor = Decorator;
}
