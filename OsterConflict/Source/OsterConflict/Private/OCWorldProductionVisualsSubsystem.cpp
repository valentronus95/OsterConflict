#include "OCWorldProductionVisualsSubsystem.h"

#include "Engine/World.h"

bool UOCWorldProductionVisualsSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    // 2026-08-25 factual UE runtime rejected this B2 layer:
    // - large world/ground areas rendered black after material replacement;
    // - generic AdvancedVillagePack houses/fences did not match Oster references;
    // - source semantic visuals were hidden behind an unaccepted visual owner.
    //
    // Keep the subsystem class only as historical/source compatibility. It must not mutate the normal
    // runtime again until a reference-faithful visual layer has its own factual UE acceptance evidence.
    // A readable semantic/blockout baseline is preferable to black or falsely-authentic production art.
    (void)Outer;
    return false;
}

void UOCWorldProductionVisualsSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    UE_LOG(LogTemp, Error,
        TEXT("PASS45_B2_PRODUCTION_VISUALS_RUNTIME_REJECTED disabled=1 reason=black_world_and_non_oster_generic_assets"));
}

void UOCWorldProductionVisualsSubsystem::Deinitialize()
{
    RuntimeWorld.Reset();
    Attempts = 0;
    bBuilt = false;
    Super::Deinitialize();
}

void UOCWorldProductionVisualsSubsystem::TryBuildProductionVisuals()
{
    // Deliberately inert. Do not resurrect the rejected material/house/fence conversion here.
}
