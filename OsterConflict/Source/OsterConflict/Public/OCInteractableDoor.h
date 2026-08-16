#pragma once

#include "CoreMinimal.h"
#include "OCInteractableActor.h"
#include "OCInteractableDoor.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UOCWorldAudioComponent;

/** Replicated hinged door used by the first enterable S08 house. */
UCLASS()
class OSTERCONFLICT_API AOCInteractableDoor : public AOCInteractableActor
{
    GENERATED_BODY()

public:
    AOCInteractableDoor();

    virtual void Tick(float DeltaSeconds) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual FString GetInteractionPrompt(const AOCCharacter* InteractingCharacter) const override;
    virtual bool CanInteractServer(const AOCCharacter* InteractingCharacter) const override;
    virtual void InteractServer(AOCCharacter* InteractingCharacter) override;

    UFUNCTION(BlueprintPure, Category="Door")
    bool IsOpen() const { return bOpen; }
    void ResetServer();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Door")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Door")
    TObjectPtr<UStaticMeshComponent> FrameLeft;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Door")
    TObjectPtr<UStaticMeshComponent> FrameRight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Door")
    TObjectPtr<UStaticMeshComponent> FrameTop;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Door")
    TObjectPtr<UStaticMeshComponent> DoorLeaf;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Door")
    TObjectPtr<UStaticMeshComponent> Handle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Door|Audio")
    TObjectPtr<UOCWorldAudioComponent> WorldAudioComponent;

    UPROPERTY(EditDefaultsOnly, Category="Door")
    float DoorWidthCm = 130.0f;

    UPROPERTY(EditDefaultsOnly, Category="Door")
    float DoorHeightCm = 235.0f;

    UPROPERTY(EditDefaultsOnly, Category="Door")
    float OpenYawDegrees = 95.0f;

    UPROPERTY(EditDefaultsOnly, Category="Door", meta=(ClampMin="0.1"))
    float DoorInterpSpeed = 8.0f;

    UPROPERTY(ReplicatedUsing=OnRep_Open, VisibleInstanceOnly, BlueprintReadOnly, Category="Door")
    bool bOpen = false;

    UFUNCTION()
    void OnRep_Open();
};
