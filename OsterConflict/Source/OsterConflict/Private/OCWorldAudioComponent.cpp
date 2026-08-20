#include "OCWorldAudioComponent.h"
#include "OCAudioUserSettings.h"
#include "OCWorldAudioProfile.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

namespace
{
    USoundBase* LoadR13WorldFallback(EOCWorldAudioEvent Event)
    {
        const TCHAR* AssetName = nullptr;
        switch (Event)
        {
            case EOCWorldAudioEvent::ExplosionSmall:
            case EOCWorldAudioEvent::ExplosionLarge:
                AssetName = TEXT("dull_explosion");
                break;
            case EOCWorldAudioEvent::WindowBreak:
            case EOCWorldAudioEvent::DestructionWood:
            case EOCWorldAudioEvent::DestructionMetal:
            case EOCWorldAudioEvent::DestructionMasonry:
            case EOCWorldAudioEvent::Debris:
                AssetName = TEXT("snd_bullethit");
                break;
            default:
                break;
        }
        if (!AssetName) return nullptr;
        const FString Path = FString::Printf(TEXT("/Game/R13/Audio/%s.%s"), AssetName, AssetName);
        return LoadObject<USoundBase>(nullptr, *Path);
    }
}

UOCWorldAudioComponent::UOCWorldAudioComponent()
{
    PrimaryComponentTick.bCanEverTick=false;
    SetIsReplicatedByDefault(true);
}

void UOCWorldAudioComponent::PlayEventServer(EOCWorldAudioEvent Event, FVector Location, int32 EventSeed)
{
    if (!GetOwner() || !GetOwner()->HasAuthority()) return;
    MulticastWorldAudio(Event, Location, EventSeed == 0 ? FMath::Rand() : EventSeed);
}

void UOCWorldAudioComponent::MulticastWorldAudio_Implementation(EOCWorldAudioEvent Event, FVector_NetQuantize Location, int32 EventSeed)
{
    PlayEventLocal(Event, Location, EventSeed);
}

void UOCWorldAudioComponent::PlayEventLocal(EOCWorldAudioEvent Event, const FVector& Location, int32 EventSeed) const
{
    if (!GetWorld() || GetWorld()->GetNetMode()==NM_DedicatedServer) return;

    // R13 content bridge: the semantic audio system existed long before the project had assigned profile assets.
    // Keep authored profiles as the preferred path, but make explosions/destruction audible immediately after option 8 import.
    if (!AudioProfile)
    {
        USoundBase* Sound = LoadR13WorldFallback(Event);
        if (!Sound) return;
        const float Bus = UOCAudioUserSettings::Get()->GetBusVolume(EOCAudioBus::WorldSFX);
        if (Bus <= 0.0f) return;
        const float Volume = (Event == EOCWorldAudioEvent::ExplosionLarge) ? 1.0f : 0.82f;
        UGameplayStatics::PlaySoundAtLocation(this, Sound, Location, FMath::Clamp(Bus * Volume, 0.0f, 1.25f));
        return;
    }

    const TArray<TObjectPtr<USoundBase>>& Set=AudioProfile->GetEventSet(Event);
    if (Set.IsEmpty()) return;
    const int32 Index=FMath::Abs(EventSeed)%Set.Num();
    USoundBase* Sound=Set[Index].Get();
    if (!Sound) return;
    FRandomStream R(EventSeed);
    const float VolJitter=1.0f+R.FRandRange(-AudioProfile->VolumeVariation,AudioProfile->VolumeVariation);
    const float Pitch=1.0f+R.FRandRange(-AudioProfile->PitchVariation,AudioProfile->PitchVariation);
    const float Bus=UOCAudioUserSettings::Get()->GetBusVolume(EOCAudioBus::WorldSFX);
    if (Bus<=0.0f) return;
    UGameplayStatics::PlaySoundAtLocation(this,Sound,Location,FMath::Clamp(Bus*VolJitter,0.0f,1.25f),Pitch);
}
