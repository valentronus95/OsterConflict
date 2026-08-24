#!/usr/bin/env python3
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
HEADER = SRC / "Public" / "OCPlayerUserSettings.h"
CPP = SRC / "Private" / "OCPlayerUserSettings.cpp"
BUILD = SRC / "OsterConflict.Build.cs"
UI = SRC / "Private" / "OCGameUIRootWidget.cpp"
LAUNCHER = ROOT / "RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS16 VERIFY FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        raise SystemExit(f"PASS16 VERIFY FAIL: {label}: missing {needle!r}")


def forbid(text: str, needle: str, label: str) -> None:
    if needle in text:
        raise SystemExit(f"PASS16 VERIFY FAIL: {label}: forbidden {needle!r}")

header = read(HEADER)
cpp = read(CPP)
build = read(BUILD)
ui = read(UI)
launcher = read(LAUNCHER)

for needle in (
    "void EnsureInitialGraphicsProfile();",
    "bool bInitialGraphicsProfileApplied = false;",
    "bool bPass39GraphicsQualityRecoveryApplied = false;",
    "bool bPass42GraphicsClarityRecoveryApplied = false;",
    "UPROPERTY(Config",
):
    require(header, needle, "persistent graphics contract")

get_match = re.search(r"UOCPlayerUserSettings\*\s+UOCPlayerUserSettings::Get\(\)\s*\{(?P<body>.*?)\n\}", cpp, re.S)
if not get_match:
    raise SystemExit("PASS16 VERIFY FAIL: could not locate UOCPlayerUserSettings::Get")
get_body = get_match.group("body")
require(get_body, "ValidateSettingsSchema();", "settings Get schema validation")
require(get_body, "EnsureInitialGraphicsProfile();", "settings Get graphics initialization")
if get_body.index("ValidateSettingsSchema();") > get_body.index("EnsureInitialGraphicsProfile();"):
    raise SystemExit("PASS16 VERIFY FAIL: graphics initialization must run after schema validation")

require(cpp, "if (!bInitialGraphicsProfileApplied)", "one-time graphics guard")
require(cpp, "GameSettings->LoadSettings(false);", "persisted settings load")

# Pass 42 keeps Pass 39's conservative expensive-lighting limits but restores image clarity:
# native 100% internal scale and Texture Quality 3 for the automatic profile.
for needle in (
    "auto SafeQuality", "FMath::Min(Current, Ceiling)",
    "SetViewDistanceQuality(SafeQuality(GameSettings->GetViewDistanceQuality(), 2))",
    "SetShadowQuality(SafeQuality(GameSettings->GetShadowQuality(), 1))",
    "SetTextureQuality(SafeQuality(GameSettings->GetTextureQuality(), 3))",
    "SetVisualEffectQuality(SafeQuality(GameSettings->GetVisualEffectQuality(), 2))",
    "SetFoliageQuality(SafeQuality(GameSettings->GetFoliageQuality(), 1))",
    "SetPostProcessingQuality(SafeQuality(GameSettings->GetPostProcessingQuality(), 2))",
    "SetAntiAliasingQuality(SafeQuality(GameSettings->GetAntiAliasingQuality(), 2))",
    "SetShadingQuality(SafeQuality(GameSettings->GetShadingQuality(), 2))",
    "SetGlobalIlluminationQuality(SafeQuality(GameSettings->GetGlobalIlluminationQuality(), 1))",
    "SetReflectionQuality(SafeQuality(GameSettings->GetReflectionQuality(), 1))",
    "SetLandscapeQuality(SafeQuality(GameSettings->GetLandscapeQuality(), 2))",
    "SetResolutionScaleValueEx(100.0f)",
    "GameSettings->ApplySettings(false);", "GameSettings->SaveSettings();",
    "bInitialGraphicsProfileApplied = true;",
    "bPass42GraphicsClarityRecoveryApplied = true;",
    "PASS16_INITIAL_GRAPHICS_PROFILE_APPLIED",
    "PASS42_GRAPHICS_CLARITY_RECOVERY_APPLIED mode=new_profile scale=100 texture_ceiling=3",
):
    require(cpp, needle, "Pass 42 balanced native-scale graphics ceiling")

