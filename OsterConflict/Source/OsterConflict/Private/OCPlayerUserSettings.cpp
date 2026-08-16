#include "OCPlayerUserSettings.h"

#include "HAL/IConsoleManager.h"

UOCPlayerUserSettings* UOCPlayerUserSettings::Get()
{
    UOCPlayerUserSettings* Settings = GetMutableDefault<UOCPlayerUserSettings>();
    Settings->ValidateSettingsSchema();
    return Settings;
}

void UOCPlayerUserSettings::ValidateSettingsSchema()
{
    if (SettingsSchemaVersion == CurrentSettingsSchemaVersion) return;

    // First schema contract: unknown/future/invalid schema falls back to safe Oster-specific defaults.
    // Renderer settings owned by UGameUserSettings are intentionally not erased here.
    UE_LOG(LogTemp, Warning, TEXT("Settings schema mismatch: found=%d current=%d. Resetting Oster-specific preferences."),
        SettingsSchemaVersion, CurrentSettingsSchemaVersion);
    ResetPlayerDefaults();
    SettingsSchemaVersion = CurrentSettingsSchemaVersion;
    SaveConfig();
}

void UOCPlayerUserSettings::SavePlayerSettings()
{
    ApplyPresentationCVars();
    SaveConfig();
}

void UOCPlayerUserSettings::SetFrontendIdentity(const FString& Username, const FString& ServerAddress)
{
    FString CleanName = Username;
    CleanName.TrimStartAndEndInline();
    if (!CleanName.IsEmpty()) LastUsername = CleanName.Left(24);

    FString CleanAddress = ServerAddress;
    CleanAddress.TrimStartAndEndInline();
    if (!CleanAddress.IsEmpty() && !CleanAddress.Contains(TEXT(" ")) && CleanAddress.Len() <= 128)
    {
        LastServerAddress = CleanAddress;
    }
    SaveConfig();
}

void UOCPlayerUserSettings::ApplyPresentationCVars()
{
    if (IConsoleVariable* Gore = IConsoleManager::Get().FindConsoleVariable(TEXT("oc.GoreLevel")))
    {
        Gore->Set(FMath::Clamp(GoreLevel, 0, 2), ECVF_SetByGameSetting);
    }
}

void UOCPlayerUserSettings::ResetPlayerDefaults()
{
    // LastUsername and LastServerAddress deliberately survive Reset Defaults.
    SettingsSchemaVersion = CurrentSettingsSchemaVersion;
    MouseSensitivity = 1.0f;
    AimSensitivityMultiplier = 0.72f;
    bInvertMouseY = false;
    FieldOfView = 90.0f;
    HUDScale = 1.0f;
    bShowFPS = false;
    bShowPing = true;
    bShowCrosshair = true;
    bShowHitMarker = true;
    GoreLevel = 2;
    bSubtitles = true;
    bReduceFlashes = false;
    CameraShakeScale = 1.0f;
    ColorVisionMode = EOCColorVisionMode::Off;
    KeyMoveForward = TEXT("W"); KeyMoveBackward = TEXT("S"); KeyMoveLeft = TEXT("A"); KeyMoveRight = TEXT("D");
    KeyJump = TEXT("SpaceBar"); KeySprint = TEXT("LeftShift"); KeyCrouch = TEXT("LeftControl");
    KeyFire = TEXT("LeftMouseButton"); KeyAim = TEXT("RightMouseButton"); KeyReload = TEXT("R"); KeyInteract = TEXT("E");
    KeyThrowGrenade = TEXT("F"); KeyScoreboard = TEXT("Tab"); KeyChat = TEXT("T");
}

FName UOCPlayerUserSettings::GetKeyName(FName ActionId) const
{
    if (ActionId == TEXT("MoveForward")) return KeyMoveForward;
    if (ActionId == TEXT("MoveBackward")) return KeyMoveBackward;
    if (ActionId == TEXT("MoveLeft")) return KeyMoveLeft;
    if (ActionId == TEXT("MoveRight")) return KeyMoveRight;
    if (ActionId == TEXT("Jump")) return KeyJump;
    if (ActionId == TEXT("Sprint")) return KeySprint;
    if (ActionId == TEXT("Crouch")) return KeyCrouch;
    if (ActionId == TEXT("Fire")) return KeyFire;
    if (ActionId == TEXT("Aim")) return KeyAim;
    if (ActionId == TEXT("Reload")) return KeyReload;
    if (ActionId == TEXT("Interact")) return KeyInteract;
    if (ActionId == TEXT("ThrowGrenade")) return KeyThrowGrenade;
    if (ActionId == TEXT("Scoreboard")) return KeyScoreboard;
    if (ActionId == TEXT("Chat")) return KeyChat;
    return NAME_None;
}

FKey UOCPlayerUserSettings::GetKey(FName ActionId) const
{
    const FName Name = GetKeyName(ActionId);
    return Name.IsNone() ? FKey() : FKey(Name);
}

void UOCPlayerUserSettings::SetKeyName(FName ActionId, FName KeyName)
{
    if (KeyName.IsNone()) return;
    if (ActionId == TEXT("MoveForward")) KeyMoveForward = KeyName;
    else if (ActionId == TEXT("MoveBackward")) KeyMoveBackward = KeyName;
    else if (ActionId == TEXT("MoveLeft")) KeyMoveLeft = KeyName;
    else if (ActionId == TEXT("MoveRight")) KeyMoveRight = KeyName;
    else if (ActionId == TEXT("Jump")) KeyJump = KeyName;
    else if (ActionId == TEXT("Sprint")) KeySprint = KeyName;
    else if (ActionId == TEXT("Crouch")) KeyCrouch = KeyName;
    else if (ActionId == TEXT("Fire")) KeyFire = KeyName;
    else if (ActionId == TEXT("Aim")) KeyAim = KeyName;
    else if (ActionId == TEXT("Reload")) KeyReload = KeyName;
    else if (ActionId == TEXT("Interact")) KeyInteract = KeyName;
    else if (ActionId == TEXT("ThrowGrenade")) KeyThrowGrenade = KeyName;
    else if (ActionId == TEXT("Scoreboard")) KeyScoreboard = KeyName;
    else if (ActionId == TEXT("Chat")) KeyChat = KeyName;
}
