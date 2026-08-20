#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13FrontendAudioSubsystem.generated.h"

class UAudioComponent;

/** Pregame-only frontend audio owner: suppresses local combat weapon bus and owns the menu music loop. */
UCLASS()
class OSTERCONFLICT_API UOCR13FrontendAudioSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual void Deinitialize() override;

private:
    UFUNCTION() void HandleMenuMusicFinished();
    void EnterPregameFrontendAudio();
    void LeavePregameFrontendAudio();
    void StartMenuMusicIfAvailable();
    void RestoreWeaponBus();

    UPROPERTY() TObjectPtr<UAudioComponent> MenuMusic;
    bool bPregameAudioActive = false;
    bool bWeaponBusSuppressed = false;
    bool bWarnedMissingMusic = false;
    float SavedWeaponsVolume = 1.0f;
};
