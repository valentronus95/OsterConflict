#!/usr/bin/env python3
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private"
SETTINGS = SRC / "OCPlayerUserSettings.cpp"
UI = SRC / "OCGameUIRootWidget.cpp"
LAUNCHER = ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd"
PASS23 = ROOT / "VERIFY_DX11_SM5_RENDER_TARGET_PASS_23.py"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS43 VERIFY FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        raise SystemExit(f"PASS43 VERIFY FAIL: {label}: missing {needle!r}")


def forbid(text: str, needle: str, label: str) -> None:
    if needle in text:
        raise SystemExit(f"PASS43 VERIFY FAIL: {label}: forbidden {needle!r}")


settings = read(SETTINGS)
ui = read(UI)
launcher = read(LAUNCHER)
pass23 = read(PASS23)

# Reproduce the exact startup call chain that was active in the reported RenderTargetPool/SlateRHIRenderer crash.
native_construct = re.search(
    r"void\s+UOCGameUIRootWidget::NativeConstruct\(\)\s*\{(?P<body>.*?)\n\}", ui, re.S
)
if not native_construct:
    raise SystemExit("PASS43 VERIFY FAIL: could not isolate UOCGameUIRootWidget::NativeConstruct")
require(native_construct.group("body"), "SyncSettingsWidgetsFromBackend();", "frontend startup settings sync")

sync = re.search(
    r"void\s+UOCGameUIRootWidget::SyncSettingsWidgetsFromBackend\(\)\s*\{(?P<body>.*?)\n\}\n\nvoid\s+UOCGameUIRootWidget::ApplySettingsWidgets",
    ui,
    re.S,
)
if not sync:
    raise SystemExit("PASS43 VERIFY FAIL: could not isolate SyncSettingsWidgetsFromBackend")
require(sync.group("body"), "UOCPlayerUserSettings::Get()", "startup settings sync reaches player settings")

get_match = re.search(
    r"UOCPlayerUserSettings\*\s+UOCPlayerUserSettings::Get\(\)\s*\{(?P<body>.*?)\n\}", settings, re.S
)
if not get_match:
    raise SystemExit("PASS43 VERIFY FAIL: could not isolate UOCPlayerUserSettings::Get")
require(get_match.group("body"), "EnsureInitialGraphicsProfile();", "Get reaches automatic graphics migration")

ensure = re.search(
    r"void\s+UOCPlayerUserSettings::EnsureInitialGraphicsProfile\(\)\s*\{(?P<body>.*?)\n\}\n\nvoid\s+UOCPlayerUserSettings::SavePlayerSettings",
    settings,
    re.S,
)
if not ensure:
    raise SystemExit("PASS43 VERIFY FAIL: could not isolate EnsureInitialGraphicsProfile")
ensure_body = ensure.group("body")

# Critical fix: startup migration is persistence-only. A live UGameUserSettings::ApplySettings here can
# resize/recreate the game viewport while SlateRHIRenderer is still constructing its RHI render target.
forbid(ensure_body, "ApplySettings(", "automatic graphics migration must not mutate live viewport/backbuffer")
require(ensure_body, "GameSettings->SaveSettings();", "automatic profile persistence")
require(ensure_body, "PASS43_STARTUP_GRAPHICS_PERSIST_ONLY_READY", "Pass 43 runtime evidence marker")
require(ensure_body, "live_apply=0 slate_construction_safe=1", "Pass 43 marker payload")

# Manual graphics changes remain live and explicit after the UI/viewport is established.
manual = re.search(
    r"void\s+UOCGameUIRootWidget::ApplySettingsWidgets\(bool bCloseAfterApply\)\s*\{(?P<body>.*?)\n\}\n\nvoid\s+UOCGameUIRootWidget::CancelPendingSettings",
    ui,
    re.S,
)
if not manual:
    raise SystemExit("PASS43 VERIFY FAIL: could not isolate ApplySettingsWidgets")
require(manual.group("body"), "GU->ApplySettings(false);", "explicit settings UI live apply")
require(manual.group("body"), "GU->ConfirmVideoMode();", "explicit settings UI video mode confirmation")

# Keep the already-established DX11/SM5/no-HDR/no-RHI-thread startup isolation from Pass 23.
for token in ("-d3d11", "-sm5", "-nohdr", "-norhithread"):
    require(launcher, token, f"normal launcher safe renderer flag {token}")
require(pass23, "DefaultGraphicsRHI=DefaultGraphicsRHI_DX11", "Pass 23 DX11 contract")
require(pass23, "+D3D11TargetedShaderFormats=PCD3D_SM5", "Pass 23 SM5 contract")

# Do not silently turn the weapon preflight into the suspected game process. It stays an isolated NullRHI
# commandlet and the actual frontend launches only after that process exits successfully.
require(launcher, "-run=pythonscript", "isolated weapon preflight")
require(launcher, "-nullrhi", "weapon preflight NullRHI isolation")
require(launcher, "[4/4] Launching CURRENT NORMAL GAME frontend", "normal frontend starts after preflight")

print("SLATE RENDER TARGET STARTUP PASS 43 SOURCE CONTRACT PASS")
print("- reported crash is localized to the frontend startup settings -> Slate/backbuffer lifecycle")
print("- automatic graphics migrations no longer call live ApplySettings during NativeConstruct")
print("- automatic profile changes are persisted for safe boot-time application")
print("- explicit user graphics Apply remains available after the viewport is initialized")
print("- DX11 + SM5 + no HDR + no RHI thread startup isolation remains intact")
print("STATUS: CODED_UNTESTED; the next local UE 5.8 normal-game launch is authoritative")
