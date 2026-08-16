#include "OCAudioUserSettings.h"

UOCAudioUserSettings* UOCAudioUserSettings::Get()
{
    return GetMutableDefault<UOCAudioUserSettings>();
}

float UOCAudioUserSettings::GetBusVolume(EOCAudioBus Bus) const
{
    float Category = 1.0f;
    bool bEnabled = true;
    switch (Bus)
    {
        case EOCAudioBus::Master: Category = 1.0f; break;
        case EOCAudioBus::Weapons: Category = WeaponsVolume; bEnabled = bWeaponsEnabled; break;
        case EOCAudioBus::Vehicles: Category = VehiclesVolume; bEnabled = bVehiclesEnabled; break;
        case EOCAudioBus::Characters: Category = CharactersVolume; bEnabled = bCharactersEnabled; break;
        case EOCAudioBus::WorldSFX: Category = WorldSFXVolume; bEnabled = bWorldSFXEnabled; break;
        case EOCAudioBus::Ambience: Category = AmbienceVolume; bEnabled = bAmbienceEnabled; break;
        case EOCAudioBus::Music: Category = MusicVolume; bEnabled = bMusicEnabled; break;
        case EOCAudioBus::UI: Category = UIVolume; bEnabled = bUIAudioEnabled; break;
        case EOCAudioBus::VoiceChat: Category = VoiceChatVolume; bEnabled = bVoiceChatEnabled; break;
        case EOCAudioBus::Dialogue: Category = DialogueVolume; bEnabled = bDialogueEnabled; break;
        default: break;
    }
    return (bMasterEnabled && bEnabled) ? FMath::Clamp(MasterVolume * Category, 0.0f, 1.0f) : 0.0f;
}

int32 UOCAudioUserSettings::GetBusPercent(EOCAudioBus Bus) const
{
    if (Bus == EOCAudioBus::Master) return FMath::RoundToInt(MasterVolume * 100.0f);
    float Value = 1.0f;
    switch (Bus)
    {
        case EOCAudioBus::Master: Value = MasterVolume; break;
        case EOCAudioBus::Weapons: Value = WeaponsVolume; break;
        case EOCAudioBus::Vehicles: Value = VehiclesVolume; break;
        case EOCAudioBus::Characters: Value = CharactersVolume; break;
        case EOCAudioBus::WorldSFX: Value = WorldSFXVolume; break;
        case EOCAudioBus::Ambience: Value = AmbienceVolume; break;
        case EOCAudioBus::Music: Value = MusicVolume; break;
        case EOCAudioBus::UI: Value = UIVolume; break;
        case EOCAudioBus::VoiceChat: Value = VoiceChatVolume; break;
        case EOCAudioBus::Dialogue: Value = DialogueVolume; break;
        default: break;
    }
    return FMath::RoundToInt(FMath::Clamp(Value, 0.0f, 1.0f) * 100.0f);
}

float* UOCAudioUserSettings::GetMutableBusValue(EOCAudioBus Bus)
{
    switch (Bus)
    {
        case EOCAudioBus::Master: return &MasterVolume;
        case EOCAudioBus::Weapons: return &WeaponsVolume;
        case EOCAudioBus::Vehicles: return &VehiclesVolume;
        case EOCAudioBus::Characters: return &CharactersVolume;
        case EOCAudioBus::WorldSFX: return &WorldSFXVolume;
        case EOCAudioBus::Ambience: return &AmbienceVolume;
        case EOCAudioBus::Music: return &MusicVolume;
        case EOCAudioBus::UI: return &UIVolume;
        case EOCAudioBus::VoiceChat: return &VoiceChatVolume;
        case EOCAudioBus::Dialogue: return &DialogueVolume;
        default: return nullptr;
    }
}

void UOCAudioUserSettings::SetBusPercent(EOCAudioBus Bus, int32 Percent)
{
    if (float* Value = GetMutableBusValue(Bus)) *Value = FMath::Clamp(static_cast<float>(Percent) / 100.0f, 0.0f, 1.0f);
}

bool UOCAudioUserSettings::IsBusEnabled(EOCAudioBus Bus) const
{
    switch (Bus)
    {
        case EOCAudioBus::Weapons: return bWeaponsEnabled;
        case EOCAudioBus::Vehicles: return bVehiclesEnabled;
        case EOCAudioBus::Characters: return bCharactersEnabled;
        case EOCAudioBus::WorldSFX: return bWorldSFXEnabled;
        case EOCAudioBus::Ambience: return bAmbienceEnabled;
        case EOCAudioBus::Music: return bMusicEnabled;
        case EOCAudioBus::UI: return bUIAudioEnabled;
        case EOCAudioBus::VoiceChat: return bVoiceChatEnabled;
        case EOCAudioBus::Dialogue: return bDialogueEnabled;
        case EOCAudioBus::Master: default: return bMasterEnabled;
    }
}

void UOCAudioUserSettings::SetBusEnabled(EOCAudioBus Bus, bool bEnabled)
{
    switch (Bus)
    {
        case EOCAudioBus::Master: bMasterEnabled=bEnabled; break;
        case EOCAudioBus::Weapons: bWeaponsEnabled=bEnabled; break;
        case EOCAudioBus::Vehicles: bVehiclesEnabled=bEnabled; break;
        case EOCAudioBus::Characters: bCharactersEnabled=bEnabled; break;
        case EOCAudioBus::WorldSFX: bWorldSFXEnabled=bEnabled; break;
        case EOCAudioBus::Ambience: bAmbienceEnabled=bEnabled; break;
        case EOCAudioBus::Music: bMusicEnabled=bEnabled; break;
        case EOCAudioBus::UI: bUIAudioEnabled=bEnabled; break;
        case EOCAudioBus::VoiceChat: bVoiceChatEnabled=bEnabled; break;
        case EOCAudioBus::Dialogue: bDialogueEnabled=bEnabled; break;
        default: break;
    }
}

void UOCAudioUserSettings::SaveAudioSettings()
{
    SaveConfig();
}

void UOCAudioUserSettings::ResetAudioDefaults()
{
    MasterVolume=1.0f; WeaponsVolume=1.0f; VehiclesVolume=0.95f; CharactersVolume=0.90f; WorldSFXVolume=0.90f;
    AmbienceVolume=0.72f; MusicVolume=0.55f; UIVolume=0.75f; VoiceChatVolume=1.0f; DialogueVolume=0.90f;
    bMasterEnabled=true; bWeaponsEnabled=bVehiclesEnabled=bCharactersEnabled=bWorldSFXEnabled=bAmbienceEnabled=true;
    bMusicEnabled=bMenuMusicEnabled=bUIAudioEnabled=bVoiceChatEnabled=bDialogueEnabled=true;
    DynamicRange=EOCDynamicRange::Standard; OutputMode=EOCAudioOutputMode::Headphones;
}
