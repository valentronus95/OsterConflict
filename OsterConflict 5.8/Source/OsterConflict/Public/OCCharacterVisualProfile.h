#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "OCCharacterVisualTypes.h"
#include "OCCharacterVisualProfile.generated.h"

class UAnimInstance;
class UAnimMontage;
class USkeletalMesh;
class UStaticMesh;

/**
 * Data-driven art profile for a faction. S16C ships the schema and source-only proxy fallback;
 * final meshes/animations are imported later without changing gameplay/network code.
 */
UCLASS(BlueprintType)
class OSTERCONFLICT_API UOCCharacterVisualProfile : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Identity") EOCFactionArchetype Faction = EOCFactionArchetype::UASpecialUnit;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Identity") FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Body") TSoftObjectPtr<USkeletalMesh> ThirdPersonBodyMesh;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Body") TSoftObjectPtr<USkeletalMesh> FirstPersonArmsMesh;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Body") TSubclassOf<UAnimInstance> ThirdPersonAnimClass;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Body") TSubclassOf<UAnimInstance> FirstPersonAnimClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Gear") TArray<TSoftObjectPtr<UStaticMesh>> HelmetMeshes;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Gear") TArray<TSoftObjectPtr<UStaticMesh>> VestMeshes;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Gear") TArray<TSoftObjectPtr<UStaticMesh>> BackpackMeshes;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Gear") FName HelmetSocket = TEXT("head");
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Gear") FName VestSocket = TEXT("spine_03");
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Gear") FName BackpackSocket = TEXT("spine_02");

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Montage") TSoftObjectPtr<UAnimMontage> FireMontage;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Montage") TSoftObjectPtr<UAnimMontage> ReloadMontage;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Montage") TSoftObjectPtr<UAnimMontage> ReviveMontage;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Montage") TSoftObjectPtr<UAnimMontage> DownedMontage;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Montage") TSoftObjectPtr<UAnimMontage> DeathMontage;

    /** Source-only visual fallback/debug tint. Final materials belong to the imported character assets. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Debug") FLinearColor DebugFactionColor = FLinearColor(0.26f, 0.34f, 0.24f, 1.0f);
};
