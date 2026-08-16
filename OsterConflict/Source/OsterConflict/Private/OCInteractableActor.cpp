#include "OCInteractableActor.h"

#include "OCCharacter.h"
#include "OCHealthComponent.h"

AOCInteractableActor::AOCInteractableActor()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    SetReplicateMovement(false);
}

FString AOCInteractableActor::GetInteractionPrompt(const AOCCharacter* InteractingCharacter) const
{
    return TEXT("E  INTERACT");
}

bool AOCInteractableActor::CanInteractServer(const AOCCharacter* InteractingCharacter) const
{
    if (!HasAuthority() || !InteractingCharacter || !InteractingCharacter->GetHealthComponent() ||
        !InteractingCharacter->GetHealthComponent()->IsAlive())
    {
        return false;
    }

    return FVector::DistSquared(GetActorLocation(), InteractingCharacter->GetActorLocation()) <=
        FMath::Square(MaxInteractionDistance);
}

void AOCInteractableActor::InteractServer(AOCCharacter* InteractingCharacter)
{
}
