#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"

SETTINGS_H = SRC / "Public" / "OCPlayerUserSettings.h"
SETTINGS = SRC / "Private" / "OCPlayerUserSettings.cpp"
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


settings_h = read(SETTINGS_H)
settings = read(SETTINGS)
build = read(BUILD)
ui = read(UI)
launcher = read(LAUNCHER)

# One-time initialization must be persisted separately from UGameUserSettings.
for needle in (
    "EnsureInitialGraphicsProfile",
    "bInitialGraphicsProfileApplied = false",
    "UPROPERTY(Config",
):
    require(settings_h, needle, "graphics initialization flag/header")

# Get() must activate the first-run initialization, while disk reload is strictly inside the false-flag branch.
require(settings, "Settings->EnsureInitialGraphicsProfile();", "settings activation")
require(settings, "if (!bInitialGraphicsProfileApplied)", "one-time graphics guard")
require(settings, "GameSettings->LoadSettings(false);", "first initialization persisted settings load")
load_pos = settings.find("GameSettings->LoadSettings(false);")
guard_pos = settings.find("if (!bInitialGraphicsProfileApplied)")
if load_pos < guard_pos:
    raise SystemExit("PASS16 VERIFY FAIL: LoadSettings runs outside the one-time initialization guard")

# Pass 16 applies ceilings, never a blind higher preset. Existing cheaper values remain cheaper.
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
    "PASS16_INITIAL_GRAPHICS_PROFILE_APPLIED",
):
    require(settings, needle, "one-time safe renderer ceiling")
forbid(settings, "SetOverallScalabilityLevel(3)", "Epic first-run preset")
forbid(settings, "SetOverallScalabilityLevel(4)", "Cinematic first-run preset")

# User choices must become authoritative after initialization.
require(settings, "bInitialGraphicsProfileApplied = true;", "persist first-run completion")
require(settings, "GameSettings->SaveSettings();", "persist engine video settings")
require(settings, "bInitialGraphicsProfileApplied deliberately survive Reset Defaults", "reset does not arm silent next-launch override")

# Existing graphics menu remains fully functional and can replace Pass 16 values when the user presses Apply/Save.
for needle in (
    "SetOverallScalabilityLevel(Preset)",
    "SetShadowQuality(QualityFromString(ShadowCombo->GetSelectedOption()))",
    "SetGlobalIlluminationQuality(QualityFromString(GlobalIlluminationCombo->GetSelectedOption()))",
    "SetReflectionQuality(QualityFromString(ReflectionCombo->GetSelectedOption()))",
    "ApplySettings(false)",
):
    require(ui, needle, "manual graphics controls remain authoritative")

# Runtime evidence must identify the renderer from the real RHI process, not the null-RHI preflight log.
for needle in (
    '#include "DynamicRHI.h"',
    "FPlatformMisc::GetPrimaryGPUBrand()",
    "GDynamicRHI->GetName()",
    "PASS16_RUNTIME_GRAPHICS_IDENTITY",
):
    require(settings, needle, "runtime GPU/RHI identity")
require(build, '"RHI"', "RHI module dependency")

# Focused launcher must demand renderer identity in the same gameplay log as spawn/weapons/FPS evidence.
for needle in (
    "VERIFY_RUNTIME_GRAPHICS_PASS_16.py",
    "PASS16_RUNTIME_GRAPHICS_IDENTITY",
    "R14_CURRENT_GAMEPLAY.log",
):
    require(launcher, needle, "Pass 16 focused runtime evidence")

print("RUNTIME GRAPHICS PASS 16 SOURCE CONTRACT PASS")
print("- first run applies only safe graphics ceilings; lower existing values are never raised")
print("- the initialization flag persists and manual graphics settings remain authoritative afterwards")
print("- graphics UI still controls full UGameUserSettings scalability after first-run initialization")
print("- gameplay log records primary GPU brand + actual dynamic RHI name")
print("STATUS: SOURCE CONTRACT ONLY; UE 5.8 compile and measured runtime still required")
