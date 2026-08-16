#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OCInteractableActor.generated.h"

class AOCCharacter;

/**
 * Shared base for world objects that can be used with the Character's E interaction.
 * The Character traces the object locally for UI and repeats the decision on the server.
 * Derived actors must still validate state and distance in InteractServer().
 */
UCLASS(Abstract)
class OSTERCONFLICT_API AOCInteractableActor : public AActor
{
    GENERATED_BODY()

public:
    AOCInteractableActor();

    virtual FString GetInteractionPrompt(const AOCCharacter* InteractingCharacter) const;
    virtual bool CanInteractServer(const AOCCharacter* InteractingCharacter) const;
    virtual void InteractServer(AOCCharacter* InteractingCharacter);

    UFUNCTION(BlueprintPure, Category="Interaction")
    float GetMaxInteractionDistance() const { return MaxInteractionDistance; }

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interaction", meta=(ClampMin="50.0"))
    float MaxInteractionDistance = 350.0f;
};
