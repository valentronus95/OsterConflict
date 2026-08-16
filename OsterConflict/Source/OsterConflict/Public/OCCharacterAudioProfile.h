#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "OCTraumaTypes.h"
#include "OCCharacterAudioProfile.generated.h"
class USoundBase;
UCLASS(BlueprintType)
class OSTERCONFLICT_API UOCCharacterAudioProfile : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Footsteps") TArray<TObjectPtr<USoundBase>> FootstepDirt;
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Footsteps") TArray<TObjectPtr<USoundBase>> FootstepMasonry;
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Footsteps") TArray<TObjectPtr<USoundBase>> FootstepWood;
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Footsteps") TArray<TObjectPtr<USoundBase>> FootstepMetal;
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Movement") TArray<TObjectPtr<USoundBase>> GearRustle;
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Voice") TArray<TObjectPtr<USoundBase>> PainLight;
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Voice") TArray<TObjectPtr<USoundBase>> PainHeavy;
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Voice") TArray<TObjectPtr<USoundBase>> Downed;
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Voice") TArray<TObjectPtr<USoundBase>> Revived;
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Voice") TArray<TObjectPtr<USoundBase>> Death;
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Cadence") float WalkStepInterval=0.46f;
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Cadence") float SprintStepInterval=0.31f;
    const TArray<TObjectPtr<USoundBase>>& GetFootstepSet(EOCImpactSurface Surface) const;
};
