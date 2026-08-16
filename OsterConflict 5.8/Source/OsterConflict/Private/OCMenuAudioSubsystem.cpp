#include "OCMenuAudioSubsystem.h"
#include "Sound/SoundBase.h"
#include "OCAudioUserSettings.h"
#include "OCMenuAudioProfile.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
void UOCMenuAudioSubsystem::SetProfile(UOCMenuAudioProfile* NewProfile){Profile=NewProfile;}
void UOCMenuAudioSubsystem::StartMenuMusic(){if(!Profile||!Profile->MenuMusic)return;UOCAudioUserSettings* S=UOCAudioUserSettings::Get();if(!S->bMusicEnabled||!S->bMenuMusicEnabled)return;if(MusicComponent)MusicComponent->Stop();MusicComponent=UGameplayStatics::SpawnSound2D(this,Profile->MenuMusic,S->GetBusVolume(EOCAudioBus::Music),1.0f,0.0f,nullptr,true,false);}
void UOCMenuAudioSubsystem::StopMenuMusic(float FadeSeconds){if(MusicComponent){MusicComponent->FadeOut(FMath::Max(0.0f,FadeSeconds),0.0f);MusicComponent=nullptr;}}
void UOCMenuAudioSubsystem::RefreshMenuMusicVolume(){if(MusicComponent)MusicComponent->SetVolumeMultiplier(UOCAudioUserSettings::Get()->GetBusVolume(EOCAudioBus::Music));}
void UOCMenuAudioSubsystem::PlayUIEvent(EOCMenuAudioEvent Event,int32 Seed){if(!Profile)return;const TArray<TObjectPtr<USoundBase>>& Set=Profile->GetSet(Event);if(Set.IsEmpty())return;const float V=UOCAudioUserSettings::Get()->GetBusVolume(EOCAudioBus::UI);if(V<=0.0f)return;UGameplayStatics::PlaySound2D(this,Set[FMath::Abs(Seed==0?FMath::Rand():Seed)%Set.Num()],V);}
