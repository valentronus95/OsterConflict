#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OCVehicleAudioComponent.generated.h"
class UAudioComponent;class UOCVehicleAudioProfile;
UCLASS(ClassGroup=(Audio),meta=(BlueprintSpawnableComponent))
class OSTERCONFLICT_API UOCVehicleAudioComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UOCVehicleAudioComponent();
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime,ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction) override;
    void PlayCollisionServer(float ImpactStrength,const FVector& Location);
    void SetAudioProfile(UOCVehicleAudioProfile* InProfile);
protected:
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Audio") TObjectPtr<UOCVehicleAudioProfile> AudioProfile;
    UFUNCTION(NetMulticast,Unreliable) void MulticastCollisionAudio(bool bHeavy,FVector_NetQuantize Location,int32 Seed);
private:
    UPROPERTY(Transient) TObjectPtr<UAudioComponent> ExteriorEngine;
    UPROPERTY(Transient) TObjectPtr<UAudioComponent> InteriorEngine;
    UPROPERTY(Transient) TObjectPtr<UAudioComponent> TireLoop;
    UPROPERTY(Transient) TObjectPtr<UAudioComponent> SkidLoop;
    bool bHadDriver=false;
    int32 EventSeed=0;
    void EnsureLoops();
    void PlayOneShot(const TArray<TObjectPtr<class USoundBase>>& Set,const FVector& Location,float Volume=1.0f);
};