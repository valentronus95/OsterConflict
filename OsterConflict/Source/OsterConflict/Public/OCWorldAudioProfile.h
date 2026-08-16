#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "OCAudioTypes.h"
#include "OCWorldAudioProfile.generated.h"

class USoundBase;

/** Variant banks for interactions, destruction and restrained natural ambience. */
UCLASS(BlueprintType)
class OSTERCONFLICT_API UOCWorldAudioProfile : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction") TArray<TObjectPtr<USoundBase>> InteractionGeneric;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction") TArray<TObjectPtr<USoundBase>> DoorOpen;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction") TArray<TObjectPtr<USoundBase>> DoorClose;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction") TArray<TObjectPtr<USoundBase>> GateOpen;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction") TArray<TObjectPtr<USoundBase>> GateClose;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction") TArray<TObjectPtr<USoundBase>> LightOn;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction") TArray<TObjectPtr<USoundBase>> LightOff;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="World") TArray<TObjectPtr<USoundBase>> WindowBreak;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="World") TArray<TObjectPtr<USoundBase>> DestructionWood;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="World") TArray<TObjectPtr<USoundBase>> DestructionMetal;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="World") TArray<TObjectPtr<USoundBase>> DestructionMasonry;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="World") TArray<TObjectPtr<USoundBase>> ExplosionSmall;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="World") TArray<TObjectPtr<USoundBase>> ExplosionLarge;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="World") TArray<TObjectPtr<USoundBase>> Debris;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ambience") TObjectPtr<USoundBase> AmbientBed;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ambience") TArray<TObjectPtr<USoundBase>> Birds;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ambience") TArray<TObjectPtr<USoundBase>> WindLeaves;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ambience") TArray<TObjectPtr<USoundBase>> YardAnimals;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ambience") TArray<TObjectPtr<USoundBase>> DistantDogs;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ambience") TArray<TObjectPtr<USoundBase>> DistantTraffic;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ambience") TArray<TObjectPtr<USoundBase>> Water;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Variation", meta=(ClampMin="0.0", ClampMax="0.25")) float PitchVariation = 0.035f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Variation", meta=(ClampMin="0.0", ClampMax="0.40")) float VolumeVariation = 0.08f;

    const TArray<TObjectPtr<USoundBase>>& GetEventSet(EOCWorldAudioEvent Event) const;
};
