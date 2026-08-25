#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS35 VERIFY FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        raise SystemExit(f"PASS35 VERIFY FAIL: {label}: missing {needle!r}")


def forbid(text: str, needle: str, label: str) -> None:
    if needle in text:
        raise SystemExit(f"PASS35 VERIFY FAIL: {label}: forbidden {needle!r}")


def require_absent(path: Path, label: str) -> None:
    if path.exists():
        raise SystemExit(f"PASS35 VERIFY FAIL: stale {label} resurrected: {path.relative_to(ROOT)}")


# Pass45 supersedes the old Pass35 Museum recovery owner. Delayed carrier/rebuild paths that can
# create or repair a second visible Museum remain physically retired.
for path, label in (
    (SRC / "Public" / "OCMuseumCoreRecoverySubsystem.h", "Museum core recovery header"),
    (SRC / "Private" / "OCMuseumCoreRecoverySubsystem.cpp", "Museum core recovery source"),
    (SRC / "Public" / "OCMuseumVisibilityPass37Subsystem.h", "Museum visibility rebuild header"),
    (SRC / "Private" / "OCMuseumVisibilityPass37Subsystem.cpp", "Museum visibility rebuild source"),
):
    require_absent(path, label)

map_guard_h = read(SRC / "Public" / "OCTacticalMapPlayerMarkerGuardSubsystem.h")
map_guard = read(SRC / "Private" / "OCTacticalMapPlayerMarkerGuardSubsystem.cpp")
r137 = read(SRC / "Private" / "OCR137MuseumPhotoModelSubsystem.cpp")
r138 = read(SRC / "Private" / "OCR138MuseumInteractiveArchitectureSubsystem.cpp")
layer_guard = read(SRC / "Private" / "OCMuseumLayerPerformanceGuardSubsystem.cpp")
startup = read(SRC / "Private" / "OCLandmarkStartupCoordinatorSubsystem.cpp")
spawn = read(SRC / "Private" / "OCTeamSpawnPoint.cpp")
tactical = read(SRC / "Private" / "OCTacticalMapSubsystem.cpp")

# Current Museum ownership: coordinator builds R13.7 once, then R13.8 hidden interaction collision/final glass.
for needle in (
    "UOCR137MuseumPhotoModelSubsystem",
    "UOCR138MuseumInteractiveArchitectureSubsystem",
    "Timers.ClearAllTimersForObject(Stage)",
    "Stage->RunAuthoritativeBuildNow(World)",
    "Stage->RunAuthoritativeUpgradeNow(World)",
):
    require(startup, needle, "single startup window Museum coordination")
for needle in (
    'TEXT("R137_MuseumPhotoModel")',
    "PASS45_MUSEUM_R137_PRIMARY_EXTERIOR_READY",
    "static_glass=0",
    "prototype_doors=0",
    "prototype_trees=0",
):
    require(r137, needle, "R13.7 single visible Museum exterior")
for needle in (
    'TEXT("R138_MuseumInteractionCollision")',
    "MuseumInteractionCollision",
    "SetVisibility(false, true)",
    "SetHiddenInGame(true, true)",
    "PASS45_MUSEUM_R138_COLLISION_ONLY_READY",
):
    require(r138, needle, "R13.8 hidden Museum interaction owner")
for needle in (
    "PASS45_MUSEUM_LAYER_VALIDATION_READY",
    "PASS45_MUSEUM_LAYER_VALIDATION_FAIL",
    "mutation=0",
    "primary_authoring_fix_required=1",
):
    require(layer_guard, needle, "validation-only Museum layer ownership")
for forbidden in (
    'TEXT("R138_MuseumHighFidelityArchitecture")',
    "PASS30_MUSEUM_SPECULATIVE_INTERIOR_REMOVED",
):
    forbid(r138, forbidden, "retired visible R13.8 compatibility contract")

# Current BASE/rack placement remains close to Museum and no longer depends on the retired edge world.
for needle in (
    "FVector(-1400.0f, -2400.0f, 120.0f)",
    "FVector(1400.0f, -2400.0f, 120.0f)",
    "PASS37_RUNTIME_BASE_RACK_NEAR_MUSEUM",
):
    require(spawn, needle, "near-Museum BASE compatibility")

require(map_guard_h, "UOCTacticalMapPlayerMarkerGuardSubsystem", "map marker guard class")
for needle in (
    'GetWidgetFromName(TEXT("TacticalMapPlayerMarker"))',
    "Slot->SetZOrder(60)",
    "Font.Size = 26",
    "PASS35_TACTICAL_PLAYER_MARKER_FOREGROUND",
):
    require(map_guard, needle, "foreground player marker repair")

# Tactical map owns projection/position; Pass35 only keeps foreground presentation evidence.
require(tactical, 'TEXT("TacticalMapPlayerMarker")', "canonical tactical player marker")
require(tactical, "MarkerSlot->SetPosition(WorldToMap(Location))", "player map projection update")
require(tactical, "FVector2D(0.5f, 0.5f), 22", "objective marker z-order evidence")

print("RUNTIME LOCATION + MAP PASS35/PASS45 SOURCE CONTRACT PASS")
print("- stale Museum recovery/rebuild owners stay physically retired")
print("- R13.7 is the visible Museum exterior; R13.8 is hidden collision/interactivity only")
print("- Museum layer ownership is validated without late repair")
print("- Tactical Map foreground player marker ownership remains intact")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 runtime remains required")