# Existing users may still carry the old Pass 16 signature. Pass 39 can recognize/migrate that family,
# then Pass 42 upgrades only the recognizable automatic Pass 39 family to 100% + Texture 3.
for needle in (
    "else if (!bPass39GraphicsQualityRecoveryApplied)",
    "const bool bLooksLikeLegacyPass16",
    "CurrentScale <= 75.5f",
    "GameSettings->GetShadowQuality() == 0",
    "GameSettings->GetGlobalIlluminationQuality() == 0",
    "GameSettings->GetReflectionQuality() == 0",
    "if (bLooksLikeLegacyPass16)",
    "SetResolutionScaleValueEx(85.0f)",
    "PASS39_GRAPHICS_QUALITY_RECOVERY_APPLIED",
    "PASS39_GRAPHICS_CUSTOM_PROFILE_PRESERVED",
    "bPass39GraphicsQualityRecoveryApplied = true;",
    "if (!bPass42GraphicsClarityRecoveryApplied)",
    "const bool bLooksLikeAutomaticPass39",
    "CurrentScale <= 85.5f",
    "GameSettings->SetTextureQuality(3);",
    "PASS42_GRAPHICS_CLARITY_RECOVERY_APPLIED mode=pass39_auto scale=100 texture=3 expensive_lighting_unchanged=1",
    "PASS42_GRAPHICS_CUSTOM_PROFILE_PRESERVED automatic_pass39_profile=0",
    "bPass42GraphicsClarityRecoveryApplied = true;",
    "PASS39_GRAPHICS_QUALITY_PROFILE_READY",
):
    require(cpp, needle, "one-time Pass 39 to Pass 42 graphics migration")

reset_match = re.search(r"void\s+UOCPlayerUserSettings::ResetPlayerDefaults\(\)\s*\{(?P<body>.*?)\n\}", cpp, re.S)
if not reset_match:
    raise SystemExit("PASS16 VERIFY FAIL: could not locate ResetPlayerDefaults")
reset_body = reset_match.group("body")
forbid(reset_body, "bInitialGraphicsProfileApplied = false", "Reset Defaults re-arming graphics profile")
forbid(reset_body, "bPass39GraphicsQualityRecoveryApplied = false", "Reset Defaults re-arming Pass 39 migration")
forbid(reset_body, "bPass42GraphicsClarityRecoveryApplied = false", "Reset Defaults re-arming Pass 42 migration")

for needle in (
    "SetOverallScalabilityLevel(Preset)",
    "SetShadowQuality(QualityFromString(ShadowCombo->GetSelectedOption()))",
    "SetGlobalIlluminationQuality(QualityFromString(GlobalIlluminationCombo->GetSelectedOption()))",
    "SetReflectionQuality(QualityFromString(ReflectionCombo->GetSelectedOption()))",
    "ApplySettings(false)",
):
    require(ui, needle, "manual graphics controls remain authoritative")

for needle in (
    '#include "DynamicRHI.h"', '#include "HAL/PlatformMisc.h"',
    "GDynamicRHI && !bRuntimeGraphicsIdentityLogged", "FPlatformMisc::GetPrimaryGPUBrand()",
    "GDynamicRHI->GetName()", "PASS16_RUNTIME_GRAPHICS_IDENTITY gpu=%s rhi=%s",
):
    require(cpp, needle, "runtime GPU/RHI identity")
require(build, '"RHI"', "RHI module dependency")

# Pass 19 extends the same focused launcher; Pass 16 still owns the renderer evidence inside it.
for needle in (
    'set "VERIFY16=%~dp0VERIFY_RUNTIME_GRAPHICS_PASS_16.py"',
    '%PY_CMD% "%VERIFY16%"', "PASS16_RUNTIME_GRAPHICS_IDENTITY",
    "Pass 15-19 Recovery", "PASS 15-19 FOCUSED RUNTIME RECOVERY: AUTOMATED EVIDENCE PASSED",
):
    require(launcher, needle, "Pass 16 runtime launcher integration")
forbid(launcher.lower(), "-nullrhi", "focused runtime launcher must use a real renderer")

print("RUNTIME GRAPHICS PASS 16/39/42 SOURCE CONTRACT PASS")
print("- automatic first-run profile uses native 100% scale + Texture Quality 3 with conservative expensive lighting")
print("- legacy Pass 16 / automatic Pass 39 profiles receive controlled one-time migrations")
print("- user-customized graphics profiles remain authoritative")
print("- real GPU/RHI evidence and manual graphics controls remain intact")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 visual/FPS acceptance still required")
