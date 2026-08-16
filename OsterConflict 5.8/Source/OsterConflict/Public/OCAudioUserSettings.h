#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "OCAudioTypes.h"
#include "OCAudioUserSettings.generated.h"

/** Persistent audio controls used by S15B and later UMG settings screens. Values are stored as 0..1. */
UCLASS(Config=GameUserSettings, ConfigDoNotCheckDefaults, BlueprintType)
class OSTERCONFLICT_API UOCAudioUserSettings : public UObject
{
    GENERATED_BODY()
public:
    static UOCAudioUserSettings* Get();

    UFUNCTION(BlueprintPure, Category="Audio|Settings") float GetBusVolume(EOCAudioBus Bus) const;
    UFUNCTION(BlueprintPure, Category="Audio|Settings") int32 GetBusPercent(EOCAudioBus Bus) const;
    UFUNCTION(BlueprintCallable, Category="Audio|Settings") void SetBusPercent(EOCAudioBus Bus, int32 Percent);
    UFUNCTION(BlueprintPure, Category="Audio|Settings") bool IsBusEnabled(EOCAudioBus Bus) const;
    UFUNCTION(BlueprintCallable, Category="Audio|Settings") void SetBusEnabled(EOCAudioBus Bus, bool bEnabled);
    UFUNCTION(BlueprintCallable, Category="Audio|Settings") void SaveAudioSettings();
    UFUNCTION(BlueprintCallable, Category="Audio|Settings") void ResetAudioDefaults();

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category="Audio", meta=(ClampMin="0", ClampMax="1")) float MasterVolume = 1.0f;
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category="Audio", meta=(ClampMin="0", ClampMax="1")) float WeaponsVolume = 1.0f;
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category="Audio", meta=(ClampMin="0", ClampMax="1")) float VehiclesVolume = 0.95f;
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category="Audio", meta=(ClampMin="0", ClampMax="1")) float CharactersVolume = 0.90f;
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category="Audio", meta=(ClampMin="0", ClampMax="1")) float WorldSFXVolume = 0.90f;
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category="Audio", meta=(ClampMin="0", ClampMax="1")) float AmbienceVolume = 0.72f;
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category="Audio", meta=(ClampMin="0", ClampMax="1")) float MusicVolume = 0.55f;
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category="Audio", meta=(ClampMin="0", ClampMax="1")) float UIVolume = 0.75f;
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category="Audio", meta=(ClampMin="0", ClampMax="1")) float VoiceChatVolume = 1.0f;
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category="Audio", meta=(ClampMin="0", ClampMax="1")) float DialogueVolume = 0.90f;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category="Audio|Toggles") bool bMasterEnabled = true;
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category="Audio|Toggles") bool bWeaponsEnabled = true;
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category="Audio|Toggles") bool bVehiclesEnabled = true;
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category="Audio|Toggles") bool bCharactersEnabled = true;
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category="Audio|Toggles") bool bWorldSFXEnabled = true;
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category="Audio|Toggles") bool bAmbienceEnabled = true;
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category="Audio|Toggles") bool bMusicEnabled = true;
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category="Audio|Toggles") bool bMenuMusicEnabled = true;
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category="Audio|Toggles") bool bUIAudioEnabled = true;
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category="Audio|Toggles") bool bVoiceChatEnabled = true;
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category="Audio|Toggles") bool bDialogueEnabled = true;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category="Audio|Output") EOCDynamicRange DynamicRange = EOCDynamicRange::Standard;
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category="Audio|Output") EOCAudioOutputMode OutputMode = EOCAudioOutputMode::Headphones;

private:
    float* GetMutableBusValue(EOCAudioBus Bus);
};
