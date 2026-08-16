#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "OCWeaponTypes.h"
#include "OCWeaponDefinition.generated.h"

class UStaticMesh;
class UOCWeaponAudioProfile;

/** Editor-authored data asset used to tune a weapon without changing combat code. */
UCLASS(BlueprintType)
class OSTERCONFLICT_API UOCWeaponDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon")
    FOCWeaponTuning Tuning;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon|Presentation")
    TSoftObjectPtr<UStaticMesh> WorldMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon|Audio")
    TObjectPtr<UOCWeaponAudioProfile> AudioProfile;
};
