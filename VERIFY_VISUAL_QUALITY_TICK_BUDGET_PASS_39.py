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

# Old Pass 16 automatic profile visibly degraded the user's playtest. New installs use a balanced ceiling,
# and existing installs migrate only if they still match the recognizable old automatic signature.
for needle in (
    "bPass39GraphicsQualityRecoveryApplied = false",
    "SetViewDistanceQuality(SafeQuality(GameSettings->GetViewDistanceQuality(), 2))",
    "SetShadowQuality(SafeQuality(GameSettings->GetShadowQuality(), 1))",
    "SetTextureQuality(SafeQuality(GameSettings->GetTextureQuality(), 2))",
    "SetFoliageQuality(SafeQuality(GameSettings->GetFoliageQuality(), 1))",
    "SetGlobalIlluminationQuality(SafeQuality(GameSettings->GetGlobalIlluminationQuality(), 1))",
    "SetReflectionQuality(SafeQuality(GameSettings->GetReflectionQuality(), 1))",
    "if (CurrentScale > 85.0f)",
    "const bool bLooksLikeLegacyPass16",
    "CurrentScale <= 75.5f",
    "PASS39_GRAPHICS_QUALITY_RECOVERY_APPLIED",
    "PASS39_GRAPHICS_CUSTOM_PROFILE_PRESERVED",
    "PASS39_GRAPHICS_QUALITY_PROFILE_READY",
):
    require(settings_h + settings, needle, "graphics quality recovery")

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

# Minimap scene capture stays one-shot. Only the marker/visibility UI updates at 10 Hz, with visibility writes deduped.
for needle in (
    "MinimapUpdateIntervalSeconds = 0.10f",
    "UpdateAccumulator",
    "DesiredVisibility != LastWidgetVisibility",
    "PASS39_MINIMAP_UPDATE_BUDGET_READY",
    "hz=10",
):
    require(minimap_h + minimap, needle, "minimap update budget")
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

# Full runtime acceptance must prove the new profile and tick-budget paths and reject stale emergency downgrade code.
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

print("VISUAL QUALITY + TICK BUDGET PASS 39 SOURCE CONTRACT PASS")
print("- old 75% mostly-low automatic graphics profile gets a one-time balanced quality recovery")
print("- low FPS is diagnostic evidence only; no hidden mid-session graphics downgrade survives")
print("- finished performance/foliage guards stop ticking")
print("- minimap Slate updates are capped at 10 Hz while tactical-map capture remains one-shot")
print("- first-person presentation resolves the local pawn directly instead of scanning all characters every frame")
print("- runtime acceptance still requires >=30 FPS")
print("STATUS: CODED_UNTESTED; local UE 5.8 visual/FPS/temperature playtest remains authoritative")