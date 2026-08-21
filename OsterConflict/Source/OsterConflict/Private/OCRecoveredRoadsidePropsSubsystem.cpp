#include "OCRecoveredRoadsidePropsSubsystem.h"

#include "Engine/World.h"

bool UOCRecoveredRoadsidePropsSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    // The only historical consumer was the unverified unfinished-building showcase at
    // (-69000, 64500). OCRecoveredEnvironmentSubsystem has retired that site, so keeping
    // this subsystem alive leaves wheelbarrows/gravel/tools floating at an ownerless location.
    return false;
}

void UOCRecoveredRoadsidePropsSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
}

void UOCRecoveredRoadsidePropsSubsystem::Populate(UWorld& World)
{
    // Intentionally retired. Historical placement remains available through Git history.
    bPopulated = true;
}