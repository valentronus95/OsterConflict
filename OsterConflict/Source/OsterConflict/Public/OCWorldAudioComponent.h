#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OCAudioTypes.h"
#include "OCWorldAudioComponent.generated.h"

class UOCWorldAudioProfile;

/** Replicates semantic interaction/world events; clients pick one of several local variants. */
UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class OSTERCONFLICT_API UOCWorldAudioComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UOCWorldAudioComponent();
    UFUNCTION(BlueprintCallable, Category="Audio") void PlayEventServer(EOCWorldAudioEvent Event, FVector Location, int32 EventSeed=0);
    void PlayEventLocal(EOCWorldAudioEvent Event, const FVector& Location, int32 EventSeed=0) const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio") TObjectPtr<UOCWorldAudioProfile> AudioProfile;

    UFUNCTION(NetMulticast, Unreliable) void MulticastWorldAudio(EOCWorldAudioEvent Event, FVector_NetQuantize Location, int32 EventSeed);
};
