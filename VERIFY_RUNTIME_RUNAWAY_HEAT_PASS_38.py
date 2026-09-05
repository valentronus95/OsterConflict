#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS38 VERIFY FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        raise SystemExit(f"PASS38 VERIFY FAIL: {label}: missing {needle!r}")


def forbid(text: str, needle: str, label: str) -> None:
    if needle in text:
        raise SystemExit(f"PASS38 VERIFY FAIL: {label}: forbidden {needle!r}")


def absent(path: Path, label: str) -> None:
    if path.exists():
        raise SystemExit(f"PASS38 VERIFY FAIL: stale {label} resurrected: {path.relative_to(ROOT)}")


# Pass 45 removes the old Museum destructive recovery entirely instead of budgeting one rebuild.
for path, label in (
    (SRC / "Public" / "OCMuseumVisibilityPass37Subsystem.h", "Museum visibility/rebuild guard"),
    (SRC / "Private" / "OCMuseumVisibilityPass37Subsystem.cpp", "Museum visibility/rebuild guard"),
    (SRC / "Public" / "OCMuseumCoreRecoverySubsystem.h", "Museum core recovery owner"),
    (SRC / "Private" / "OCMuseumCoreRecoverySubsystem.cpp", "Museum core recovery owner"),
    (SRC / "Public" / "OCWeaponPalettePass37Subsystem.h", "weapon palette compatibility owner"),
    (SRC / "Private" / "OCWeaponPalettePass37Subsystem.cpp", "weapon palette compatibility owner"),
):
    absent(path, label)

