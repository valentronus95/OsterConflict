#include "OCCharacterAnimInstance.h"

#include "OCCharacter.h"
#include "OCHealthComponent.h"
#include "OCWeaponBase.h"
#include "GameFramework/CharacterMovementComponent.h"

void UOCCharacterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    CachedCharacter = Cast<AOCCharacter>(TryGetPawnOwner());
}

void UOCCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);
    AOCCharacter* Character = CachedCharacter.Get();
    if (!Character)
    {
        Character = Cast<AOCCharacter>(TryGetPawnOwner());
        CachedCharacter = Character;
    }
    if (!Character) return;

    const FVector Velocity = Character->GetVelocity();
    Speed2D = Velocity.Size2D();
    VerticalSpeed = Velocity.Z;
    const FVector Forward = Character->GetActorForwardVector().GetSafeNormal2D();
    const FVector Right = Character->GetActorRightVector().GetSafeNormal2D();
    const FVector MoveDir = Velocity.GetSafeNormal2D();
    if (MoveDir.IsNearlyZero())
    {
        DirectionDegrees = 0.0f;
    }
    else
    {
        const float ForwardDot = FVector::DotProduct(Forward, MoveDir);
        const float RightDot = FVector::DotProduct(Right, MoveDir);
        DirectionDegrees = FMath::RadiansToDegrees(FMath::Atan2(RightDot, ForwardDot));
    }

    const UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
    bInAir = Movement ? Movement->IsFalling() : false;
    bCrouched = Character->IsCrouchedOC();
    bSprinting = Character->IsSprinting();
    bAiming = Character->IsAiming();
    bInVehicle = Character->IsInVehicle();
    bVehicleGunner = Character->IsVehicleGunner();
    bReviving = Character->GetReviveProgress() > 0.0f;

    AOCWeaponBase* Weapon = Character->GetCurrentWeapon();
    bHasWeapon = Weapon != nullptr;
    bReloading = Weapon && Weapon->IsReloading();

    const UOCHealthComponent* Health = Character->GetHealthComponent();
    bDowned = Health && Health->IsDowned();
    bDead = Health && Health->IsDead();

    if (const AController* Controller = Character->GetController())
    {
        const FRotator Delta = (Controller->GetControlRotation() - Character->GetActorRotation()).GetNormalized();
        AimPitch = FMath::ClampAngle(Delta.Pitch, -89.0f, 89.0f);
        AimYaw = FMath::ClampAngle(Delta.Yaw, -120.0f, 120.0f);
    }
}
