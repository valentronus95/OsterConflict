#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "OCVehicleAudioProfile.generated.h"
class USoundBase;
UCLASS(BlueprintType)
class OSTERCONFLICT_API UOCVehicleAudioProfile : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Engine") TObjectPtr<USoundBase> EngineExteriorLoop;
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Engine") TObjectPtr<USoundBase> EngineInteriorLoop;
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Movement") TObjectPtr<USoundBase> TireRollLoop;
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Movement") TObjectPtr<USoundBase> SkidLoop;
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Events") TArray<TObjectPtr<USoundBase>> EngineStart;
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Events") TArray<TObjectPtr<USoundBase>> EngineStop;
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Events") TArray<TObjectPtr<USoundBase>> CollisionLight;
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Events") TArray<TObjectPtr<USoundBase>> CollisionHeavy;
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Tuning") float IdlePitch=0.78f;
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Tuning") float MaxPitch=1.55f;
};
