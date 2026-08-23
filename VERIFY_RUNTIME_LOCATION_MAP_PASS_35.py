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


museum_guard_h = read(SRC / "Public" / "OCMuseumCoreRecoverySubsystem.h")
museum_guard = read(SRC / "Private" / "OCMuseumCoreRecoverySubsystem.cpp")
map_guard_h = read(SRC / "Public" / "OCTacticalMapPlayerMarkerGuardSubsystem.h")
map_guard = read(SRC / "Private" / "OCTacticalMapPlayerMarkerGuardSubsystem.cpp")
r137 = read(SRC / "Private" / "OCR137MuseumPhotoModelSubsystem.cpp")
r138 = read(SRC / "Private" / "OCR138MuseumInteractiveArchitectureSubsystem.cpp")
spawn = read(SRC / "Private" / "OCTeamSpawnPoint.cpp")
tactical = read(SRC / "Private" / "OCTacticalMapSubsystem.cpp")

require(museum_guard_h, "UOCMuseumCoreRecoverySubsystem", "museum recovery class")
for needle in (
    'MuseumPrototypeTag(TEXT("R137_MuseumPhotoModel"))',
    'MuseumArchitectureTag(TEXT("R138_MuseumHighFidelityArchitecture"))',
    'PASS35_MUSEUM_OWNER_CARRIER_RECOVERED',
    'RunAuthoritativeUpgradeNow(*World)',
    'PASS35_MUSEUM_DETAIL_REPLAY_COMPLETE',
    'PASS35_MUSEUM_CORE_READY',
    'PASS35_MUSEUM_CORE_FAIL',
    'PASS35_MUSEUM_BASE_DISTANCE_READY',
    'MuseumNoSpawnRadiusCm = 3000.0f',
    'MuseumNearbyBaseRadiusCm = 6000.0f',
    'Roof_Both_Ends_4m.Roof_Both_Ends_4m',
    'PASS35_MUSEUM_RECOVERY_PRESENTATION_READY roof=authored_asset',
    'PASS35_MUSEUM_RECOVERY_PRESENTATION_READY roof=fallback_slabs',
    'PASS35Museum_RecoveryPlinth',
):
    require(museum_guard, needle, "museum presence recovery")

# Pass 35 still owns the optional-asset owner/core recovery. Pass 37 adds a stronger visible-structure
# guard because the latest runtime proved that actor tags alone can pass while the site is visually empty.
require(r137, "if (!Cube || !Basic || !RoofMesh) return;", "known R13.7 optional-asset failure path")
require(r138, 'TEXT("R138_MuseumHighFidelityArchitecture")', "R13.8 architecture ownership")
require(r138, "PASS30_MUSEUM_SPECULATIVE_INTERIOR_REMOVED", "museum interior cleanup regression")

# Pass 37 supersedes only the BASE distance presentation: primary BASE is now ~27.8 m from the anchor,
# still outside the authored shell, and faces the museum directly. Pass 35 map-marker ownership remains.
for needle in (
    "FVector(-1400.0f, -2400.0f, 120.0f)",
    "FVector(1400.0f, -2400.0f, 120.0f)",
    "PASS37_RUNTIME_BASE_RACK_NEAR_MUSEUM",
):
    require(spawn, needle, "Pass 37 near-museum BASE compatibility")

require(map_guard_h, "UOCTacticalMapPlayerMarkerGuardSubsystem", "map marker guard class")
for needle in (
    'GetWidgetFromName(TEXT("TacticalMapPlayerMarker"))',
    "Slot->SetZOrder(60)",
    "Font.Size = 26",
    "PASS35_TACTICAL_PLAYER_MARKER_FOREGROUND",
):
    require(map_guard, needle, "foreground player marker repair")

# The underlying map still owns projection/position; Pass 35 only fixes presentation priority.
require(tactical, 'TEXT("TacticalMapPlayerMarker")', "canonical tactical player marker")
require(tactical, "MarkerSlot->SetPosition(WorldToMap(Location))", "player map projection update")
require(tactical, "FVector2D(0.5f, 0.5f), 22", "objective marker z-order evidence")

print("RUNTIME LOCATION + MAP PASS 35 SOURCE CONTRACT PASS")
print("- Pass 35 optional-asset museum owner/core recovery remains present")
print("- Pass 37 supersedes the old 30-60 m BASE presentation with a closer visible approach")
print("- R13.8 remains the authoritative enterable museum core")
print("- Tactical Map player marker is forced above objective/POI labels without duplicating map projection ownership")
print("STATUS: SOURCE VERIFIED; local UE 5.8 runtime remains required")
