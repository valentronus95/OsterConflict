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


museum_h = read(SRC / "Public" / "OCMuseumVisibilityPass37Subsystem.h")
museum = read(SRC / "Private" / "OCMuseumVisibilityPass37Subsystem.cpp")
fallback_h = read(SRC / "Public" / "OCRealWeaponFallbackSubsystem.h")
fallback = read(SRC / "Private" / "OCRealWeaponFallbackSubsystem.cpp")
palette_h = read(SRC / "Public" / "OCWeaponPalettePass37Subsystem.h")
palette = read(SRC / "Private" / "OCWeaponPalettePass37Subsystem.cpp")
game_h = read(SRC / "Public" / "OCGameMode.h")
runtime_safe = read(SRC / "Private" / "OCGameModeRuntimeSafe.cpp")
acceptance = read(ROOT / "RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd")

for needle in (
    "int32 RebuildAttemptCount = 0",
    "MaxRebuildAttempts = 1",
    "RebuildAttemptCount < MaxRebuildAttempts",
    "PASS38_MUSEUM_SINGLE_REBUILD_EXECUTED",
    "PASS38_MUSEUM_REBUILD_BUDGET_READY",
    "PASS38_MUSEUM_REBUILD_BUDGET_FAIL",
    "destructive_loop=0",
):
    require(museum_h + museum, needle, "single museum rebuild budget")

# Pass 44 keeps the real-mesh fallback finite and turns missing authored materials into a terminal audited state.
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

# Pass 44 fully retires the separate palette scan. There must be no timer, material creation or SetMaterial path.
for needle in (
    "compatibility shell",
    "PASS44_WEAPON_PALETTE_MUTATION_DISABLED",
    "PASS38_WEAPON_PALETTE_SCAN_STOPPED reason=retired_by_pass44",
    "polling=0",
):
    require(palette_h + palette, needle, "retired palette scan")
for forbidden in (
    "SetTimer(",
    "UMaterialInstanceDynamic::Create",
    "BasicShapeMaterial.BasicShapeMaterial",
    "SetMaterial(",
):
    forbid(palette_h + palette, forbidden, "no legacy palette runtime work")

# User runtime showed a delayed catastrophic FPS collapse. Normal local game must not silently create filler AI.
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

for marker in (
    "PASS38_MUSEUM_REBUILD_BUDGET_READY",
    "PASS38_WEAPON_FALLBACK_SCAN_STOPPED",
    "PASS38_WEAPON_PALETTE_SCAN_STOPPED",
    "PASS44_LOCAL_BOT_AUTOFILL_DISABLED_READY",
    "PASS14_PERF_30FPS_READY",
):
    require(acceptance, marker, f"runtime acceptance marker {marker}")

print("RUNTIME RUNAWAY / HEAT PASS 38/44 SOURCE CONTRACT PASS")
print("- museum architecture destructive recovery is capped at one rebuild")
print("- weapon fallback/material audit is finite and missing materials become terminal fail-visible evidence")
print("- obsolete palette scan is retired completely: zero timer, zero material writes")
print("- normal local game defaults to zero filler bots unless bot options are explicitly requested")
print("- runtime acceptance still requires >=30 FPS")
print("STATUS: CODED_UNTESTED; local UE 5.8 runtime remains authoritative")
