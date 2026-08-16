#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OCAudioTypes.h"
#include "OCMenuAudioSubsystem.generated.h"
class UAudioComponent; class UOCMenuAudioProfile;
UCLASS()
class OSTERCONFLICT_API UOCMenuAudioSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable,Category="Audio|Menu") void SetProfile(UOCMenuAudioProfile* NewProfile);
    UFUNCTION(BlueprintCallable,Category="Audio|Menu") void StartMenuMusic();
    UFUNCTION(BlueprintCallable,Category="Audio|Menu") void StopMenuMusic(float FadeSeconds=0.35f);
    UFUNCTION(BlueprintCallable,Category="Audio|Menu") void RefreshMenuMusicVolume();
    UFUNCTION(BlueprintCallable,Category="Audio|Menu") void PlayUIEvent(EOCMenuAudioEvent Event, int32 Seed=0);
private:
    UPROPERTY(Transient) TObjectPtr<UOCMenuAudioProfile> Profile;
    UPROPERTY(Transient) TObjectPtr<UAudioComponent> MusicComponent;
};
