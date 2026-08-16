#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OCAudioTypes.h"
#include "OCTraumaTypes.h"
#include "OCWeaponAudioComponent.generated.h"

class UOCWeaponAudioProfile;
class USoundBase;

/**
 * Client-side presentation component for weapon audio. Gameplay remains server authoritative;
 * this component only selects and plays presentation assets on each listening client.
 */
UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class OSTERCONFLICT_API UOCWeaponAudioComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UOCWeaponAudioComponent();

    void SetAudioProfile(UOCWeaponAudioProfile* NewProfile);
    UOCWeaponAudioProfile* GetAudioProfile() const { return AudioProfile; }

    /** Server helper used before multicast so all clients receive the same broad acoustic classification. */
    EOCAcousticEnvironment DetectEnvironmentAt(const FVector& SourceLocation) const;

    /** Runs locally on every client after the server multicasts a confirmed shot. */
    void HandleShotLocal(const FVector& ShotOrigin, const FVector& TraceEnd, bool bSuppressed, bool bSupersonic,
        EOCAcousticEnvironment Environment, int32 EventSeed);

    void HandleStateEventLocal(EOCWeaponAudioEvent Event, const FVector& SourceLocation, int32 EventSeed);
    void HandleImpactLocal(const FVector& ImpactLocation, EOCImpactSurface Surface, int32 EventSeed);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio")
    TObjectPtr<UOCWeaponAudioProfile> AudioProfile;

private:
    USoundBase* Pick(const TArray<TObjectPtr<USoundBase>>& Sounds, int32 EventSeed) const;
    FVector GetListenerLocation(bool& bOutHasListener) const;
    bool IsLocalWeaponOwner() const;
    void PlayAt(USoundBase* Sound, const FVector& Location, float Volume = 1.0f) const;
    void Play2D(USoundBase* Sound, float Volume = 1.0f) const;
    void EmitDebugEvent(const FString& Label, const FVector& Location) const;
};
