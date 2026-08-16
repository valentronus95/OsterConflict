#pragma once

#include "CoreMinimal.h"
#include "OCInteractableActor.h"
#include "OCInteractableLight.generated.h"

class USceneComponent;
class UPointLightComponent;
class UStaticMeshComponent;
class UOCWorldAudioComponent;

UCLASS()
class OSTERCONFLICT_API AOCInteractableLight : public AOCInteractableActor
{
    GENERATED_BODY()
public:
    AOCInteractableLight();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual FString GetInteractionPrompt(const AOCCharacter* InteractingCharacter) const override;
    virtual void InteractServer(AOCCharacter* InteractingCharacter) override;
    void ResetServer();

protected:
    UPROPERTY(VisibleAnywhere) TObjectPtr<USceneComponent> SceneRoot;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Fixture;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UPointLightComponent> Light;
    UPROPERTY(VisibleAnywhere, Category="Light|Audio") TObjectPtr<UOCWorldAudioComponent> WorldAudioComponent;
    UPROPERTY(ReplicatedUsing=OnRep_LightState) bool bLightOn = false;
    UFUNCTION() void OnRep_LightState();
    void ApplyState();
};
