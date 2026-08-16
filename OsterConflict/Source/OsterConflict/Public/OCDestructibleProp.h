#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OCTraumaTypes.h"
#include "OCDestructibleProp.generated.h"

class UStaticMeshComponent;
class UOCWorldAudioComponent;

/** Source-only destructible test prop used by S14B for bullets/explosions and local debris physics. */
UCLASS()
class OSTERCONFLICT_API AOCDestructibleProp : public AActor
{
    GENERATED_BODY()

public:
    AOCDestructibleProp();

    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION(BlueprintPure, Category="Destruction")
    bool IsDestroyed() const { return bDestroyed; }

    UFUNCTION(BlueprintPure, Category="Destruction")
    EOCImpactSurface GetImpactSurface() const { return ImpactSurface; }

    void ConfigureRuntime(EOCImpactSurface NewSurface, float NewDurability, const FVector& NewScale);
    /** Authority-only round/sandbox reset. */
    void ResetServer();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Destruction")
    TObjectPtr<UStaticMeshComponent> IntactMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Destruction|Audio")
    TObjectPtr<UOCWorldAudioComponent> WorldAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Destruction")
    EOCImpactSurface ImpactSurface = EOCImpactSurface::Wood;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Destruction", meta=(ClampMin="1.0"))
    float MaxDurability = 120.0f;

    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Destruction")
    float CurrentDurability = 120.0f;

    UPROPERTY(ReplicatedUsing=OnRep_Destroyed, VisibleInstanceOnly, BlueprintReadOnly, Category="Destruction")
    bool bDestroyed = false;

    UPROPERTY(EditDefaultsOnly, Category="Destruction", meta=(ClampMin="1", ClampMax="16"))
    int32 LocalChunkCount = 6;

    UPROPERTY(EditDefaultsOnly, Category="Destruction", meta=(ClampMin="1.0"))
    float ChunkLifetime = 10.0f;

    UFUNCTION()
    void HandleAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
        AController* InstigatedBy, AActor* DamageCauser);

    UFUNCTION()
    void OnRep_Destroyed();

private:
    bool bLocalDestroyedPresentationApplied = false;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> TransientChunks;
    void BreakServer(const FVector& ImpulseOrigin);
    void ApplyDestroyedPresentationLocal(const FVector& ImpulseOrigin);
    void ApplyIntactPresentationLocal();
};
