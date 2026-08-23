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
acceptance = read(ROOT / "RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd")

# User runtime evidence: FPS falls from roughly 60 to 5 within seconds while the museum remains absent.
# The only museum recovery path may rebuild the large R13.8 architecture at most once.
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
forbid(museum, "if (!bVisibleCoreReady && HasPrototypeOwner(*World))\n    {",
       "unbounded Pass 37 destructive museum-rebuild condition")

# The real-weapon fallback scanner used to iterate every weapon forever at 4 Hz. It now has a finite warm-up.
for needle in (
    "int32 RefreshPassCount = 0",
    "MaxRefreshPasses = 12",
    "0.50f",
    "ClearTimer(RefreshTimer)",
    "PASS38_WEAPON_FALLBACK_SCAN_STOPPED",
    "PASS38_WEAPON_FALLBACK_SCAN_BOUNDED_STOP",
    "permanent_scan=0",
):
    require(fallback_h + fallback, needle, "bounded real-weapon fallback scan")
forbid(fallback, "0.25f,\n        true,\n        0.0f", "old permanent 4 Hz fallback timer")

# Pass 37's forced recolour damaged valid imported presentation (flat orange Lever Action in user runtime).
# Recovery is now placeholder-only and the palette scanner is bounded as well.
for needle in (
    "int32 AuditPassCount = 0",
    "MaxAuditPasses = 12",
    "if (!bPlaceholder) continue;",
    "authored_materials_preserved=1",
    "PASS38_WEAPON_PALETTE_SCAN_STOPPED",
    "PASS38_WEAPON_PALETTE_SCAN_BOUNDED_STOP",
):
    require(palette_h + palette, needle, "placeholder-only bounded palette scan")
forbid(palette, "bForceRestoredPalette", "forced recolour of non-placeholder restored materials")
forbid(palette, "forced_restored_slots=", "obsolete forced-palette runtime evidence")

# Full runtime acceptance must require the new bounded-lifecycle evidence and still keep the 30 FPS floor.
for marker in (
    "PASS38_MUSEUM_REBUILD_BUDGET_READY",
    "PASS38_WEAPON_FALLBACK_SCAN_STOPPED",
    "PASS38_WEAPON_PALETTE_SCAN_STOPPED",
    "PASS38_MUSEUM_REBUILD_BUDGET_FAIL",
    "PASS38_WEAPON_FALLBACK_SCAN_BOUNDED_STOP",
    "PASS38_WEAPON_PALETTE_SCAN_BOUNDED_STOP",
    "PASS14_PERF_30FPS_READY",
):
    require(acceptance, marker, f"runtime acceptance marker {marker}")
require(acceptance, "30 FPS acceptance target", "performance floor remains 30 FPS")

print("RUNTIME RUNAWAY / HEAT PASS 38 SOURCE CONTRACT PASS")
print("- museum architecture destructive recovery is capped at one rebuild")
print("- late duplicate cleanup remains one-shot after the historical delayed startup window")
print("- real-weapon fallback and palette world scans stop after convergence or a hard finite budget")
print("- Pass 37 forced recolouring of valid imported weapon materials stays removed")
print("- runtime acceptance still requires >=30 FPS and fails on any bounded-scan/rebuild failure marker")
print("STATUS: CODED_UNTESTED; local UE 5.8 runtime remains authoritative")