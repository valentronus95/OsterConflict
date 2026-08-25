#!/usr/bin/env python3
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"

GUARD_H = SRC / "Public" / "OCLandmarkShellOwnershipGuardSubsystem.h"
GUARD = SRC / "Private" / "OCLandmarkShellOwnershipGuardSubsystem.cpp"
COORDINATOR = SRC / "Private" / "OCLandmarkStartupCoordinatorSubsystem.cpp"
MUSEUM137 = SRC / "Private" / "OCR137MuseumPhotoModelSubsystem.cpp"
MUSEUM138 = SRC / "Private" / "OCR138MuseumInteractiveArchitectureSubsystem.cpp"
MUSEUM_LAYER = SRC / "Private" / "OCMuseumLayerPerformanceGuardSubsystem.cpp"
SILPO140 = SRC / "Private" / "OCR140SilpoPhotoModelSubsystem.cpp"
CULTURE146 = SRC / "Private" / "OCR146CultureHousePhotoModelSubsystem.cpp"
SEPARATION = SRC / "Private" / "OCR146LandmarkSeparationSubsystem.cpp"
LAUNCHER = ROOT / "RUN_R21_LANDMARK_OWNERSHIP_RUNTIME_ACCEPTANCE.cmd"
START = ROOT / "START_HERE.cmd"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS21 VERIFY FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        raise SystemExit(f"PASS21 VERIFY FAIL: {label}: missing {needle!r}")


def forbid(text: str, needle: str, label: str) -> None:
    if needle in text:
        raise SystemExit(f"PASS21 VERIFY FAIL: {label}: forbidden {needle!r}")


def require_absent(path: Path, label: str) -> None:
    if path.exists():
        raise SystemExit(f"PASS21 VERIFY FAIL: retired {label} resurrected: {path.relative_to(ROOT)}")


# Pass45 physically retires the old late duplicate-repair owner. It must not be kept as an inert shell.
require_absent(GUARD_H, "landmark shell ownership guard header")
require_absent(GUARD, "landmark shell ownership guard source")

coordinator = read(COORDINATOR)
museum137 = read(MUSEUM137)
museum138 = read(MUSEUM138)
museum_layer = read(MUSEUM_LAYER)
silpo140 = read(SILPO140)
culture146 = read(CULTURE146)
separation = read(SEPARATION)
launcher = read(LAUNCHER)
start = read(START)

# Current owner identities: R13.7 is the single visible Museum exterior; R13.8 owns hidden interaction/collision + glass.
require(museum137, 'R137_MuseumPhotoModel', "Museum R13.7 visible exterior identity")
require(museum138, 'R138_MuseumInteractionCollision', "Museum R13.8 collision owner identity")
for needle in (
    "PASS45_MUSEUM_R138_COLLISION_ONLY_READY",
    "visible_components=0",
    "PASS45_MUSEUM_R137_VISIBLE_OWNER_PRESERVED",
    "visibility_mutation=0",
    "material_mutation=0",
):
    require(museum138, needle, "Museum R13.8 collision-only contract")
for needle in (
    "Component->SetVisibility(false, true);",
    "Component->SetHiddenInGame(true, true);",
    "MuseumBreakableGlass",
):
    require(museum138, needle, "hidden collision/final glass contract")

require(silpo140, 'Tags.Add(TEXT("R140_SilpoPhotoModel"))', "Silpo R14.0 owner identity")
require(silpo140, 'Tags.Add(TEXT("R140_SilpoEntranceDoor"))', "Silpo entrance identity")
require(culture146, 'Tags.Add(TEXT("R146_CultureHouseAuthoritative"))', "Culture House owner identity")

for text, marker, label in (
    (museum137, "MuseumPhotoModelDelaySeconds", "Museum R13.7 startup stage"),
    (museum138, "R138MuseumDelaySeconds", "Museum R13.8 interaction stage"),
    (silpo140, "SilpoBuildDelaySeconds", "Silpo R14.0 startup stage"),
):
    require(text, marker, label)

