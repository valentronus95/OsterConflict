#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCFirstPersonWeaponPresentationSubsystem.generated.h"

class AOCCharacter;
class AOCWeaponBase;
class UAnimSequence;
class USkeletalMeshComponent;

struct FOCFirstPersonWeaponState
{
    TWeakObjectPtr<AOCWeaponBase> Weapon;
    int32 LastAmmo = INDEX_NONE;
    bool bWasReloading = false;
    bool bADSArmsPose = false;
    bool bRiflePoseApplied = false;
    bool bWeaponAnimationActive = false;
    double ReloadStartTime = 0.0;
    double WeaponAnimationResetTime = 0.0;
    float RecoilAlpha = 0.0f;
    FVector BaseWeaponLocation = FVector::ZeroVector;
    FRotator BaseWeaponRotation = FRotator::ZeroRotator;
    FVector BaseArmsLocation = FVector::ZeroVector;
    FRotator BaseArmsRotation = FRotator::ZeroRotator;
};

/**
 * Local presentation bridge for production first-person arms and weapon meshes.
 * Gameplay remains server-authoritative; this subsystem only drives camera-space pose,
 * recoil, ADS and reload presentation using imported animations when skeletons match.
 */
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
    void ApplyArmsPose(AOCCharacter& Character, FOCFirstPersonWeaponState& State, bool bADS);
    void PlayWeaponAnimation(AOCWeaponBase& Weapon, UAnimSequence* Sequence,
        FOCFirstPersonWeaponState& State, double ResetDelaySeconds);
    USkeletalMeshComponent* FindProductionWeaponVisual(AOCWeaponBase& Weapon) const;

    UPROPERTY(Transient) TObjectPtr<UAnimSequence> RifleIdleAnimation;
    UPROPERTY(Transient) TObjectPtr<UAnimSequence> RifleADSIdleAnimation;
    UPROPERTY(Transient) TObjectPtr<UAnimSequence> AKFireAnimation;
    UPROPERTY(Transient) TObjectPtr<UAnimSequence> AKReloadAnimation;

    TMap<TWeakObjectPtr<AOCCharacter>, FOCFirstPersonWeaponState> StateByCharacter;
};
