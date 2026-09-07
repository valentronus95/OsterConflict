#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCFirstPersonWeaponPresentationSubsystem.generated.h"

class AOCCharacter;
class AOCWeaponBase;
class UAnimSequence;
class UPrimitiveComponent;
class USkeletalMeshComponent;
struct FOCFirstPersonWeaponProfile;

struct FOCFirstPersonWeaponState
{
    TWeakObjectPtr<AOCWeaponBase> Weapon;
    int32 LastAmmo = INDEX_NONE;
    bool bWasReloading = false;
    bool bWasActionCycling = false;
    bool bWasAiming = false;
    bool bADSArmsPose = false;
    bool bRiflePoseApplied = false;
    bool bWeaponAnimationActive = false;
    uint8 ArmsLocomotionState = 255;
    double ReloadStartTime = 0.0;
    double WeaponAnimationResetTime = 0.0;
    float RecoilAlpha = 0.0f;
    FVector BaseWeaponLocation = FVector::ZeroVector;
    FRotator BaseWeaponRotation = FRotator::ZeroRotator;
    // Preserve the character-owned arms transform separately from the per-weapon presentation baseline.
    // Without this split, switching weapons can repeatedly add grip offsets and slowly drift the arms in camera space.
    FVector OriginalArmsLocation = FVector::ZeroVector;
    FRotator OriginalArmsRotation = FRotator::ZeroRotator;
    FVector BaseArmsLocation = FVector::ZeroVector;
    FRotator BaseArmsRotation = FRotator::ZeroRotator;
};

UCLASS()
class OSTERCONFLICT_API UOCFirstPersonWeaponPresentationSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;

private:
    void UpdateLocalCharacter(AOCCharacter& Character, float DeltaTime);
    void RestorePresentationState(AOCCharacter& Character, FOCFirstPersonWeaponState& State);
    void ApplyArmsPose(AOCCharacter& Character, FOCFirstPersonWeaponState& State, bool bADS);
    bool PlayWeaponAnimation(AOCWeaponBase& Weapon, UAnimSequence* Sequence,
        FOCFirstPersonWeaponState& State, double ResetDelaySeconds);
    void ValidateADSAlignment(AOCCharacter& Character, AOCWeaponBase& Weapon,
        UPrimitiveComponent* ProductionVisual, const FOCFirstPersonWeaponProfile& Profile) const;

    UPrimitiveComponent* FindProductionWeaponVisual(AOCWeaponBase& Weapon) const;
    USkeletalMeshComponent* FindProductionSkeletalWeaponVisual(AOCWeaponBase& Weapon) const;

    UPROPERTY(Transient) TObjectPtr<UAnimSequence> RifleIdleAnimation;
    UPROPERTY(Transient) TObjectPtr<UAnimSequence> RifleADSIdleAnimation;
    UPROPERTY(Transient) TObjectPtr<UAnimSequence> RifleWalkForwardAnimation;
    UPROPERTY(Transient) TObjectPtr<UAnimSequence> RifleWalkBackwardAnimation;
    UPROPERTY(Transient) TObjectPtr<UAnimSequence> RifleWalkLeftAnimation;
    UPROPERTY(Transient) TObjectPtr<UAnimSequence> RifleWalkRightAnimation;
    UPROPERTY(Transient) TObjectPtr<UAnimSequence> RifleADSWalkForwardAnimation;
    UPROPERTY(Transient) TObjectPtr<UAnimSequence> RifleADSWalkBackwardAnimation;
    UPROPERTY(Transient) TObjectPtr<UAnimSequence> RifleADSWalkLeftAnimation;
    UPROPERTY(Transient) TObjectPtr<UAnimSequence> RifleADSWalkRightAnimation;
    UPROPERTY(Transient) TObjectPtr<UAnimSequence> AKFireAnimation;
    UPROPERTY(Transient) TObjectPtr<UAnimSequence> AKReloadAnimation;

    TMap<TWeakObjectPtr<AOCCharacter>, FOCFirstPersonWeaponState> StateByCharacter;
    bool bLocalPawnFastPathLogged = false;
};
