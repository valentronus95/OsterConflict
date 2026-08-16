#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "OCAudioTypes.h"
#include "OCMenuAudioProfile.generated.h"
class USoundBase;
UCLASS(BlueprintType)
class OSTERCONFLICT_API UOCMenuAudioProfile : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Music") TObjectPtr<USoundBase> MenuMusic;
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="UI") TArray<TObjectPtr<USoundBase>> Hover;
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="UI") TArray<TObjectPtr<USoundBase>> Click;
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="UI") TArray<TObjectPtr<USoundBase>> Back;
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="UI") TArray<TObjectPtr<USoundBase>> Confirm;
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="UI") TArray<TObjectPtr<USoundBase>> Error;
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="UI") TArray<TObjectPtr<USoundBase>> OpenPanel;
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="UI") TArray<TObjectPtr<USoundBase>> ClosePanel;
    const TArray<TObjectPtr<USoundBase>>& GetSet(EOCMenuAudioEvent Event) const;
};
