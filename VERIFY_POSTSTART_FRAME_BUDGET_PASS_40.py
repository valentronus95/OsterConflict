#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS40 VERIFY FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        raise SystemExit(f"PASS40 VERIFY FAIL: {label}: missing {needle!r}")


def forbid(text: str, needle: str, label: str) -> None:
    if needle in text:
        raise SystemExit(f"PASS40 VERIFY FAIL: {label}: forbidden {needle!r}")


viewport_h = read(SRC / "Public" / "OCR13UIViewportStabilizerSubsystem.h")
viewport = read(SRC / "Private" / "OCR13UIViewportStabilizerSubsystem.cpp")
deployment_h = read(SRC / "Public" / "OCR13DeploymentPresentationSubsystem.h")
deployment = read(SRC / "Private" / "OCR13DeploymentPresentationSubsystem.cpp")
launcher = read(ROOT / "RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd")

# The old viewport stabilizer globally searched UObject space and rewrote deployment/menu Slate layout every frame.
# Pass 40 keeps the safety behavior but reduces observation to 10 Hz, caches the root and only mutates structure on state transitions.
for needle in (
    "TWeakObjectPtr<UOCGameUIRootWidget> CachedRoot;",
    "TWeakObjectPtr<AOCPlayerController> CachedController;",
    "float UpdateAccumulator = 0.0f;",
    "bool bDeploymentStabilized = false;",
    "UOCGameUIRootWidget* ResolveRoot(UWorld* World, AOCPlayerController* PC);",
    "constexpr float ViewportStabilizerIntervalSeconds = 0.10f;",
    "if (UpdateAccumulator < ViewportStabilizerIntervalSeconds) return;",
    "if (!bDeploymentStabilized || (bDeploymentVisible && !bLastDeploymentVisible))",
    "if (bStartupShell != bStartupIsolationActive)",
    "if (bStartupIsolationActive) return;",
    "PASS40_UI_STABILIZER_BUDGET_READY",
    "root_scan=cache_miss",
    "layout_writes=transition_only",
    "startup_visibility_writes=transition_only",
):
    require(viewport_h + viewport, needle, "viewport stabilizer budget")

viewport_tick_start = viewport.find("void UOCR13UIViewportStabilizerSubsystem::Tick")
viewport_resolve_start = viewport.find("UOCGameUIRootWidget* UOCR13UIViewportStabilizerSubsystem::ResolveRoot")
if viewport_tick_start < 0 or viewport_resolve_start <= viewport_tick_start:
    raise SystemExit("PASS40 VERIFY FAIL: could not isolate viewport Tick/ResolveRoot")
forbid(viewport[viewport_tick_start:viewport_resolve_start], "TObjectIterator<UOCGameUIRootWidget>",
       "viewport Tick must not globally scan UI roots")
if viewport.count("TObjectIterator<UOCGameUIRootWidget>") != 1:
    raise SystemExit("PASS40 VERIFY FAIL: viewport root global scan must exist only in the cache-miss resolver")

# Deployment presentation had the same root scan plus repeated SetVisibility calls every frame.
for needle in (
    "TWeakObjectPtr<AOCPlayerController> ActiveController;",
    "float UpdateAccumulator = 0.0f;",
    "bool bPresentationVisibilityValid = false;",
    "bool bLastPresentationVisible = false;",
    "UOCGameUIRootWidget* ResolveRoot(UWorld* World, AOCPlayerController* PC);",
    "constexpr float DeploymentPresentationIntervalSeconds = 0.10f;",
    "if (UpdateAccumulator < DeploymentPresentationIntervalSeconds) return;",
    "if (StyledFlowPanel.IsValid() && BackdropBlur.IsValid() && BackdropShade.IsValid() && bStyleApplied) return;",
    "if (bPresentationVisibilityValid && bLastPresentationVisible == bVisible) return;",
    "PASS40_DEPLOYMENT_PRESENTATION_BUDGET_READY",
    "root_scan=cache_miss",
    "visibility_writes=deduped",
    "style_writes=once_per_root",
):
    require(deployment_h + deployment, needle, "deployment presentation budget")

deployment_tick_start = deployment.find("void UOCR13DeploymentPresentationSubsystem::Tick")
deployment_resolve_start = deployment.find("UOCGameUIRootWidget* UOCR13DeploymentPresentationSubsystem::ResolveRoot")
if deployment_tick_start < 0 or deployment_resolve_start <= deployment_tick_start:
    raise SystemExit("PASS40 VERIFY FAIL: could not isolate deployment Tick/ResolveRoot")
forbid(deployment[deployment_tick_start:deployment_resolve_start], "TObjectIterator<UOCGameUIRootWidget>",
       "deployment Tick must not globally scan UI roots")
if deployment.count("TObjectIterator<UOCGameUIRootWidget>") != 1:
    raise SystemExit("PASS40 VERIFY FAIL: deployment root global scan must exist only in the cache-miss resolver")

# Runtime acceptance must prove both budgets while preserving every earlier visual/FPS gate.
for marker in (
    "PASS40_UI_STABILIZER_BUDGET_READY",
    "PASS40_DEPLOYMENT_PRESENTATION_BUDGET_READY",
    "PASS39_GRAPHICS_QUALITY_PROFILE_READY",
    "PASS39_PERF_SAMPLER_IDLE_READY",
    "PASS38_MUSEUM_REBUILD_BUDGET_READY",
    "PASS14_PERF_30FPS_READY",
):
    require(launcher, marker, f"runtime acceptance marker {marker}")
require(launcher, "PASS 29-40 RUNTIME ACCEPTANCE", "Pass 40 runtime acceptance title")
require(launcher, "30 FPS acceptance target", "30 FPS floor remains unchanged")

print("POST-START FRAME BUDGET PASS 40 SOURCE CONTRACT PASS")
print("- viewport stabilizer no longer scans all UObjects or rewrites Slate layout every rendered frame")
print("- deployment presentation caches its root and deduplicates visibility/style writes")
print("- both UI guards run at 10 Hz instead of render-frame cadence")
print("- full runtime acceptance requires Pass 40 markers and still requires >=30 FPS")
print("STATUS: CODED_UNTESTED; local UE 5.8 UI/playflow/FPS/thermal runtime remains authoritative")
