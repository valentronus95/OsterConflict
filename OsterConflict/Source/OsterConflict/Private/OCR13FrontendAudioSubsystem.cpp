#include "OCR13FrontendAudioSubsystem.h"

#include "OCAudioTypes.h"
#include "OCAudioUserSettings.h"
#include "OCPlayerController.h"

#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

bool UOCR13FrontendAudioSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13FrontendAudioSubsystem::Tick(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World || DeltaTime <= 0.0f) return;

    AOCPlayerController* PC = Cast<AOCPlayerController>(World->GetFirstPlayerController());
    const bool bPregameFrontend = PC && PC->IsLocalController() &&
        PC->IsFrontendMenuVisible() && PC->GetPawn() == nullptr;

    if (bPregameFrontend)
    {
        EnterPregameFrontendAudio();
    }
    else
    {
        LeavePregameFrontendAudio();
    }
}

void UOCR13FrontendAudioSubsystem::EnterPregameFrontendAudio()
{
    UOCAudioUserSettings* AudioSettings = UOCAudioUserSettings::Get();
    if (!AudioSettings) return;

    if (!bWeaponBusSuppressed)
    {
        // Listen-server smoke/gameplay may already have bots fighting behind the frontend. Muting only the local
        // weapon bus keeps the server simulation alive while preventing those shots from leaking into the main menu.
        SavedWeaponsVolume = AudioSettings->WeaponsVolume;
        AudioSettings->WeaponsVolume = 0.0f;
        bWeaponBusSuppressed = true;
    }

    bPregameAudioActive = true;
    StartMenuMusicIfAvailable();

    if (MenuMusic)
    {
        const float MusicVolume = AudioSettings->bMenuMusicEnabled
            ? AudioSettings->GetBusVolume(EOCAudioBus::Music) * 0.72f
            : 0.0f;
        MenuMusic->SetVolumeMultiplier(MusicVolume);
    }
}

void UOCR13FrontendAudioSubsystem::LeavePregameFrontendAudio()
{
    if (!bPregameAudioActive && !bWeaponBusSuppressed && !MenuMusic) return;

    bPregameAudioActive = false;
    RestoreWeaponBus();
    if (MenuMusic)
    {
        MenuMusic->OnAudioFinished.RemoveDynamic(this, &UOCR13FrontendAudioSubsystem::HandleMenuMusicFinished);
        MenuMusic->Stop();
        MenuMusic->DestroyComponent();
        MenuMusic = nullptr;
    }
}

void UOCR13FrontendAudioSubsystem::StartMenuMusicIfAvailable()
{
    if (MenuMusic || !bPregameAudioActive) return;
    UWorld* World = GetWorld();
    if (!World || World->GetNetMode() == NM_DedicatedServer) return;

    USoundBase* MenuSound = LoadObject<USoundBase>(nullptr,
        TEXT("/Game/R13/Audio/menu_ambient.menu_ambient"));
    if (!MenuSound)
    {
        if (!bWarnedMissingMusic)
        {
            UE_LOG(LogTemp, Warning,
                TEXT("R13 frontend audio: /Game/R13/Audio/menu_ambient is not imported yet; menu remains silent instead of leaking combat audio."));
            bWarnedMissingMusic = true;
        }
        return;
    }

    UOCAudioUserSettings* AudioSettings = UOCAudioUserSettings::Get();
    const float MusicVolume = AudioSettings && AudioSettings->bMenuMusicEnabled
        ? AudioSettings->GetBusVolume(EOCAudioBus::Music) * 0.72f
        : 0.0f;
    MenuMusic = UGameplayStatics::SpawnSound2D(
        this, MenuSound, MusicVolume, 1.0f, 0.0f, nullptr, false, false);
    if (MenuMusic)
    {
        MenuMusic->OnAudioFinished.AddDynamic(this, &UOCR13FrontendAudioSubsystem::HandleMenuMusicFinished);
    }
}

void UOCR13FrontendAudioSubsystem::HandleMenuMusicFinished()
{
    if (!bPregameAudioActive || !MenuMusic) return;
    MenuMusic->Play(0.0f);
}

void UOCR13FrontendAudioSubsystem::RestoreWeaponBus()
{
    if (!bWeaponBusSuppressed) return;
    if (UOCAudioUserSettings* AudioSettings = UOCAudioUserSettings::Get())
    {
        AudioSettings->WeaponsVolume = SavedWeaponsVolume;
    }
    bWeaponBusSuppressed = false;
}

void UOCR13FrontendAudioSubsystem::Deinitialize()
{
    LeavePregameFrontendAudio();
    Super::Deinitialize();
}

TStatId UOCR13FrontendAudioSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCR13FrontendAudioSubsystem, STATGROUP_Tickables);
}
