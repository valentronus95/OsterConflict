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


def require_absent(path: Path, label: str) -> None:
    if path.exists():
        raise SystemExit(f"PASS35 VERIFY FAIL: stale {label} resurrected: {path.relative_to(ROOT)}")


# Pass 45 supersedes the old Pass 35 Museum recovery owner. It used a delayed carrier/rebuild path that
# could create a second visible Museum and replay later detail stages after the current startup owner.
for path, label in (
    (SRC / "Public" / "OCMuseumCoreRecoverySubsystem.h", "Museum core recovery header"),
    (SRC / "Private" / "OCMuseumCoreRecoverySubsystem.cpp", "Museum core recovery source"),
):
    require_absent(path, label)

map_guard_h = read(SRC / "Public" / "OCTacticalMapPlayerMarkerGuardSubsystem.h")
map_guard = read(SRC / "Private" / "OCTacticalMapPlayerMarkerGuardSubsystem.cpp")
r137 = read(SRC / "Private" / "OCR137MuseumPhotoModelSubsystem.cpp")
r138 = read(SRC / "Private" / "OCR138MuseumInteractiveArchitectureSubsystem.cpp")
startup = read(SRC / "Private" / "OCLandmarkStartupCoordinatorSubsystem.cpp")
spawn = read(SRC / "Private" / "OCTeamSpawnPoint.cpp")
tactical = read(SRC / "Private" / "OCTacticalMapSubsystem.cpp")

# Museum startup now uses the explicit coordinator instead of a second delayed recovery owner.
for needle in (
    "UOCR137MuseumPhotoModelSubsystem",
    "UOCR138MuseumInteractiveArchitectureSubsystem",
    "Timers.ClearAllTimersForObject(Stage)",
    "Stage->RunAuthoritativeBuildNow(World)",
    "Stage->RunAuthoritativeUpgradeNow(World)",
):
    require(startup, needle, "single startup window Museum coordination")
require(r137, 'TEXT("R137_MuseumPhotoModel")', "Museum exterior owner")
require(r138, 'TEXT("R138_MuseumHighFidelityArchitecture")', "Museum interaction architecture")
require(r138, "PASS30_MUSEUM_SPECULATIVE_INTERIOR_REMOVED", "museum interior cleanup regression")

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

# Tactical map owns projection/position; Pass 35 only keeps its foreground presentation evidence.
require(tactical, 'TEXT("TacticalMapPlayerMarker")', "canonical tactical player marker")
require(tactical, "MarkerSlot->SetPosition(WorldToMap(Location))", "player map projection update")
require(tactical, "FVector2D(0.5f, 0.5f), 22", "objective marker z-order evidence")

print("RUNTIME LOCATION + MAP PASS 35 FORWARD-PORTED SOURCE CONTRACT PASS")
print("- stale Pass35 Museum recovery carrier/rebuild owner is physically retired")
print("- Museum startup is coordinated through the current R13.7/R13.8 startup window")
print("- Tactical Map foreground player marker ownership remains intact")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 runtime remains required")
