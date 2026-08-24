#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS37 VERIFY FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        raise SystemExit(f"PASS37 VERIFY FAIL: {label}: missing {needle!r}")


def forbid(text: str, needle: str, label: str) -> None:
    if needle in text:
        raise SystemExit(f"PASS37 VERIFY FAIL: {label}: forbidden {needle!r}")


spawn = read(SRC / "Private" / "OCTeamSpawnPoint.cpp")
spawn_guard = read(SRC / "Private" / "OCMuseumSpawnGuardSubsystem.cpp")
museum_h = read(SRC / "Public" / "OCMuseumVisibilityPass37Subsystem.h")
museum = read(SRC / "Private" / "OCMuseumVisibilityPass37Subsystem.cpp")
palette_h = read(SRC / "Public" / "OCWeaponPalettePass37Subsystem.h")
palette = read(SRC / "Private" / "OCWeaponPalettePass37Subsystem.cpp")
acceptance = read(ROOT / "RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd")

for needle in (
    "FVector(-1400.0f, -2400.0f, 120.0f)",
    "FVector(1400.0f, -2400.0f, 120.0f)",
    "RequiredRackWeaponCount = 11",
    "PASS37_BASE_RELOCATED_VISIBLE_MUSEUM_APPROACH",
    "PASS37_RUNTIME_BASE_RACK_NEAR_MUSEUM",
):
    require(spawn, needle, "canonical museum BASE/rack")

for needle in (
    "MuseumNoSpawnRadiusCm = 2000.0f",
    "PrimaryBaseRadiusCm = 4500.0f",
    "BaseDeploymentAcceptanceRadiusCm = 5000.0f",
    "PASS37_MUSEUM_VISIBLE_BASES_READY",
    "PASS37_BASE_DEPLOYMENT_VISIBLE_MUSEUM_APPROACH",
    "PASS37_BASE_DEPLOYMENT_RECOVERY_FAIL",
):
    require(spawn_guard, needle, "legacy spawn guard kept as secondary evidence")

require(museum_h, "UOCMuseumVisibilityPass37Subsystem", "visible museum guard class")
for needle in (
    "MinVisibleStructuralComponents = 12",
    "FirstPollDelaySeconds = 1.45f",
    "PollIntervalSeconds = 0.35f",
    "LateStartupSettleSeconds = 2.20f",
    "MaxRebuildAttempts = 1",
    "PASS37_MUSEUM_VISIBLE_CORE_READY",
    "PASS37_MUSEUM_VISIBLE_CORE_FAIL",
    "PASS38_MUSEUM_REBUILD_BUDGET_READY",
    "PASS42_MUSEUM_EARLY_VISIBILITY_READY",
):
    require(museum_h + museum, needle, "bounded visible museum proof")

# Pass 44 supersedes every runtime palette-recovery rule. The old Pass 37 forced palette and the later
# placeholder-only BasicShapeMaterial repair both produced fake grey/orange presentation. Keep the class only
# as a compatibility shell; authored material truth is now audited elsewhere.
for needle in (
    "compatibility shell",
    "performs no polling",
    "no material creation",
    "no SetMaterial calls",
    "PASS44_WEAPON_PALETTE_MUTATION_DISABLED",
    "runtime_material_creation=0",
    "set_material_calls=0",
    "polling=0",
    "PASS38_WEAPON_PALETTE_SCAN_STOPPED reason=retired_by_pass44",
):
    require(palette_h + palette, needle, "retired runtime palette mutation")

for forbidden in (
    "BasicShapeMaterial.BasicShapeMaterial",
    "UMaterialInstanceDynamic::Create",
    "ResolvePaletteColor",
    "ApplyPalette(",
    "SetMaterial(",
    "SetTimer(",
    "PASS37_WEAPON_VISIBLE_PALETTE_APPLIED",
):
    forbid(palette_h + palette, forbidden, "obsolete palette mutation must not survive Pass 44")

for marker in (
    "PASS44_ACTUAL_PAWN_MUSEUM_BASE_READY",
    "PASS37_MUSEUM_VISIBLE_CORE_READY",
    "PASS37_MUSEUM_VISIBLE_BASES_READY",
    "PASS42_BASE_RACK_GROUNDED_READY",
    "PASS44_WEAPON_PALETTE_MUTATION_DISABLED",
    "PASS38_WEAPON_PALETTE_SCAN_STOPPED",
    "PASS14_PERF_30FPS_READY",
):
    require(acceptance, marker, f"runtime acceptance marker {marker}")

print("VISIBLE MUSEUM + RETIRED WEAPON PALETTE PASS 37/38/42/44 SOURCE CONTRACT PASS")
print("- legacy Museum BASE markers remain secondary evidence")
print("- actual pawn Museum BASE proof is required by Pass 44 acceptance")
print("- museum destructive recovery remains bounded to one attempt")
print("- all BasicShapeMaterial palette mutation is retired; missing authored materials remain fail-visible")
print("- palette subsystem performs no polling and no material writes")
print("STATUS: SOURCE VERIFIED; actual UE 5.8 visual/runtime acceptance remains required")
