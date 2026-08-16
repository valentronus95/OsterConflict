#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OCAmbientAudioZone.generated.h"

class UAudioComponent;
class UBoxComponent;
class UOCWorldAudioProfile;

/** Client-local ambient zone. It intentionally avoids network RPCs for birds/wind/distant life. */
UCLASS()
class OSTERCONFLICT_API AOCAmbientAudioZone : public AActor
{
    GENERATED_BODY()
public:
    AOCAmbientAudioZone();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    void ConfigureRuntime(const FVector& ExtentCm, float MinInterval, float MaxInterval, float RadiusCm);
protected:
    UPROPERTY(VisibleAnywhere) TObjectPtr<UBoxComponent> Zone;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio") TObjectPtr<UOCWorldAudioProfile> AudioProfile;
    UPROPERTY(EditAnywhere, Category="Audio", meta=(ClampMin="1.0")) float OneShotMinInterval=5.0f;
    UPROPERTY(EditAnywhere, Category="Audio", meta=(ClampMin="1.0")) float OneShotMaxInterval=14.0f;
    UPROPERTY(EditAnywhere, Category="Audio", meta=(ClampMin="200.0")) float OneShotRadius=1600.0f;
private:
    UPROPERTY(Transient) TObjectPtr<UAudioComponent> BedComponent;
    float NextOneShotTime=0.0f;
    bool bListenerInside=false;
    void UpdateListenerState();
    void PlayRandomOneShot();
};
