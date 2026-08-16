#pragma once

#include "CoreMinimal.h"
#include "OCInteractableActor.h"
#include "OCInteractableGate.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UOCWorldAudioComponent;

UCLASS()
class OSTERCONFLICT_API AOCInteractableGate : public AOCInteractableActor
{
    GENERATED_BODY()
public:
    AOCInteractableGate();
    virtual void Tick(float DeltaSeconds) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual FString GetInteractionPrompt(const AOCCharacter* InteractingCharacter) const override;
    virtual void InteractServer(AOCCharacter* InteractingCharacter) override;
    void ResetServer();
protected:
    UPROPERTY(VisibleAnywhere) TObjectPtr<USceneComponent> SceneRoot;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> GateLeaf;
    UPROPERTY(VisibleAnywhere, Category="Gate|Audio") TObjectPtr<UOCWorldAudioComponent> WorldAudioComponent;
    UPROPERTY(ReplicatedUsing=OnRep_Open) bool bOpen = false;
    UPROPERTY(EditDefaultsOnly) float OpenYaw = 105.0f;
    UFUNCTION() void OnRep_Open();
};
