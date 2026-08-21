#include "OCRecoveredBuildingDetailsSubsystem.h"

#include "Engine/World.h"

bool UOCRecoveredBuildingDetailsSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    // The unfinished-building showcase at raw world coordinate (-69000,64500) had no
    // evidence-backed Oster site and duplicated ownership with OCRecoveredEnvironmentSubsystem.
    // Keep the class for history/compatibility, but do not instantiate it in runtime worlds.
    return false;
}

void UOCRecoveredBuildingDetailsSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
}

void UOCRecoveredBuildingDetailsSubsystem::Populate(UWorld& World)
{
    // Intentionally disabled until an evidence-backed site and single authoritative owner exist.
}
