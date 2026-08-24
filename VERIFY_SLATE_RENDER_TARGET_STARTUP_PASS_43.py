#!/usr/bin/env python3
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private"
SETTINGS = SRC / "OCPlayerUserSettings.cpp"
UI = SRC / "OCGameUIRootWidget.cpp"
MINIMAP = SRC / "OCMinimapSubsystem.cpp"
TACTICAL = SRC / "OCTacticalMapSubsystem.cpp"
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
minimap = read(MINIMAP)
tactical = read(TACTICAL)
launcher = read(LAUNCHER)
pass23 = read(PASS23)

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
forbid(ensure_body, "GameSettings->ApplySettings(", "automatic graphics migration must not mutate live viewport/backbuffer")
require(ensure_body, "GameSettings->SaveSettings();", "automatic profile persistence")
require(ensure_body, "PASS43_STARTUP_GRAPHICS_PERSIST_ONLY_READY", "Pass 43 startup persistence marker")
require(ensure_body, "live_apply=0 slate_construction_safe=1", "Pass 43 marker payload")

manual = re.search(
    r"void\s+UOCGameUIRootWidget::ApplySettingsWidgets\(bool bCloseAfterApply\)\s*\{(?P<body>.*?)\n\}\n\nvoid\s+UOCGameUIRootWidget::CancelPendingSettings",
    ui,
    re.S,
)
if not manual:
    raise SystemExit("PASS43 VERIFY FAIL: could not isolate ApplySettingsWidgets")
require(manual.group("body"), "GU->ApplySettings(false);", "explicit settings UI live apply")
require(manual.group("body"), "GU->ConfirmVideoMode();", "explicit settings UI video mode confirmation")

ensure_minimap = re.search(
    r"void\s+UOCMinimapSubsystem::EnsureMinimap\(AOCPlayerController& PlayerController\)\s*\{(?P<body>.*?)\n\}\n\nvoid\s+UOCMinimapSubsystem::Tick",
    minimap,
    re.S,
)
if not ensure_minimap:
    raise SystemExit("PASS43 VERIFY FAIL: could not isolate UOCMinimapSubsystem::EnsureMinimap")
ensure_minimap_body = ensure_minimap.group("body")
for needle in (
    "APawn* Pawn = PlayerController.GetPawn();",
    "!Pawn || PlayerController.IsFrontendMenuVisible()",
    "PlayerController.IsDeploymentPanelVisible()",
    "PlayerController.IsSettingsVisible()",
    "if (TacticalMap && TacticalMap->IsMapOpen()) return;",
    "TacticalMap->EnsureMapSnapshot()",
    "PASS43_MINIMAP_RENDER_TARGET_GAMEPLAY_ONLY_READY",
):
    require(ensure_minimap_body, needle, "gameplay-only minimap render target")
if ensure_minimap_body.index("!Pawn || PlayerController.IsFrontendMenuVisible()") > ensure_minimap_body.index("TacticalMap->EnsureMapSnapshot()"):
    raise SystemExit("PASS43 VERIFY FAIL: minimap still creates its render target before no-pawn/frontend rejection")

minimap_tick = re.search(
    r"void\s+UOCMinimapSubsystem::Tick\(float DeltaTime\)\s*\{(?P<body>.*?)\n\}\n\nvoid\s+UOCMinimapSubsystem::Deinitialize",
    minimap,
    re.S,
)
if not minimap_tick:
    raise SystemExit("PASS43 VERIFY FAIL: could not isolate UOCMinimapSubsystem::Tick")
minimap_tick_body = minimap_tick.group("body")
require(minimap_tick_body, "if (bBlocked)", "blocked minimap startup gate")
require(minimap_tick_body, "EnsureMinimap(*PlayerController);", "gameplay minimap creation")
if minimap_tick_body.index("if (bBlocked)") > minimap_tick_body.index("EnsureMinimap(*PlayerController);"):
    raise SystemExit("PASS43 VERIFY FAIL: Tick can still create the minimap render target before blocked-state rejection")

for needle in (
    "MapRenderTarget->UpdateResourceImmediate(true);",
    "CaptureComponent->bCaptureEveryFrame = false;",
    "CaptureComponent->bCaptureOnMovement = false;",
    "CaptureComponent->CaptureScene();",
):
    require(tactical, needle, "one-shot tactical map render target")

# Pass 45 keeps the Pass 43 DX11/SM5/no-HDR isolation but no longer treats -norhithread as a permanent
# normal-game invariant. It is now an explicit compatibility A/B route after the latest menu-at-8-FPS evidence.
for token in ("-d3d11", "-sm5", "-nohdr"):
    require(launcher, token, f"normal launcher renderer flag {token}")
require(launcher, 'set "RHI_FLAGS=-d3d11 -sm5 -nohdr"', "Pass 45 normal RHI-thread route")
require(launcher, 'if /I "%OC_RHI_COMPAT%"=="1"', "Pass 45 explicit compatibility selector")
require(launcher, '-norhithread', "Pass 45 compatibility route retains no-RHI-thread fallback")
require(launcher, 'set "RHI_MODE=dx11_sm5_rhi_thread"', "Pass 45 normal RHI mode marker")
require(launcher, 'set "RHI_MODE=dx11_sm5_no_rhi_thread_compat"', "Pass 45 compatibility RHI mode marker")
require(pass23, "DefaultGraphicsRHI=DefaultGraphicsRHI_DX11", "Pass 23 DX11 contract")
require(pass23, "+D3D11TargetedShaderFormats=PCD3D_SM5", "Pass 23 SM5 contract")

require(launcher, "-run=pythonscript", "isolated weapon preflight")
require(launcher, "-nullrhi", "weapon preflight NullRHI isolation")
require(launcher, "[4/4] Launching CURRENT NORMAL GAME frontend", "normal frontend starts after preflight")

print("SLATE RENDER TARGET STARTUP PASS 43 SOURCE CONTRACT PASS")
print("- startup settings sync no longer performs live renderer ApplySettings during NativeConstruct")
print("- automatic graphics changes are persisted for safe boot-time application")
print("- minimap SceneCapture/render target/Slate brush is not created in frontend, deployment or settings UI")
print("- minimap render target is first allowed only with an actual unblocked gameplay Pawn")
print("- explicit user graphics Apply remains available after viewport initialization")
print("- DX11 + SM5 + no HDR isolation remains; -norhithread is now explicit Pass 45 compatibility A/B only")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 runtime remains authoritative")
