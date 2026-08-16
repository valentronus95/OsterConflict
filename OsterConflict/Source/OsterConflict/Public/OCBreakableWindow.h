#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/NetSerialization.h"
#include "OCBreakableWindow.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UOCWorldAudioComponent;

/**
 * S08 prototype breakable window.
 * Gameplay state (intact/broken collision) is server authoritative and replicated.
 * Flying fragments are short-lived cosmetic physics so gameplay never depends on shard simulation.
 */
UCLASS()
class OSTERCONFLICT_API AOCBreakableWindow : public AActor
{
    GENERATED_BODY()

public:
    AOCBreakableWindow();

    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION(BlueprintPure, Category="Window")
    bool IsBroken() const { return bBroken; }

    /** Authority-only round/sandbox reset. */
    void ResetServer();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Window")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Window")
    TObjectPtr<UStaticMeshComponent> GlassPane;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Window")
    TObjectPtr<UStaticMeshComponent> FrameLeft;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Window")
    TObjectPtr<UStaticMeshComponent> FrameRight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Window")
    TObjectPtr<UStaticMeshComponent> FrameTop;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Window")
    TObjectPtr<UStaticMeshComponent> FrameBottom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Window|Audio")
    TObjectPtr<UOCWorldAudioComponent> WorldAudioComponent;

    UPROPERTY()
    TArray<TObjectPtr<UStaticMeshComponent>> DebrisPieces;

    UPROPERTY(EditDefaultsOnly, Category="Window", meta=(ClampMin="1.0"))
    float BreakDamageThreshold = 8.0f;

    UPROPERTY(EditDefaultsOnly, Category="Window", meta=(ClampMin="0.1"))
    float DebrisLifetime = 4.0f;

    UPROPERTY(ReplicatedUsing=OnRep_Broken, VisibleInstanceOnly, BlueprintReadOnly, Category="Window")
    bool bBroken = false;

    UFUNCTION()
    void OnRep_Broken();

    UFUNCTION(NetMulticast, Unreliable)
    void MulticastBreakFX(FVector_NetQuantizeNormal ImpulseDirection);

private:
    FTimerHandle HideDebrisTimerHandle;

    void BreakServer(const FVector& ImpulseDirection);
    void ApplyBrokenPresentation();
    void ApplyIntactPresentation();
    void HideDebris();
};
