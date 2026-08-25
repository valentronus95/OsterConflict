#include "OCWorldAssetModelsSubsystem.h"

#include "OCAssetModelDecorator.h"
#include "OCWorldSectorOster.h"

#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"

namespace
{
    void HideLegacyVisualProxies(AOCWorldSectorOster& Sector)
    {
        TArray<UActorComponent*> Components;
        Sector.GetComponents(Components);

        for (UActorComponent* Component : Components)
        {
            UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Component);
            if (!Primitive) continue;

            const FName Name = Primitive->GetFName();
            const bool bLegacyVisualProxy =
                Name == TEXT("Buildings") ||
                Name == TEXT("ResidentialRoofs") ||
                Name == TEXT("ResidentialDetails") ||
                Name == TEXT("GrassMown") ||
                Name == TEXT("GrassRough") ||
                Name == TEXT("GrassWetland");

            if (!bLegacyVisualProxy) continue;

            Primitive->SetVisibility(false, true);
            Primitive->SetHiddenInGame(true, true);
        }
    }
}

bool UOCWorldAssetModelsSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    // PASS45 runtime evidence rejected the imported generic village presentation: AdvancedVillagePack
    // houses/fences and the companion rural shed/tower-like silhouettes do not establish Oster fidelity.
    // This subsystem is the runtime owner that spawns AOCAssetModelDecorator and then hides the semantic
    // world-sector proxies underneath it. Disable the owner as one unit so rejected generic visuals cannot
    // re-enter runtime through a different placement path. The semantic AOCWorldSectorOster geometry stays
    // visible and authoritative until evidence-backed replacement models exist.
    return false;
}

void UOCWorldAssetModelsSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    if (InWorld.GetNetMode() == NM_DedicatedServer) return;

    AttachAttempts = 0;
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
    HideLegacyVisualProxies(*OsterSector);
    DecoratorActor = Decorator;

    UE_LOG(LogTemp, Display,
        TEXT("Oster imported world models own presentation; legacy Buildings/Grass cube proxies hidden while building collision remains active."));
}
