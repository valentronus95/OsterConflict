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

# Persistent Oster-owned first-run flag. It is false only for a genuinely fresh local profile.
for needle in (
    "void EnsureInitialGraphicsProfile();",
    "bool bInitialGraphicsProfileApplied = false;",
    "UPROPERTY(Config",
):
    require(header, needle, "persistent first-run graphics contract")

# Every normal settings access initializes the safe ceiling after schema validation, before UI consumers read values.
get_match = re.search(
    r"UOCPlayerUserSettings\*\s+UOCPlayerUserSettings::Get\(\)\s*\{(?P<body>.*?)\n\}",
    cpp,
    re.S,
)
if not get_match:
    raise SystemExit("PASS16 VERIFY FAIL: could not locate UOCPlayerUserSettings::Get")
get_body = get_match.group("body")
require(get_body, "ValidateSettingsSchema();", "settings Get schema validation")
require(get_body, "EnsureInitialGraphicsProfile();", "settings Get first-run graphics initialization")
if get_body.index("ValidateSettingsSchema();") > get_body.index("EnsureInitialGraphicsProfile();"):
    raise SystemExit("PASS16 VERIFY FAIL: graphics initialization must run after schema validation")

# The disk reload must remain inside the one-time branch. Re-loading on every Get() would destroy pending UI edits.
require(cpp, "if (!bInitialGraphicsProfileApplied)", "one-time graphics guard")
require(cpp, "GameSettings->LoadSettings(false);", "first initialization persisted settings load")
load_pos = cpp.find("GameSettings->LoadSettings(false);")
guard_pos = cpp.find("if (!bInitialGraphicsProfileApplied)")
if load_pos < guard_pos:
    raise SystemExit("PASS16 VERIFY FAIL: LoadSettings runs outside the one-time initialization guard")

# Apply ceilings, never a blind upgrade. Existing cheaper values stay cheaper.
for needle in (
    "auto SafeQuality",
    "FMath::Min(Current, Ceiling)",
    "SetViewDistanceQuality(SafeQuality(GameSettings->GetViewDistanceQuality(), 1))",
    "SetShadowQuality(SafeQuality(GameSettings->GetShadowQuality(), 0))",
    "SetTextureQuality(SafeQuality(GameSettings->GetTextureQuality(), 1))",
    "SetVisualEffectQuality(SafeQuality(GameSettings->GetVisualEffectQuality(), 1))",
    "SetFoliageQuality(SafeQuality(GameSettings->GetFoliageQuality(), 0))",
    "SetPostProcessingQuality(SafeQuality(GameSettings->GetPostProcessingQuality(), 1))",
    "SetAntiAliasingQuality(SafeQuality(GameSettings->GetAntiAliasingQuality(), 1))",
    "SetShadingQuality(SafeQuality(GameSettings->GetShadingQuality(), 1))",
    "SetGlobalIlluminationQuality(SafeQuality(GameSettings->GetGlobalIlluminationQuality(), 0))",
    "SetReflectionQuality(SafeQuality(GameSettings->GetReflectionQuality(), 0))",
    "SetLandscapeQuality(SafeQuality(GameSettings->GetLandscapeQuality(), 1))",
    "if (CurrentScale > 75.0f)",
    "SetResolutionScaleValueEx(75.0f)",
    "GameSettings->ApplySettings(false);",
    "GameSettings->SaveSettings();",
    "bInitialGraphicsProfileApplied = true;",
    "PASS16_INITIAL_GRAPHICS_PROFILE_APPLIED",
):
    require(cpp, needle, "one-time safe graphics ceiling")
forbid(cpp, "SetOverallScalabilityLevel(3)", "Epic first-run preset")
forbid(cpp, "SetOverallScalabilityLevel(4)", "Cinematic first-run preset")

# Reset Defaults must not silently re-arm Pass 16 after a player intentionally changes/reset video settings.
reset_match = re.search(
    r"void\s+UOCPlayerUserSettings::ResetPlayerDefaults\(\)\s*\{(?P<body>.*?)\n\}",
    cpp,
    re.S,
)
if not reset_match:
    raise SystemExit("PASS16 VERIFY FAIL: could not locate ResetPlayerDefaults")
reset_body = reset_match.group("body")
forbid(reset_body, "bInitialGraphicsProfileApplied = false", "Reset Defaults re-arming first-run graphics")
forbid(reset_body, "bInitialGraphicsProfileApplied = true", "Reset Defaults mutating first-run graphics flag")

# Existing graphics menu remains authoritative after initialization.
for needle in (
    "SetOverallScalabilityLevel(Preset)",
    "SetShadowQuality(QualityFromString(ShadowCombo->GetSelectedOption()))",
    "SetGlobalIlluminationQuality(QualityFromString(GlobalIlluminationCombo->GetSelectedOption()))",
    "SetReflectionQuality(QualityFromString(ReflectionCombo->GetSelectedOption()))",
    "ApplySettings(false)",
):
    require(ui, needle, "manual graphics controls remain authoritative")

# Runtime evidence must reveal the real GPU/RHI and effective scalability values.
for needle in (
    '#include "DynamicRHI.h"',
    '#include "HAL/PlatformMisc.h"',
    "GDynamicRHI && !bRuntimeGraphicsIdentityLogged",
    "FPlatformMisc::GetPrimaryGPUBrand()",
    "GDynamicRHI->GetName()",
    "PASS16_RUNTIME_GRAPHICS_IDENTITY gpu=%s rhi=%s",
):
    require(cpp, needle, "runtime GPU/RHI identity")
require(build, '"RHI"', "RHI module dependency")

# Focused Windows recovery launcher runs Pass 15 + 16 and refuses a runtime without real renderer evidence.
for needle in (
    'set "VERIFY16=%~dp0VERIFY_RUNTIME_GRAPHICS_PASS_16.py"',
    '%PY_CMD% "%VERIFY16%"',
    "PASS16_RUNTIME_GRAPHICS_IDENTITY",
    "Pass 15-16 Recovery",
    "PASS 15-16 RUNTIME RECOVERY: AUTOMATED EVIDENCE PASSED",
):
    require(launcher, needle, "Pass 16 runtime launcher integration")
forbid(launcher.lower(), "-nullrhi", "focused runtime launcher must use a real renderer")

print("RUNTIME GRAPHICS PASS 16 SOURCE CONTRACT PASS")
print("- fresh profiles receive a one-time low-risk graphics CEILING, never a forced forever-preset")
print("- existing cheaper values are never raised and Reset Defaults does not re-arm Pass 16")
print("- shadows/GI/reflections/foliage are capped at 0; resolution scale is capped at 75%")
print("- manual graphics UI remains authoritative after initialization")
print("- gameplay logs expose actual GPU brand, RHI and effective scalability values")
print("- focused recovery launcher runs Pass 15 + 16 and requires real GPU/RHI evidence")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 compile/runtime FPS acceptance still required")
