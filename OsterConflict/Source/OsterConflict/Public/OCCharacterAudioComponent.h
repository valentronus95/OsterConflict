#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OCCharacterAudioComponent.generated.h"
class UOCCharacterAudioProfile;
UCLASS(ClassGroup=(Audio),meta=(BlueprintSpawnableComponent))
class OSTERCONFLICT_API UOCCharacterAudioComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UOCCharacterAudioComponent();
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime,ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction) override;
protected:
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Audio") TObjectPtr<UOCCharacterAudioProfile> AudioProfile;
    UFUNCTION() void OnHealthChanged(float NewHealth,float Delta);
    UFUNCTION() void OnDowned();
    UFUNCTION() void OnRevived();
    UFUNCTION() void OnDeath();
private:
    float StepTimer=0.0f;
    int32 EventSeed=0;
    void PlayFootstep();
    void PlayVariant(const TArray<TObjectPtr<class USoundBase>>& Set,float Volume=1.0f);
};
