#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS39 VERIFY FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        raise SystemExit(f"PASS39 VERIFY FAIL: {label}: missing {needle!r}")


def forbid(text: str, needle: str, label: str) -> None:
    if needle in text:
        raise SystemExit(f"PASS39 VERIFY FAIL: {label}: forbidden {needle!r}")

settings_h = read(SRC / "Public" / "OCPlayerUserSettings.h")
settings = read(SRC / "Private" / "OCPlayerUserSettings.cpp")
perf_h = read(SRC / "Public" / "OCPerformanceSampleSubsystem.h")
perf = read(SRC / "Private" / "OCPerformanceSampleSubsystem.cpp")
foliage_h = read(SRC / "Public" / "OCFoliageRuntimeGuardSubsystem.h")
minimap_h = read(SRC / "Public" / "OCMinimapSubsystem.h")
minimap = read(SRC / "Private" / "OCMinimapSubsystem.cpp")
tactical = read(SRC / "Private" / "OCTacticalMapSubsystem.cpp")
fp = read(SRC / "Private" / "OCFirstPersonWeaponPresentationSubsystem.cpp")
acceptance = read(ROOT / "RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd")

# Pass 39 removed the hidden low-quality emergency path. Pass 42 keeps the same conservative expensive
# lighting ceilings but restores native internal resolution and full texture quality for automatic profiles.
for needle in (
    "bPass39GraphicsQualityRecoveryApplied = false",
    "bPass42GraphicsClarityRecoveryApplied = false",
    "SetViewDistanceQuality(SafeQuality(GameSettings->GetViewDistanceQuality(), 2))",
    "SetShadowQuality(SafeQuality(GameSettings->GetShadowQuality(), 1))",
    "SetTextureQuality(SafeQuality(GameSettings->GetTextureQuality(), 3))",
    "SetFoliageQuality(SafeQuality(GameSettings->GetFoliageQuality(), 1))",
    "SetGlobalIlluminationQuality(SafeQuality(GameSettings->GetGlobalIlluminationQuality(), 1))",
    "SetReflectionQuality(SafeQuality(GameSettings->GetReflectionQuality(), 1))",
    "SetResolutionScaleValueEx(100.0f)",
    "const bool bLooksLikeLegacyPass16",
    "CurrentScale <= 75.5f",
    "const bool bLooksLikeAutomaticPass39",
    "CurrentScale <= 85.5f",
    "PASS39_GRAPHICS_QUALITY_RECOVERY_APPLIED",
    "PASS39_GRAPHICS_CUSTOM_PROFILE_PRESERVED",
    "PASS39_GRAPHICS_QUALITY_PROFILE_READY",
    "PASS42_GRAPHICS_CLARITY_RECOVERY_APPLIED",
    "PASS42_GRAPHICS_CUSTOM_PROFILE_PRESERVED",
):
    require(settings_h + settings, needle, "graphics quality recovery")

# Historical Pass 39's 85% migration remains recognizable only as an intermediate one-time legacy migration;
# Pass 42 must upgrade that exact automatic family to native 100% scale + texture quality 3.
require(settings, "GameSettings->SetTextureQuality(3);", "Pass 42 automatic profile texture recovery")
require(settings, "expensive_lighting_unchanged=1", "Pass 42 must not re-enable costly lighting")

# Pass 43 keeps those migrations persistence-only during frontend construction. A live ApplySettings here
# can invalidate Slate's backbuffer/render target before the frontend is fully established.
require(settings, "PASS43_STARTUP_GRAPHICS_PERSIST_ONLY_READY", "Pass 43 startup persistence evidence")

# Low FPS must never trigger another hidden scalability mutation. It is diagnostic evidence only.
for needle in (
    "ReportLowFpsProbe",
    "PASS39_LOW_FPS_PROBE_DIAGNOSTIC",
    "quality_mutation=0",
    "virtual bool IsTickable() const override { return !bFinished; }",
    "PASS39_PERF_SAMPLER_IDLE_READY",
):
    require(perf_h + perf, needle, "diagnostic-only finite performance sampler")
