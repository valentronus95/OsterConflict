#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "OCCharacterAnimInstance.generated.h"

/**
 * C++ animation parameter bridge. Final S16C Animation Blueprints read these values for state machines,
 * blend spaces, aim offsets and montages. It contains no gameplay authority.
 */
UCLASS(Transient, Blueprintable)
class OSTERCONFLICT_API UOCCharacterAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    UPROPERTY(BlueprintReadOnly, Category="Locomotion") float Speed2D = 0.0f;
    UPROPERTY(BlueprintReadOnly, Category="Locomotion") float DirectionDegrees = 0.0f;
    UPROPERTY(BlueprintReadOnly, Category="Locomotion") float VerticalSpeed = 0.0f;
    UPROPERTY(BlueprintReadOnly, Category="Locomotion") bool bInAir = false;
    UPROPERTY(BlueprintReadOnly, Category="Locomotion") bool bCrouched = false;
    UPROPERTY(BlueprintReadOnly, Category="Locomotion") bool bSprinting = false;

    UPROPERTY(BlueprintReadOnly, Category="Combat") bool bAiming = false;
    UPROPERTY(BlueprintReadOnly, Category="Combat") bool bReloading = false;
    UPROPERTY(BlueprintReadOnly, Category="Combat") bool bHasWeapon = false;

    UPROPERTY(BlueprintReadOnly, Category="Life") bool bDowned = false;
    UPROPERTY(BlueprintReadOnly, Category="Life") bool bDead = false;
    UPROPERTY(BlueprintReadOnly, Category="Life") bool bReviving = false;

    UPROPERTY(BlueprintReadOnly, Category="Vehicle") bool bInVehicle = false;
    UPROPERTY(BlueprintReadOnly, Category="Vehicle") bool bVehicleGunner = false;

    UPROPERTY(BlueprintReadOnly, Category="Aim") float AimPitch = 0.0f;
    UPROPERTY(BlueprintReadOnly, Category="Aim") float AimYaw = 0.0f;

private:
    TWeakObjectPtr<class AOCCharacter> CachedCharacter;
};