fallback_h = read(SRC / "Public" / "OCRealWeaponFallbackSubsystem.h")
fallback = read(SRC / "Private" / "OCRealWeaponFallbackSubsystem.cpp")
game_h = read(SRC / "Public" / "OCGameMode.h")
runtime_safe = read(SRC / "Private" / "OCGameModeRuntimeSafe.cpp")
startup = read(SRC / "Private" / "OCLandmarkStartupCoordinatorSubsystem.cpp")
perf_h = read(SRC / "Public" / "OCPerformanceSampleSubsystem.h")
perf = read(SRC / "Private" / "OCPerformanceSampleSubsystem.cpp")
launcher = read(ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd")
acceptance = read(ROOT / "RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd")
evidence = read(ROOT / "VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py")

# Current Museum startup has one coordinated startup window and explicitly reports no old recovery owners.
for needle in (
    "PASS45_LANDMARK_STARTUP_COORDINATED_READY",
    "delayed_stage_timers_cancelled=1",
    "legacy_core_recovery=0",
    "destructive_visibility_rebuild=0",
):
    require(startup, needle, "coordinated landmark startup")

# Real-mesh fallback/material audit remains finite and truth-only.
for needle in (
    "int32 RefreshPassCount = 0",
    "MaxRefreshPasses = 12",
    "ClearTimer(RefreshTimer)",
    "PASS44_WEAPON_RACK_AUTHORED_MATERIAL_GAP",
    "reason=material_gap_audited",
    "PASS38_WEAPON_FALLBACK_SCAN_STOPPED",
    "PASS38_WEAPON_FALLBACK_SCAN_BOUNDED_STOP",
    "permanent_scan=0",
):
    require(fallback_h + fallback, needle, "bounded truth-only weapon scan")
forbid(fallback, "UMaterialInstanceDynamic::Create", "weapon fallback must not fabricate material recovery")
forbid(fallback, "Component->SetMaterial(Slot", "weapon audit must not repaint slots")

# Normal local game must not silently create filler AI.
for needle in (
    "int32 TargetPopulation = 0",
    "bool bAutoFillBots = false",
):
    require(game_h, needle, "safe base population defaults")
for needle in (
    "PASS44_LOCAL_BOT_AUTOFILL_DISABLED_READY",
    "TargetPopulation = 0",
    "bAutoFillBots = false",
    "background_ai_load=0",
):
    require(runtime_safe, needle, "runtime-safe local bot suppression")

# Pass45 thermal recovery must be a real UE runtime contract, not just the string `t.MaxFPS 60` in a batch file.
for needle in (
    "bRecoveryRuntimeContractLogged",
    "ValidatePass45RecoveryRuntimeContract",
):
    require(perf_h, needle, "thermal/display sampler state")
for needle in (
    '#include "HAL/IConsoleManager.h"',
    'FindConsoleVariable(TEXT("t.MaxFPS"))',
    'MaxFpsVariable->GetFloat()',
    'FMath::IsNearlyEqual(RuntimeMaxFps, 60.0f, 0.5f)',
    'PASS45_THERMAL_CAP_RUNTIME_READY',
    'PASS45_THERMAL_CAP_RUNTIME_FAIL',
    'quality_mutation=0 render_scale_mutation=0',
):
    require(perf, needle, "actual runtime thermal cap evidence")
launcher_parts = launcher.split(":quick_normal_game", 1)
if len(launcher_parts) != 2:
    raise SystemExit("PASS38 VERIFY FAIL: canonical launcher is missing explicit quick-normal section")
strict_launcher, quick_launcher = launcher_parts
for needle in ('-fullscreen', 't.MaxFPS 60'):
    require(strict_launcher, needle, "strict recovery launcher request")
forbid(strict_launcher, '-windowed', "strict recovery route must not force windowed mode")
require(quick_launcher, '-windowed', "quick normal route must remain desktop-recoverable")
require(quick_launcher, 't.MaxFPS 60', "quick normal route must retain 60 FPS thermal cap")

# Strict evidence must reject a run where the CVar request was overridden or never applied.
for needle in (
    'require(gameplay, "PASS45_THERMAL_CAP_RUNTIME_READY"',
    'forbid(gameplay, "PASS45_THERMAL_CAP_RUNTIME_FAIL"',
    '"THERMAL_CAP_RUNTIME_CONTRACT=PASS\\n"',
):
    require(evidence, needle, "strict thermal runtime evidence")

# Acceptance must no longer demand logs from physically deleted recovery/palette owners.
for marker in (
    "PASS38_MUSEUM_REBUILD_BUDGET_READY",
    "PASS38_MUSEUM_REBUILD_BUDGET_FAIL",
    "PASS38_WEAPON_PALETTE_SCAN_STOPPED",
    "PASS44_WEAPON_PALETTE_MUTATION_DISABLED",
    "PASS37_MUSEUM_VISIBLE_CORE_READY",
):
    forbid(acceptance, marker, f"stale acceptance marker {marker}")
for marker in (
    "PASS45_LANDMARK_STARTUP_COORDINATED_READY",
    "PASS38_WEAPON_FALLBACK_SCAN_STOPPED",
    "PASS44_LOCAL_BOT_AUTOFILL_DISABLED_READY",
    "PASS14_PERF_30FPS_READY",
):
    require(acceptance, marker, f"current runtime acceptance marker {marker}")

print("RUNTIME RUNAWAY / HEAT PASS 38/45 FORWARD-PORTED SOURCE CONTRACT PASS")
print("- destructive Museum recovery is physically deleted, not merely capped")
print("- obsolete palette owner is physically deleted")
print("- landmark startup is coordinated once and historical delayed stage timers are cancelled")
print("- weapon fallback/material audit remains finite and fail-visible")
print("- normal local game defaults to zero filler bots unless explicitly requested")
print("- strict recovery remains fullscreen; quick normal is windowed only so a broken startup cannot trap the desktop")
print("- both launch modes retain the 60 FPS cap, and UE runtime must confirm actual t.MaxFPS=60 with fail-visible evidence")
print("- low-FPS/thermal recovery never lowers render scale to disguise the problem")
print("STATUS: CODED_UNTESTED; local UE 5.8 runtime remains authoritative")
