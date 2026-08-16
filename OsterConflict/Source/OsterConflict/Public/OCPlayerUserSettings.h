#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "InputCoreTypes.h"
#include "OCPlayerUserSettings.generated.h"

UENUM(BlueprintType)
enum class EOCColorVisionMode : uint8
{
    Off,
    Deuteranopia,
    Protanopia,
    Tritanopia
};

/**
 * S17B persistent non-renderer preferences. Engine video/scalability settings remain in UGameUserSettings;
 * this object stores Oster-specific controls, HUD and accessibility choices.
 */
UCLASS(Config=GameUserSettings, ConfigDoNotCheckDefaults, BlueprintType)
class OSTERCONFLICT_API UOCPlayerUserSettings : public UObject
{
    GENERATED_BODY()
public:
    static UOCPlayerUserSettings* Get();

    UFUNCTION(BlueprintCallable, Category="Settings") void SavePlayerSettings();
    UFUNCTION(BlueprintCallable, Category="Settings") void ResetPlayerDefaults();
    UFUNCTION(BlueprintCallable, Category="Settings") void ApplyPresentationCVars();
    UFUNCTION(BlueprintCallable, Category="Settings") void ValidateSettingsSchema();

    UFUNCTION(BlueprintCallable, Category="Frontend") void SetFrontendIdentity(const FString& Username, const FString& ServerAddress);
    UFUNCTION(BlueprintPure, Category="Frontend") FString GetSavedUsername() const { return LastUsername.IsEmpty() ? TEXT("Гравець") : LastUsername; }
    UFUNCTION(BlueprintPure, Category="Frontend") FString GetLastServerAddress() const { return LastServerAddress.IsEmpty() ? TEXT("127.0.0.1:7777") : LastServerAddress; }

    static constexpr int32 CurrentSettingsSchemaVersion = 1;
    UPROPERTY(Config, VisibleAnywhere, BlueprintReadOnly, Category="Settings") int32 SettingsSchemaVersion = CurrentSettingsSchemaVersion;

    // S19A: identity/last endpoint are user convenience data and intentionally survive Reset Defaults.
    UPROPERTY(Config, VisibleAnywhere, BlueprintReadOnly, Category="Frontend") FString LastUsername = TEXT("Гравець");
    UPROPERTY(Config, VisibleAnywhere, BlueprintReadOnly, Category="Frontend") FString LastServerAddress = TEXT("127.0.0.1:7777");

    FKey GetKey(FName ActionId) const;
    FName GetKeyName(FName ActionId) const;
    void SetKeyName(FName ActionId, FName KeyName);

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category="Controls", meta=(ClampMin="0.10", ClampMax="4.00")) float MouseSensitivity = 1.0f;
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category="Controls", meta=(ClampMin="0.25", ClampMax="1.50")) float AimSensitivityMultiplier = 0.72f;
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category="Controls") bool bInvertMouseY = false;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category="Interface", meta=(ClampMin="75.0", ClampMax="120.0")) float FieldOfView = 90.0f;
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category="Interface", meta=(ClampMin="0.75", ClampMax="1.25")) float HUDScale = 1.0f;
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category="Interface") bool bShowFPS = false;
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category="Interface") bool bShowPing = true;
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category="Interface") bool bShowCrosshair = true;
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category="Interface") bool bShowHitMarker = true;

    /** 0 Off, 1 Reduced, 2 Full. Presentation only; damage remains unchanged. */
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category="Accessibility", meta=(ClampMin="0", ClampMax="2")) int32 GoreLevel = 2;
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category="Accessibility") bool bSubtitles = true;
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category="Accessibility") bool bReduceFlashes = false;
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category="Accessibility", meta=(ClampMin="0.0", ClampMax="1.0")) float CameraShakeScale = 1.0f;
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category="Accessibility") EOCColorVisionMode ColorVisionMode = EOCColorVisionMode::Off;

    // Source-only key profile. Stored as key FNames to remain robust in config files.
    UPROPERTY(Config) FName KeyMoveForward = TEXT("W");
    UPROPERTY(Config) FName KeyMoveBackward = TEXT("S");
    UPROPERTY(Config) FName KeyMoveLeft = TEXT("A");
    UPROPERTY(Config) FName KeyMoveRight = TEXT("D");
    UPROPERTY(Config) FName KeyJump = TEXT("SpaceBar");
    UPROPERTY(Config) FName KeySprint = TEXT("LeftShift");
    UPROPERTY(Config) FName KeyCrouch = TEXT("LeftControl");
    UPROPERTY(Config) FName KeyFire = TEXT("LeftMouseButton");
    UPROPERTY(Config) FName KeyAim = TEXT("RightMouseButton");
    UPROPERTY(Config) FName KeyReload = TEXT("R");
    UPROPERTY(Config) FName KeyInteract = TEXT("E");
    UPROPERTY(Config) FName KeyThrowGrenade = TEXT("F");
    UPROPERTY(Config) FName KeyScoreboard = TEXT("Tab");
    UPROPERTY(Config) FName KeyChat = TEXT("T");
};
