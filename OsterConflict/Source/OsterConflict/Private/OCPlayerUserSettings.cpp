#include "OCPlayerUserSettings.h"

#include "DynamicRHI.h"
#include "Engine/Engine.h"
#include "GameFramework/GameUserSettings.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformMisc.h"

UOCPlayerUserSettings* UOCPlayerUserSettings::Get()
{
    UOCPlayerUserSettings* Settings = GetMutableDefault<UOCPlayerUserSettings>();
    Settings->ValidateSettingsSchema();
    Settings->EnsureInitialGraphicsProfile();
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

void UOCPlayerUserSettings::EnsureInitialGraphicsProfile()
{
    UGameUserSettings* GameSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
    if (!GameSettings) return;

    GameSettings->LoadSettings(false);

    if (!bInitialGraphicsProfileApplied)
    {
        // Pass 16: the failed laptop run proved that relying on whatever scalability UE happens to
        // inherit on first launch can make the game start at ~5 FPS. Apply a one-time CEILING only:
        // existing settings that are already cheaper are never raised, and after this flag is saved
        // the player's manual graphics choices are never overwritten on future launches.
        auto SafeQuality = [](int32 Current, int32 Ceiling)
        {
            return Current >= 0 ? FMath::Min(Current, Ceiling) : Ceiling;
        };

        GameSettings->SetViewDistanceQuality(SafeQuality(GameSettings->GetViewDistanceQuality(), 1));
        GameSettings->SetShadowQuality(SafeQuality(GameSettings->GetShadowQuality(), 0));
        GameSettings->SetTextureQuality(SafeQuality(GameSettings->GetTextureQuality(), 1));
        GameSettings->SetVisualEffectQuality(SafeQuality(GameSettings->GetVisualEffectQuality(), 1));
        GameSettings->SetFoliageQuality(SafeQuality(GameSettings->GetFoliageQuality(), 0));
        GameSettings->SetPostProcessingQuality(SafeQuality(GameSettings->GetPostProcessingQuality(), 1));
        GameSettings->SetAntiAliasingQuality(SafeQuality(GameSettings->GetAntiAliasingQuality(), 1));
        GameSettings->SetShadingQuality(SafeQuality(GameSettings->GetShadingQuality(), 1));
        GameSettings->SetGlobalIlluminationQuality(SafeQuality(GameSettings->GetGlobalIlluminationQuality(), 0));
        GameSettings->SetReflectionQuality(SafeQuality(GameSettings->GetReflectionQuality(), 0));
        GameSettings->SetLandscapeQuality(SafeQuality(GameSettings->GetLandscapeQuality(), 1));

        float NormalizedScale = 1.0f;
        float CurrentScale = 100.0f;
        float MinScale = 50.0f;
        float MaxScale = 100.0f;
        GameSettings->GetResolutionScaleInformationEx(NormalizedScale, CurrentScale, MinScale, MaxScale);
        if (CurrentScale > 75.0f)
        {
            GameSettings->SetResolutionScaleValueEx(75.0f);
        }

        GameSettings->ApplySettings(false);
        GameSettings->SaveSettings();

        bInitialGraphicsProfileApplied = true;
        SaveConfig();

        UE_LOG(LogTemp, Warning,
            TEXT("PASS16_INITIAL_GRAPHICS_PROFILE_APPLIED view<=1 shadow=0 texture<=1 effects<=1 foliage=0 post<=1 aa<=1 shading<=1 gi=0 reflection=0 landscape<=1 resolution_scale<=75"));
    }

    // Log actual gameplay renderer identity once the RHI exists. Get() is used from several UI/controller
    // paths, so a first call that happens before RHI initialization simply waits for a later call.
    static bool bRuntimeGraphicsIdentityLogged = false;
    if (GDynamicRHI && !bRuntimeGraphicsIdentityLogged)
    {
        float NormalizedScale = 1.0f;
        float CurrentScale = 100.0f;
        float MinScale = 50.0f;
        float MaxScale = 100.0f;
        GameSettings->GetResolutionScaleInformationEx(NormalizedScale, CurrentScale, MinScale, MaxScale);

        const FString GPUBrand = FPlatformMisc::GetPrimaryGPUBrand();
        const TCHAR* RHIName = GDynamicRHI->GetName();
        UE_LOG(LogTemp, Display,
            TEXT("PASS16_RUNTIME_GRAPHICS_IDENTITY gpu=%s rhi=%s resolution=%dx%d scale=%.0f view=%d shadow=%d texture=%d effects=%d foliage=%d post=%d aa=%d shading=%d gi=%d reflection=%d landscape=%d"),
            *GPUBrand,
            RHIName,
            GameSettings->GetScreenResolution().X,
            GameSettings->GetScreenResolution().Y,
            CurrentScale,
            GameSettings->GetViewDistanceQuality(),
            GameSettings->GetShadowQuality(),
            GameSettings->GetTextureQuality(),
            GameSettings->GetVisualEffectQuality(),
            GameSettings->GetFoliageQuality(),
            GameSettings->GetPostProcessingQuality(),
            GameSettings->GetAntiAliasingQuality(),
            GameSettings->GetShadingQuality(),
            GameSettings->GetGlobalIlluminationQuality(),
            GameSettings->GetReflectionQuality(),
            GameSettings->GetLandscapeQuality());
        bRuntimeGraphicsIdentityLogged = true;
    }
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
    // LastUsername, LastServerAddress and bInitialGraphicsProfileApplied deliberately survive Reset Defaults.
    // The graphics reset button stages UGameUserSettings separately; it must not make the next launch silently
    // re-apply Pass 16 after the user intentionally chose/reset video settings.
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