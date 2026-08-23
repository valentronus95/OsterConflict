#!/usr/bin/env python3
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"

GUARD_H = SRC / "Public" / "OCLandmarkShellOwnershipGuardSubsystem.h"
GUARD = SRC / "Private" / "OCLandmarkShellOwnershipGuardSubsystem.cpp"
MUSEUM137 = SRC / "Private" / "OCR137MuseumPhotoModelSubsystem.cpp"
MUSEUM138 = SRC / "Private" / "OCR138MuseumInteractiveArchitectureSubsystem.cpp"
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


guard_h = read(GUARD_H)
guard = read(GUARD)
museum137 = read(MUSEUM137)
museum138 = read(MUSEUM138)
silpo140 = read(SILPO140)
culture146 = read(CULTURE146)
separation = read(SEPARATION)
launcher = read(LAUNCHER)
start = read(START)

# The three current site families and the known Museum upgrade layer must retain stable runtime identities.
require(museum137, 'Tags.Add(TEXT("R137_MuseumPhotoModel"))', "Museum R13.7 identity")
require(museum138, 'Tags.Add(TEXT("R138_MuseumHighFidelityArchitecture"))', "Museum R13.8 identity")
require(silpo140, 'Tags.Add(TEXT("R140_SilpoPhotoModel"))', "Silpo R14.0 identity")
require(silpo140, 'Tags.Add(TEXT("R140_SilpoEntranceDoor"))', "Silpo entrance identity")
require(culture146, 'Tags.Add(TEXT("R146_CultureHouseAuthoritative"))', "Culture House identity")

# Historical stages still own delayed callbacks; Pass 21 must remain defensive even when the startup coordinator
# normally cancels them.
for text, marker, label in (
    (museum137, "MuseumPhotoModelDelaySeconds", "Museum R13.7 delayed callback"),
    (museum138, "R138MuseumDelaySeconds", "Museum R13.8 delayed callback"),
    (silpo140, "SilpoBuildDelaySeconds", "Silpo R14.0 delayed callback"),
):
    require(text, marker, label)

for needle in (
    "UOCLandmarkShellOwnershipGuardSubsystem",
    "ActorSpawnedHandle",
    "FinalValidationTimer",
    "DuplicateRepairs",
    "EvaluateSpawnedActor",
    "RepairTaggedOwners",
    "RunFinalValidation",
    "HasInstanceGeometryNear",
):
    require(guard_h, needle, "Pass 21 guard header")

for needle in (
    'MuseumPrototypeTag(TEXT("R137_MuseumPhotoModel"))',
    'MuseumArchitectureTag(TEXT("R138_MuseumHighFidelityArchitecture"))',
    'SilpoShellTag(TEXT("R140_SilpoPhotoModel"))',
    'SilpoEntranceDoorTag(TEXT("R140_SilpoEntranceDoor"))',
    'CultureHouseShellTag(TEXT("R146_CultureHouseAuthoritative"))',
    "AddOnActorSpawnedHandler",
    "SetTimerForNextTick",
    'RepairTaggedOwners(*World, MuseumPrototypeTag, TEXT("Museum-R13.7"), false)',
    'RepairTaggedOwners(*World, MuseumArchitectureTag, TEXT("Museum-R13.8"), false)',
    'RepairTaggedOwners(*World, SilpoShellTag, TEXT("Silpo-R14.0"), true)',
    'RepairTaggedOwners(*World, CultureHouseShellTag, TEXT("CultureHouse-R14.6"), false)',
    'RepairTaggedOwners(*World, SilpoEntranceDoorTag, TEXT("SilpoEntranceDoor"), true)',
    "GetGameTimeSinceCreation",
    "GetInstanceTransform(InstanceIndex, InstanceTransform, true)",
    "PASS21_LANDMARK_DUPLICATE_REPAIRED",
    "PASS21_LANDMARK_OWNERSHIP_READY",
    "PASS21_LANDMARK_OWNERSHIP_FAIL",
):
    require(guard, needle, "Pass 21 runtime ownership guard")

# Pass 21 final proof must run after the existing eight-second landmark-separation startup window.
final_delay = re.search(r"FinalValidationDelaySeconds\s*=\s*([0-9.]+)f", guard)
if not final_delay or float(final_delay.group(1)) < 8.5:
    raise SystemExit("PASS21 VERIFY FAIL: final ownership validation runs before the historical startup window closes")
require(separation, "SeparationStartupGuardPassCount = 40", "existing separation pass count")
require(separation, "SeparationStartupGuardIntervalSeconds = 0.20f", "existing separation interval")

# The internal acceptance route chains the focused frontend/Museum/FPS run and then requires final ownership proof.
for needle in (
    "RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd",
    "PASS21_LANDMARK_OWNERSHIP_FAIL",
    "PASS21_LANDMARK_OWNERSHIP_READY",
    "PASS21_LANDMARK_DUPLICATE_REPAIRED",
    "Stay in gameplay for at least 15 seconds",
):
    require(launcher, needle, "Pass 21 runtime acceptance launcher")

# Keep the one user-facing launcher rule. Pass 21 is an internal acceptance helper only.
if "RUN_R21_LANDMARK_OWNERSHIP_RUNTIME_ACCEPTANCE.cmd" in start:
    raise SystemExit("PASS21 VERIFY FAIL: internal Pass 21 launcher leaked into START_HERE")

print("LANDMARK SHELL OWNERSHIP PASS 21 SOURCE CONTRACT PASS")
print("- R13.7/R13.8 Museum and R14.0 Silpo late rebuilds cannot persist duplicate current owners")
print("- Museum keeps the already-upgraded older owners; Silpo keeps the newest shell after its self-cleanup")
print("- Culture House remains single-owner and Silpo entrance doors are deduplicated on authority")
print("- final runtime proof requires one owner per stage plus instance geometry near canonical site anchors")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 runtime/visual acceptance still required")
