#pragma once

#include "CoreMinimal.h"
#include "OCInteractableActor.h"
#include "OCMuseumDoubleDoor.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UOCWorldAudioComponent;

/** Photo-proportioned replicated two-leaf main door for the Oster museum. */
UCLASS()
class OSTERCONFLICT_API AOCMuseumDoubleDoor : public AOCInteractableActor
{
    GENERATED_BODY()

public:
    AOCMuseumDoubleDoor();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual FString GetInteractionPrompt(const AOCCharacter* InteractingCharacter) const override;
    virtual bool CanInteractServer(const AOCCharacter* InteractingCharacter) const override;
    virtual void InteractServer(AOCCharacter* InteractingCharacter) override;

    UFUNCTION(BlueprintPure, Category="Museum|Door")
    bool IsOpen() const { return bOpen; }

    void ResetServer();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Museum|Door")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Museum|Door")
    TObjectPtr<USceneComponent> LeftHinge;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Museum|Door")
    TObjectPtr<USceneComponent> RightHinge;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Museum|Door")
    TObjectPtr<UStaticMeshComponent> LeftLeaf;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Museum|Door")
    TObjectPtr<UStaticMeshComponent> RightLeaf;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Museum|Door")
    TObjectPtr<UStaticMeshComponent> FrameLeft;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Museum|Door")
    TObjectPtr<UStaticMeshComponent> FrameRight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Museum|Door")
    TObjectPtr<UStaticMeshComponent> FrameTop;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Museum|Door")
    TObjectPtr<UStaticMeshComponent> LeftHandle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Museum|Door")
    TObjectPtr<UStaticMeshComponent> RightHandle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Museum|Door|Audio")
    TObjectPtr<UOCWorldAudioComponent> WorldAudioComponent;

    UPROPERTY()
    TArray<TObjectPtr<UStaticMeshComponent>> LeftPanelDetails;

    UPROPERTY()
    TArray<TObjectPtr<UStaticMeshComponent>> RightPanelDetails;

    UPROPERTY(EditDefaultsOnly, Category="Museum|Door")
    float OpenYawDegrees = 96.0f;

    UPROPERTY(EditDefaultsOnly, Category="Museum|Door", meta=(ClampMin="0.1"))
    float DoorInterpSpeed = 7.0f;

    UPROPERTY(ReplicatedUsing=OnRep_Open, VisibleInstanceOnly, BlueprintReadOnly, Category="Museum|Door")
    bool bOpen = false;

    UFUNCTION()
    void OnRep_Open();

private:
    void ApplyPhotoMaterials();
};
