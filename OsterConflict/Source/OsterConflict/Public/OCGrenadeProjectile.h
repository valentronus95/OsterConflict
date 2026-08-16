#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OCOrdnanceTypes.h"
#include "OCGrenadeProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UStaticMeshComponent;
class UOCWorldAudioComponent;

UCLASS()
class OSTERCONFLICT_API AOCGrenadeProjectile : public AActor
{
    GENERATED_BODY()
public:
    AOCGrenadeProjectile();
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    void InitializeGrenadeServer(EOCGrenadeType NewType, const FVector& InitialVelocity);

protected:
    UPROPERTY(VisibleAnywhere) TObjectPtr<USphereComponent> Collision;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> GrenadeMesh;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UProjectileMovementComponent> ProjectileMovement;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Grenade|Audio") TObjectPtr<UOCWorldAudioComponent> WorldAudioComponent;

    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Grenade")
    EOCGrenadeType GrenadeType = EOCGrenadeType::Fragmentation;

    UPROPERTY(EditDefaultsOnly, Category="Grenade") float FuseSeconds = 3.25f;
    UPROPERTY(EditDefaultsOnly, Category="Grenade") float FragBaseDamage = 155.0f;
    UPROPERTY(EditDefaultsOnly, Category="Grenade") float FragMinDamage = 12.0f;
    UPROPERTY(EditDefaultsOnly, Category="Grenade") float FragInnerRadius = 180.0f;
    UPROPERTY(EditDefaultsOnly, Category="Grenade") float FragOuterRadius = 760.0f;
    UPROPERTY(EditDefaultsOnly, Category="Grenade|Physics") float FragPhysicsImpulse = 85000.0f;
    UPROPERTY(EditDefaultsOnly, Category="Grenade|Physics") float MaxImpulseBodyMassKg = 250.0f;
    UPROPERTY(EditDefaultsOnly, Category="Grenade") float FlashRadius = 1150.0f;

private:
    FTimerHandle FuseTimerHandle;
    void DetonateServer();
    void ApplyFlashServer();
    void ApplyBoundedPhysicsImpulseServer(float Radius, float Strength);
};
