#include "OCWorldAudioComponent.h"
#include "OCAudioUserSettings.h"
#include "OCWorldAudioProfile.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

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
    if (!AudioProfile || !GetWorld() || GetWorld()->GetNetMode()==NM_DedicatedServer) return;
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
