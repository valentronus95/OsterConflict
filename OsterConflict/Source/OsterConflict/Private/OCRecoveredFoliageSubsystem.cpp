#include "OCRecoveredFoliageSubsystem.h"

#include "Engine/World.h"

bool UOCRecoveredFoliageSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    // Retired in favor of UOCDenseGroundFoliageSubsystem. Keeping this legacy owner alive caused
    // the park and rural verges to receive a second foliage pass on top of the dense ground cover,
    // increasing overdraw and creating overlapping grass that can shimmer at distance.
    return false;
}

void UOCRecoveredFoliageSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
}

void UOCRecoveredFoliageSubsystem::Populate(UWorld& World)
{
    // Intentionally empty. Historical implementation remains recoverable from Git history.
    bPopulated = true;
}