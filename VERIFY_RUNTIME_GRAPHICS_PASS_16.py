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

ensure_match = re.search(
    r"void\s+UOCPlayerUserSettings::EnsureInitialGraphicsProfile\(\)\s*\{(?P<body>.*?)\n\}\n\nvoid\s+UOCPlayerUserSettings::SavePlayerSettings",
    cpp,
    re.S,
)
if not ensure_match:
    raise SystemExit("PASS16 VERIFY FAIL: could not isolate EnsureInitialGraphicsProfile")
ensure_body = ensure_match.group("body")

require(cpp, "if (!bInitialGraphicsProfileApplied)", "one-time graphics guard")
require(cpp, "GameSettings->LoadSettings(false);", "persisted settings load")

# Pass42 keeps Pass39's conservative expensive-lighting limits but restores image clarity:
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
    "GameSettings->SaveSettings();",
    "bInitialGraphicsProfileApplied = true;",
    "bPass42GraphicsClarityRecoveryApplied = true;",
    "PASS16_INITIAL_GRAPHICS_PROFILE_APPLIED",
    "PASS42_GRAPHICS_CLARITY_RECOVERY_APPLIED mode=new_profile scale=100 texture_ceiling=3",
    "PASS43_STARTUP_GRAPHICS_PERSIST_ONLY_READY",
):
    require(cpp, needle, "Pass42/43 balanced native-scale graphics ceiling")

# Pass43: Get() is reached from frontend NativeConstruct. Automatic migrations must never execute a live
# UGameUserSettings apply there; that can invalidate Slate's backbuffer while SlateRHIRenderer is building it.
forbid(ensure_body, "GameSettings->ApplySettings(", "automatic startup graphics must not live-apply inside Slate construction")
require(ensure_body, "live_apply=0 slate_construction_safe=1", "Pass43 startup persistence marker")

# Existing users may still carry the old Pass16 signature. Pass39 can recognize/migrate that family,
# then Pass42 upgrades only the recognizable automatic Pass39 family to 100% + Texture 3.
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
    require(cpp, needle, "one-time Pass39 to Pass42 graphics migration")

reset_match = re.search(r"void\s+UOCPlayerUserSettings::ResetPlayerDefaults\(\)\s*\{(?P<body>.*?)\n\}", cpp, re.S)
if not reset_match:
    raise SystemExit("PASS16 VERIFY FAIL: could not locate ResetPlayerDefaults")
reset_body = reset_match.group("body")
forbid(reset_body, "bInitialGraphicsProfileApplied = false", "Reset Defaults re-arming graphics profile")
forbid(reset_body, "bPass39GraphicsQualityRecoveryApplied = false", "Reset Defaults re-arming Pass39 migration")
forbid(reset_body, "bPass42GraphicsClarityRecoveryApplied = false", "Reset Defaults re-arming Pass42 migration")

# Explicit settings UI remains the only live apply route. The user action occurs after the viewport is valid.
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

# Pass45 keeps using the same focused recovery launcher. Pass16 owns renderer evidence inside it,
# but must not depend on an obsolete window caption or historical Pass15-19 banner string.
for needle in (
    'set "VERIFY16=%~dp0VERIFY_RUNTIME_GRAPHICS_PASS_16.py"',
    '%PY_CMD% "%VERIFY16%"',
    "PASS16_RUNTIME_GRAPHICS_IDENTITY",
    "Oster Conflict Focused Recovery",
    "PASS 45 FOCUSED RUNTIME RECOVERY: AUTOMATED EVIDENCE PASSED",
    "PASS45_INITIAL_BASE_DEPLOYMENT_",
    "vehicle_revalidation=0",
    "-d3d11",
    "-sm5",
    "-nohdr",
):
    require(launcher, needle, "Pass16 current focused runtime launcher integration")
forbid(launcher.lower(), "-nullrhi", "focused runtime launcher must use a real renderer")

print("RUNTIME GRAPHICS PASS16/39/42/43 SOURCE CONTRACT PASS")
print("- automatic first-run profile uses native 100% scale + Texture Quality 3 with conservative expensive lighting")
print("- automatic startup migrations persist settings without live ApplySettings during Slate construction")
print("- legacy Pass16 / automatic Pass39 profiles receive controlled one-time migrations")
print("- explicit settings UI remains the only live graphics apply path")
print("- user-customized graphics profiles remain authoritative")
print("- current Pass45 focused launcher provides real DX11/SM5 GPU/RHI evidence without historical banner coupling")
print("STATUS: CODED_UNTESTED; local UE 5.8 startup runtime remains authoritative")