# Startup orchestration is single-window and cancels old delayed stage timers before authoritative calls.
for needle in (
    "RunAuthoritativeStartup",
    "Timers.ClearAllTimersForObject(Stage);",
    "PASS45_LANDMARK_STARTUP_COORDINATED_READY",
    "legacy_core_recovery=0",
    "destructive_visibility_rebuild=0",
):
    require(coordinator, needle, "Pass45 landmark startup coordinator")

# Museum layer ownership is observed, never repaired late.
for needle in (
    "PASS45_MUSEUM_LAYER_VALIDATION_READY",
    "PASS45_MUSEUM_LAYER_VALIDATION_FAIL",
    "mutation=0",
):
    require(museum_layer, needle, "Museum layer validation-only guard")
for stale in (
    "RunRepairPass",
    "RemoveInstance(",
    "SetVisibility(",
    "SetHiddenInGame(",
):
    forbid(museum_layer, stale, "late Museum layer mutation")

# Landmark separation is one bounded validation-only pass, not a duplicate repair loop.
for needle in (
    "constexpr float ValidationDelaySeconds",
    "ValidateSeparation",
    "PASS45_LANDMARK_SEPARATION_VALIDATION_SCHEDULED",
    "PASS45_LANDMARK_SEPARATION_VALIDATION_READY",
    "PASS45_LANDMARK_SEPARATION_VALIDATION_FAIL",
    "mutation=0",
    "periodic_scan=0",
    "primary_authoring_fix_required=1",
):
    require(separation, needle, "Pass45 landmark separation validation")
for stale in (
    "PASS21_LANDMARK_DUPLICATE_REPAIRED",
    "PASS21_LANDMARK_OWNERSHIP_READY",
    "PASS21_LANDMARK_OWNERSHIP_FAIL",
    "PASS45_LANDMARK_RECONCILIATION_BUDGET_READY",
    "PASS45_LANDMARK_RECONCILIATION_COMPLETE",
    "AddOnActorSpawnedHandler",
    "RepairTaggedOwners",
):
    forbid(separation, stale, "retired duplicate-repair ownership path")

delay = re.search(r"ValidationDelaySeconds\s*=\s*([0-9.]+)f", separation)
if not delay or not (0.0 < float(delay.group(1)) < 12.0):
    raise SystemExit("PASS21 VERIFY FAIL: landmark separation validation delay must be bounded before Pass12 baseline")

# Internal launcher must require current validation evidence and reject current fail markers.
for needle in (
    "RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd",
    "PASS45_LANDMARK_STARTUP_COORDINATED_READY",
    "PASS45_MUSEUM_R137_VISIBLE_OWNER_PRESERVED",
    "PASS45_MUSEUM_R138_COLLISION_ONLY_READY",
    "PASS45_MUSEUM_LAYER_VALIDATION_READY",
    "PASS45_LANDMARK_SEPARATION_VALIDATION_READY",
    "PASS45_MUSEUM_LAYER_VALIDATION_FAIL",
    "PASS45_LANDMARK_SEPARATION_VALIDATION_FAIL",
    "Stay in gameplay for at least 15 seconds",
):
    require(launcher, needle, "Pass21/45 runtime acceptance launcher")
for stale in (
    "PASS21_LANDMARK_DUPLICATE_REPAIRED",
    "PASS21_LANDMARK_OWNERSHIP_READY",
    "PASS21_LANDMARK_OWNERSHIP_FAIL",
):
    forbid(launcher, stale, "retired Pass21 repair/readiness marker")

if "RUN_R21_LANDMARK_OWNERSHIP_RUNTIME_ACCEPTANCE.cmd" in start:
    raise SystemExit("PASS21 VERIFY FAIL: internal Pass21 launcher leaked into START_HERE")

print("LANDMARK SHELL OWNERSHIP PASS21/PASS45 SOURCE CONTRACT PASS")
print("- retired late duplicate-repair ownership guard stays physically deleted")
print("- R13.7 is the single visible Museum exterior; R13.8 is hidden collision/interactivity + breakable glass")
print("- Museum layer and landmark separation are validation-only with mutation=0")
print("- Silpo R14.0 and Culture House R14.6 keep canonical site identities")
print("- internal acceptance follows current Pass45 validation markers instead of repair markers")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 runtime/visual separation remains authoritative")
