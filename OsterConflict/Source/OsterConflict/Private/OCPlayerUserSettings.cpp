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

    if (!bInitialGraphicsProfileApplied)
    {
        // Only the first initialization may reload persisted engine settings. Repeating LoadSettings()
        // from every Get() would overwrite pending changes while the graphics menu is open.
        GameSettings->LoadSettings(false);

        // Pass 39 replaces the old emergency-looking Pass 16 profile. The previous 75% screen scale,
        // zero shadows/GI/reflections and mostly quality-1 ceiling made the user's runtime visibly blurry
        // while the real FPS collapse was caused elsewhere. Keep a conservative balanced ceiling instead:
        // expensive lighting remains Low, while textures, AA, landscape and view distance are readable.
        auto SafeQuality = [](int32 Current, int32 Ceiling)
        {
            return Current >= 0 ? FMath::Min(Current, Ceiling) : Ceiling;
        };

        GameSettings->SetViewDistanceQuality(SafeQuality(GameSettings->GetViewDistanceQuality(), 2));
        GameSettings->SetShadowQuality(SafeQuality(GameSettings->GetShadowQuality(), 1));
        GameSettings->SetTextureQuality(SafeQuality(GameSettings->GetTextureQuality(), 2));
        GameSettings->SetVisualEffectQuality(SafeQuality(GameSettings->GetVisualEffectQuality(), 2));
        GameSettings->SetFoliageQuality(SafeQuality(GameSettings->GetFoliageQuality(), 1));
        GameSettings->SetPostProcessingQuality(SafeQuality(GameSettings->GetPostProcessingQuality(), 2));
        GameSettings->SetAntiAliasingQuality(SafeQuality(GameSettings->GetAntiAliasingQuality(), 2));
        GameSettings->SetShadingQuality(SafeQuality(GameSettings->GetShadingQuality(), 2));
        GameSettings->SetGlobalIlluminationQuality(SafeQuality(GameSettings->GetGlobalIlluminationQuality(), 1));
        GameSettings->SetReflectionQuality(SafeQuality(GameSettings->GetReflectionQuality(), 1));
        GameSettings->SetLandscapeQuality(SafeQuality(GameSettings->GetLandscapeQuality(), 2));

        float NormalizedScale = 1.0f;
        float CurrentScale = 100.0f;
        float MinScale = 50.0f;
        float MaxScale = 100.0f;
        GameSettings->GetResolutionScaleInformationEx(NormalizedScale, CurrentScale, MinScale, MaxScale);
        if (CurrentScale > 85.0f)
        {
            GameSettings->SetResolutionScaleValueEx(85.0f);
        }

        GameSettings->ApplySettings(false);
        GameSettings->SaveSettings();

        bInitialGraphicsProfileApplied = true;
        bPass39GraphicsQualityRecoveryApplied = true;
        SaveConfig();

        UE_LOG(LogTemp, Warning,
            TEXT("PASS16_INITIAL_GRAPHICS_PROFILE_APPLIED view<=2 shadow<=1 texture<=2 effects<=2 foliage<=1 post<=2 aa<=2 shading<=2 gi<=1 reflection<=1 landscape<=2 resolution_scale<=85"));
        UE_LOG(LogTemp, Display,
            TEXT("PASS39_GRAPHICS_QUALITY_RECOVERY_APPLIED mode=new_profile old_pass16_profile=0"));
    }
    else if (!bPass39GraphicsQualityRecoveryApplied)
    {
        // Existing Pass 16 installs already persisted the old degraded engine settings, so changing the
        // source ceiling alone would do nothing. Migrate exactly once, but only when the active profile
        // still matches the old automatic low-quality signature. Any user-customized profile is preserved.
        GameSettings->LoadSettings(false);

        float NormalizedScale = 1.0f;
        float CurrentScale = 100.0f;
        float MinScale = 50.0f;
        float MaxScale = 100.0f;
        GameSettings->GetResolutionScaleInformationEx(NormalizedScale, CurrentScale, MinScale, MaxScale);

        const bool bLooksLikeLegacyPass16 =
            CurrentScale <= 75.5f &&
            GameSettings->GetViewDistanceQuality() <= 1 &&
            GameSettings->GetShadowQuality() == 0 &&
            GameSettings->GetTextureQuality() <= 1 &&
            GameSettings->GetVisualEffectQuality() <= 1 &&
            GameSettings->GetFoliageQuality() == 0 &&
            GameSettings->GetPostProcessingQuality() <= 1 &&
            GameSettings->GetAntiAliasingQuality() <= 1 &&
            GameSettings->GetShadingQuality() <= 1 &&
            GameSettings->GetGlobalIlluminationQuality() == 0 &&
            GameSettings->GetReflectionQuality() == 0 &&
            GameSettings->GetLandscapeQuality() <= 1;

        if (bLooksLikeLegacyPass16)
        {
            GameSettings->SetViewDistanceQuality(2);
            GameSettings->SetShadowQuality(1);
            GameSettings->SetTextureQuality(2);
            GameSettings->SetVisualEffectQuality(2);
            GameSettings->SetFoliageQuality(1);
            GameSettings->SetPostProcessingQuality(2);
            GameSettings->SetAntiAliasingQuality(2);
            GameSettings->SetShadingQuality(2);
            GameSettings->SetGlobalIlluminationQuality(1);
            GameSettings->SetReflectionQuality(1);
            GameSettings->SetLandscapeQuality(2);
            GameSettings->SetResolutionScaleValueEx(85.0f);
            GameSettings->ApplySettings(false);
            GameSettings->SaveSettings();

            UE_LOG(LogTemp, Warning,
                TEXT("PASS39_GRAPHICS_QUALITY_RECOVERY_APPLIED mode=legacy_pass16 scale=85 view=2 shadow=1 texture=2 effects=2 foliage=1 post=2 aa=2 shading=2 gi=1 reflection=1 landscape=2"));
        }
        else
        {
            UE_LOG(LogTemp, Display,
                TEXT("PASS39_GRAPHICS_CUSTOM_PROFILE_PRESERVED old_pass16_profile=0"));
        }

        bPass39GraphicsQualityRecoveryApplied = true;
        SaveConfig();
    }

    static bool bPass39ProfileLogged = false;
    if (!bPass39ProfileLogged)
    {
        float NormalizedScale = 1.0f;
        float CurrentScale = 100.0f;
        float MinScale = 50.0f;
        float MaxScale = 100.0f;
        GameSettings->GetResolutionScaleInformationEx(NormalizedScale, CurrentScale, MinScale, MaxScale);
        UE_LOG(LogTemp, Display,
            TEXT("PASS39_GRAPHICS_QUALITY_PROFILE_READY scale=%.0f view=%d shadow=%d texture=%d effects=%d foliage=%d post=%d aa=%d shading=%d gi=%d reflection=%d landscape=%d"),
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
        bPass39ProfileLogged = true;
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
    // Identity and one-time graphics migration flags deliberately survive Reset Defaults. The graphics
    // reset button stages UGameUserSettings separately; it must not silently re-run an automatic profile.
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