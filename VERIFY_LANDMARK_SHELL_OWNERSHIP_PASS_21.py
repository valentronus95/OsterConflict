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


def forbid(text: str, needle: str, label: str) -> None:
    if needle in text:
        raise SystemExit(f"PASS21 VERIFY FAIL: {label}: forbidden {needle!r}")


guard_h = read(GUARD_H)
guard = read(GUARD)
museum137 = read(MUSEUM137)
museum138 = read(MUSEUM138)
silpo140 = read(SILPO140)
culture146 = read(CULTURE146)
separation = read(SEPARATION)
launcher = read(LAUNCHER)
start = read(START)

# Stable identities remain, but Pass 45 clarifies roles: R13.8 is the Museum shell; R13.7 is a
# reference/detail/interactivity parent after its solid prototype is suppressed.
require(museum137, 'Tags.Add(TEXT("R137_MuseumPhotoModel"))', "Museum R13.7 reference identity")
require(museum138, 'Tags.Add(TEXT("R138_MuseumHighFidelityArchitecture"))', "Museum R13.8 shell identity")
require(silpo140, 'Tags.Add(TEXT("R140_SilpoPhotoModel"))', "Silpo R14.0 shell identity")
require(silpo140, 'Tags.Add(TEXT("R140_SilpoEntranceDoor"))', "Silpo entrance identity")
require(culture146, 'Tags.Add(TEXT("R146_CultureHouseAuthoritative"))', "Culture House shell identity")

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
    require(guard_h, needle, "Pass 21/45 guard header")

for needle in (
    'MuseumReferenceLayerTag(TEXT("R137_MuseumPhotoModel"))',
    'MuseumShellTag(TEXT("R138_MuseumHighFidelityArchitecture"))',
    'SilpoShellTag(TEXT("R140_SilpoPhotoModel"))',
    'SilpoEntranceDoorTag(TEXT("R140_SilpoEntranceDoor"))',
    'CultureHouseShellTag(TEXT("R146_CultureHouseAuthoritative"))',
    "AddOnActorSpawnedHandler",
    "SetTimerForNextTick",
    "GetGameTimeSinceCreation",
    "GetInstanceTransform(InstanceIndex, InstanceTransform, true)",
    "PASS21_LANDMARK_DUPLICATE_REPAIRED",
    "PASS45_LANDMARK_SINGLE_SHELL_CONTRACT_READY",
    "PASS45_SINGLE_LANDMARK_SHELL_OWNERS_READY",
    "PASS45_SINGLE_LANDMARK_SHELL_OWNERS_FAIL",
    "PASS21_LANDMARK_OWNERSHIP_READY",
    "PASS21_LANDMARK_OWNERSHIP_FAIL",
    "periodic_owner_scan=0",
):
    require(guard, needle, "Pass 45 single-shell runtime ownership guard")

require(guard, "MuseumReferenceLayerCount == 1 && MuseumShellCount == 1",
        "Museum reference/shell roles must be counted separately")
forbid(guard, "MuseumPrototypeTag", "historical R13.7-as-second-shell ownership concept returned")
forbid(guard, "MuseumArchitectureTag", "historical dual-shell tag naming returned")

# Pass 45 deliberately retires the former 0.20 s x 40 whole-world scan loop. Final ownership proof still runs
# after the current one-shot 6.25 s reconciliation and historical landmark build window.
final_delay = re.search(r"FinalValidationDelaySeconds\s*=\s*([0-9.]+)f", guard)
if not final_delay or float(final_delay.group(1)) < 8.5:
    raise SystemExit("PASS21 VERIFY FAIL: final ownership validation runs before the startup window closes")
require(separation, "SeparationValidationDelaySeconds = 6.25f", "Pass 45 one-shot separation delay")
require(separation, "PASS45_LANDMARK_RECONCILIATION_BUDGET_READY", "Pass 45 reconciliation budget evidence")
require(separation, "full_world_scan_passes=%d further_periodic_scan=0", "one-shot reconciliation completion")
forbid(separation, "SeparationStartupGuardPassCount", "obsolete 40-pass world scan contract returned")
forbid(separation, "SeparationStartupGuardIntervalSeconds", "obsolete 0.20 s world scan interval returned")

# Internal acceptance route remains usable and requires final ownership proof.
for needle in (
    "RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd",
    "PASS21_LANDMARK_OWNERSHIP_FAIL",
    "PASS21_LANDMARK_OWNERSHIP_READY",
    "PASS21_LANDMARK_DUPLICATE_REPAIRED",
    "Stay in gameplay for at least 15 seconds",
):
    require(launcher, needle, "Pass 21 runtime acceptance launcher")

if "RUN_R21_LANDMARK_OWNERSHIP_RUNTIME_ACCEPTANCE.cmd" in start:
    raise SystemExit("PASS21 VERIFY FAIL: internal Pass 21 launcher leaked into START_HERE")

print("LANDMARK SHELL OWNERSHIP PASS 21/45 SOURCE CONTRACT PASS")
print("- Museum has one shell owner (R13.8); R13.7 is reference/detail/interactivity only")
print("- Silpo R14.0 and Culture House R14.6 remain one shell owner per canonical site")
print("- late duplicate actors are repaired by spawn guard plus one final validation")
print("- obsolete 0.20 s x 40 full-world reconciliation stays retired")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 runtime/visual separation remains authoritative")
