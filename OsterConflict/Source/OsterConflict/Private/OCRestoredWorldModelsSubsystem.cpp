#include "OCRestoredWorldModelsSubsystem.h"

#include "OCRestoredWorldModelsActor.h"
#include "OCWorldSectorOster.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"

bool UOCRestoredWorldModelsSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCRestoredWorldModelsSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (InWorld.GetNetMode() == NM_DedicatedServer) return;

    AttachAttempts = 0;
    InWorld.GetTimerManager().SetTimerForNextTick(
        FTimerDelegate::CreateUObject(this, &UOCRestoredWorldModelsSubsystem::TryAttach, &InWorld));
}

void UOCRestoredWorldModelsSubsystem::TryAttach(UWorld* World)
{
    if (!World || SpawnedLayer.IsValid()) return;

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
                FTimerDelegate::CreateUObject(this, &UOCRestoredWorldModelsSubsystem::TryAttach, World),
                0.25f,
                false);
        }
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    SpawnParams.ObjectFlags |= RF_Transient;

    AOCRestoredWorldModelsActor* Layer = World->SpawnActor<AOCRestoredWorldModelsActor>(
        OsterSector->GetActorLocation(), OsterSector->GetActorRotation(), SpawnParams);
    if (!Layer) return;

    Layer->SetActorScale3D(OsterSector->GetActorScale3D());
    Layer->Populate();
    SpawnedLayer = Layer;
}