for forbidden in (
    "PASS15_EMERGENCY_PERF_PROFILE_APPLIED",
    "ExecuteConsoleCommand",
    "r.ScreenPercentage 65",
    "r.StaticMeshLODDistanceScale 1.50",
):
    forbid(perf, forbidden, "hidden emergency quality downgrade")

# Completed foliage validation should leave the tick manager entirely, not return from an empty Tick forever.
require(foliage_h, "virtual bool IsTickable() const override { return !bFinished; }",
        "completed foliage guard must stop ticking")

# Minimap scene capture stays one-shot and its marker/visibility UI stays at 10 Hz. Pass 43 also defers
# creation of the render target/Slate brush until an actual unblocked gameplay Pawn exists.
for needle in (
    "MinimapUpdateIntervalSeconds = 0.10f",
    "UpdateAccumulator",
    "LastWidgetVisibility != ESlateVisibility::Collapsed",
    "LastWidgetVisibility != ESlateVisibility::HitTestInvisible",
    "PASS39_MINIMAP_UPDATE_BUDGET_READY",
    "PASS43_MINIMAP_RENDER_TARGET_GAMEPLAY_ONLY_READY",
    "if (bBlocked)",
    "EnsureMinimap(*PlayerController);",
    "hz=10",
):
    require(minimap_h + minimap, needle, "minimap update/render-target budget")
if minimap.index("if (bBlocked)") > minimap.index("EnsureMinimap(*PlayerController);"):
    raise SystemExit("PASS39 VERIFY FAIL: minimap render target can still be created before blocked frontend/deployment state is rejected")
for needle in (
    "bCaptureEveryFrame = false",
    "bCaptureOnMovement = false",
    "CaptureScene();",
):
    require(tactical, needle, "one-shot tactical map capture")

# First-person presentation is local-player state; scanning every character every frame is forbidden.
for needle in (
    "World->GetFirstPlayerController()",
    "Cast<AOCCharacter>(LocalPC->GetPawn())",
    "PASS39_FP_LOCAL_PAWN_FAST_PATH_READY",
    "world_character_scan=0",
):
    require(fp, needle, "local-pawn first-person presentation fast path")
forbid(fp, "TActorIterator<AOCCharacter>", "per-frame world character scan")

# Full runtime acceptance must prove the profile/tick-budget paths and reject stale emergency downgrade code.
for marker in (
    "PASS39_GRAPHICS_QUALITY_PROFILE_READY",
    "PASS39_MINIMAP_UPDATE_BUDGET_READY",
    "PASS39_FP_LOCAL_PAWN_FAST_PATH_READY",
    "PASS39_PERF_SAMPLER_IDLE_READY",
    "PASS14_PERF_30FPS_READY",
):
    require(acceptance, marker, f"runtime acceptance marker {marker}")
require(acceptance, "PASS15_EMERGENCY_PERF_PROFILE_APPLIED", "stale emergency marker rejection")
require(acceptance, "30 FPS acceptance target", "30 FPS floor remains unchanged")

print("VISUAL QUALITY + TICK BUDGET PASS 39/42/43 SOURCE CONTRACT PASS")
print("- automatic clarity recovery remains 100% scale / Texture 3 without expensive-lighting escalation")
print("- automatic graphics migration is persistence-only during frontend Slate construction")
print("- low FPS is diagnostic evidence only; no hidden mid-session graphics downgrade survives")
print("- finished performance/foliage guards stop ticking")
print("- minimap render target is deferred until stable gameplay and Slate updates remain capped at 10 Hz")
print("- tactical-map capture remains one-shot")
print("- first-person presentation resolves the local pawn directly instead of scanning all characters every frame")
print("- runtime acceptance still requires >=30 FPS")
print("STATUS: CODED_UNTESTED; local UE 5.8 visual/FPS/startup/temperature playtest remains authoritative")
