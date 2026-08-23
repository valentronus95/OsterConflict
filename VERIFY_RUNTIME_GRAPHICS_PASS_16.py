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

for needle in ("void EnsureInitialGraphicsProfile();", "bool bInitialGraphicsProfileApplied = false;", "UPROPERTY(Config"):
    require(header, needle, "persistent first-run graphics contract")

get_match = re.search(r"UOCPlayerUserSettings\*\s+UOCPlayerUserSettings::Get\(\)\s*\{(?P<body>.*?)\n\}", cpp, re.S)
if not get_match:
    raise SystemExit("PASS16 VERIFY FAIL: could not locate UOCPlayerUserSettings::Get")
get_body = get_match.group("body")
require(get_body, "ValidateSettingsSchema();", "settings Get schema validation")
require(get_body, "EnsureInitialGraphicsProfile();", "settings Get first-run graphics initialization")
if get_body.index("ValidateSettingsSchema();") > get_body.index("EnsureInitialGraphicsProfile();"):
    raise SystemExit("PASS16 VERIFY FAIL: graphics initialization must run after schema validation")

require(cpp, "if (!bInitialGraphicsProfileApplied)", "one-time graphics guard")
require(cpp, "GameSettings->LoadSettings(false);", "persisted settings load")
if cpp.find("GameSettings->LoadSettings(false);") < cpp.find("if (!bInitialGraphicsProfileApplied)"):
    raise SystemExit("PASS16 VERIFY FAIL: LoadSettings runs outside one-time guard")

for needle in (
    "auto SafeQuality", "FMath::Min(Current, Ceiling)",
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
    "if (CurrentScale > 75.0f)", "SetResolutionScaleValueEx(75.0f)",
    "GameSettings->ApplySettings(false);", "GameSettings->SaveSettings();",
    "bInitialGraphicsProfileApplied = true;", "PASS16_INITIAL_GRAPHICS_PROFILE_APPLIED",
):
    require(cpp, needle, "safe graphics ceiling")

reset_match = re.search(r"void\s+UOCPlayerUserSettings::ResetPlayerDefaults\(\)\s*\{(?P<body>.*?)\n\}", cpp, re.S)
if not reset_match:
    raise SystemExit("PASS16 VERIFY FAIL: could not locate ResetPlayerDefaults")
forbid(reset_match.group("body"), "bInitialGraphicsProfileApplied = false", "Reset Defaults re-arming Pass 16")

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

print("RUNTIME GRAPHICS PASS 16 SOURCE CONTRACT PASS")
print("- safe first-run graphics ceiling and real GPU/RHI evidence remain intact")
print("- Pass 19 extends the focused recovery launcher without weakening Pass 16")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 compile/runtime FPS acceptance still required")
