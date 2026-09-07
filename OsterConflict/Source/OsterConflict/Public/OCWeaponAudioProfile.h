#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "OCWeaponTypes.h"
#include "OCAudioTypes.h"
#include "OCTraumaTypes.h"
#include "OCWeaponAudioProfile.generated.h"

class USoundBase;

/**
 * Data-only weapon sound palette. S15A deliberately keeps copyrighted/third-party audio out of source control.
 * Final SoundWave / SoundCue / MetaSound assets can be assigned in Editor without changing combat code.
 */
UCLASS(BlueprintType)
class OSTERCONFLICT_API UOCWeaponAudioProfile : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Identity")
    FName ProfileId = FName(TEXT("DefaultWeaponAudio"));

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shot|Outdoor")
    TArray<TObjectPtr<USoundBase>> ShotNearOutdoor;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shot|Indoor")
    TArray<TObjectPtr<USoundBase>> ShotNearIndoor;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shot|Suppressed")
    TArray<TObjectPtr<USoundBase>> ShotSuppressedOutdoor;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shot|Suppressed")
    TArray<TObjectPtr<USoundBase>> ShotSuppressedIndoor;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shot|Distance")
    TArray<TObjectPtr<USoundBase>> DistantTails;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Mechanics")
    TArray<TObjectPtr<USoundBase>> MechanicalShot;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Mechanics")
    TArray<TObjectPtr<USoundBase>> ReloadStart;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Mechanics")
    TArray<TObjectPtr<USoundBase>> ReloadEnd;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Mechanics")
    TArray<TObjectPtr<USoundBase>> ReloadCancel;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Mechanics")
    TArray<TObjectPtr<USoundBase>> DryFire;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Mechanics")
    TArray<TObjectPtr<USoundBase>> FireModeSwitch;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Mechanics|Manual Action")
    TArray<TObjectPtr<USoundBase>> BoltCycle;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Mechanics|Manual Action")
    TArray<TObjectPtr<USoundBase>> PumpCycle;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Mechanics|Manual Action")
    TArray<TObjectPtr<USoundBase>> LeverCycle;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Mechanics")
    TArray<TObjectPtr<USoundBase>> Equip;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Mechanics")
    TArray<TObjectPtr<USoundBase>> Drop;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ballistics")
    TArray<TObjectPtr<USoundBase>> BulletCracks;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Impacts")
    TArray<TObjectPtr<USoundBase>> ImpactFlesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Impacts")
    TArray<TObjectPtr<USoundBase>> ImpactGlass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Impacts")
    TArray<TObjectPtr<USoundBase>> ImpactWood;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Impacts")
    TArray<TObjectPtr<USoundBase>> ImpactMetal;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Impacts")
    TArray<TObjectPtr<USoundBase>> ImpactMasonry;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Impacts")
    TArray<TObjectPtr<USoundBase>> ImpactDirt;

    /** Below this distance the local client uses a near report. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Distance", meta=(ClampMin="100.0"))
    float NearShotMaxDistanceCm = 5500.0f;

    /** Maximum distance at which this profile deliberately emits a weapon report. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Distance", meta=(ClampMin="1000.0"))
    float DistantTailMaxDistanceCm = 32000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ballistics", meta=(ClampMin="50.0"))
    float BulletCrackRadiusCm = 700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ballistics")
    bool bSupersonicProjectile = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mix", meta=(ClampMin="0.0", ClampMax="2.0"))
    float LocalMechanicalVolume = 0.85f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mix", meta=(ClampMin="0.0", ClampMax="2.0"))
    float SuppressedFallbackVolume = 0.42f;

    const TArray<TObjectPtr<USoundBase>>& GetImpactSet(EOCImpactSurface Surface) const;
};
