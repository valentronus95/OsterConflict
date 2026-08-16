#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OCAntiArmorProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UStaticMeshComponent;

UCLASS()
class OSTERCONFLICT_API AOCAntiArmorProjectile : public AActor
{
    GENERATED_BODY()
public:
    AOCAntiArmorProjectile();

protected:
    UPROPERTY(VisibleAnywhere) TObjectPtr<USphereComponent> Collision;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> ProjectileMesh;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UProjectileMovementComponent> Movement;
    UPROPERTY(EditDefaultsOnly, Category="AntiArmor") float DirectDamage = 620.0f;
    UPROPERTY(EditDefaultsOnly, Category="AntiArmor") float BlastRadius = 260.0f;

private:
    UFUNCTION()
    void HandleImpact(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
        FVector NormalImpulse, const FHitResult& Hit);
};
