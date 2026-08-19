#include "OCWorldAssetModelsSubsystem.h"

#include "OCAssetModelDecorator.h"
#include "OCWorldSectorOster.h"

#include "Engine/World.h"
#include "EngineUtils.h"

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

    AOCWorldSectorOster* OsterSector = nullptr;
    for (TActorIterator<AOCWorldSectorOster> It(&InWorld); It; ++It)
    {
        OsterSector = *It;
        break;
    }
    if (!OsterSector) return;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    SpawnParams.ObjectFlags |= RF_Transient;

    AOCAssetModelDecorator* Decorator = InWorld.SpawnActor<AOCAssetModelDecorator>(
        OsterSector->GetActorLocation(), OsterSector->GetActorRotation(), SpawnParams);
    if (!Decorator) return;

    Decorator->SetActorScale3D(OsterSector->GetActorScale3D());
    Decorator->PopulateForSector(OsterSector);
    DecoratorActor = Decorator;
}
