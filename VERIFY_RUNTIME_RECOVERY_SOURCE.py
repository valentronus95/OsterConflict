#!/usr/bin/env python3
"""Structural guard for the 2026-08-21 runtime-recovery source work.

This script proves only that the intended source contracts are present on disk. It MUST NOT be
reported as an Unreal Engine build, playtest or runtime verification.
"""

from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parent

CHECKS = {
    "runtime audit": (
        ROOT / "RUNTIME_AUDIT_2026-08-21.md",
        ["RT-01", "RT-08", "not considered runtime-fixed or acceptance-ready", "No new decorative R15/R16"],
    ),
    "legacy blockout audit": (
        ROOT / "LEGACY_BLOCKOUT_AUDIT_2026-08-21.md",
        ["LB-01", "LB-06", "4.95 seconds", "5.10 seconds", "(-69000, 64500, 0)"],
    ),
    "work ledger runtime override": (
        ROOT / "OSTER_CONFLICT_WORK_LEDGER.md",
        ["runtime override", "Runtime-over-code rule", "не створювати нові декоративні R15/R16"],
    ),
    "LocationTest launcher": (
        ROOT / "RUN_R14_MAIN_SANDBOX_TEST.cmd",
        ["LocationTest=1", "git lfs pull", "M opens/closes the tactical map", "Enter vehicle, drive, exit"],
    ),
    "11-weapon LocationTest rack": (
        ROOT / "OsterConflict/Source/OsterConflict/Private/OCRecoveredWeaponVariantSubsystem.cpp",
        ["WeaponTestCount = 11", "LocationTest=", "AOCAntiArmorLauncher::StaticClass", "IsWorldPickup()"],
    ),
    "vehicle exit input recovery": (
        ROOT / "OsterConflict/Source/OsterConflict/Private/OCVehicleExitInputRecoverySubsystem.cpp",
        ["ClearAllMappings", "ResetIgnoreMoveInput", "ResetIgnoreLookInput", "FInputModeGameOnly", "UIApplyLocalPreferences"],
    ),
    "tactical map contract": (
        ROOT / "OsterConflict/Source/OsterConflict/Private/OCTacticalMapSubsystem.cpp",
        ["EKeys::M", "EKeys::V", "UnmapKey", "Tactical map owns M exclusively", "FInputModeGameOnly"],
    ),
    "real weapon fallback": (
        ROOT / "OsterConflict/Source/OsterConflict/Private/OCRealWeaponFallbackSubsystem.cpp",
        ["/Game/R13/Weapons/machinegun.machinegun", "/Game/R13/Weapons/pistol.pistol", "Production verification remains OPEN"],
    ),
    "landmark startup exclusion guard": (
        ROOT / "OsterConflict/Source/OsterConflict/Private/OCR146LandmarkSeparationSubsystem.cpp",
        ["SeparationStartupGuardPassCount = 40", "AddOnActorSpawnedHandler", "FINAL landmark ownership validation", "runtime ownership is NOT verified"],
    ),
}

failures: list[str] = []

for label, (path, needles) in CHECKS.items():
    if not path.is_file():
        failures.append(f"{label}: missing {path.relative_to(ROOT)}")
        continue
    text = path.read_text(encoding="utf-8")
    for needle in needles:
        if needle not in text:
            failures.append(f"{label}: missing marker {needle!r} in {path.relative_to(ROOT)}")

# A source verifier must never accidentally bless runtime status.
ledger = (ROOT / "OSTER_CONFLICT_WORK_LEDGER.md").read_text(encoding="utf-8")
for required_open_id in (
    "GAME-WEAPONS-001",
    "VIS-FP-001",
    "ASSET-BTR-001",
    "ASSET-CHARACTER-001",
    "GAME-VEHICLE-INPUT-001",
    "UI-TACTICAL-MAP-001",
):
    matching = [line for line in ledger.splitlines() if f"| {required_open_id} |" in line]
    if not matching:
        failures.append(f"ledger: missing active row {required_open_id}")
    elif "| VERIFIED |" in matching[0] or "| DONE |" in matching[0]:
        failures.append(f"ledger: {required_open_id} is incorrectly runtime-closed")

if failures:
    print("RUNTIME RECOVERY SOURCE GUARD: FAIL")
    for failure in failures:
        print(f" - {failure}")
    print("This is a SOURCE-ONLY failure. It is not an UE runtime result.")
    sys.exit(1)

print("RUNTIME RECOVERY SOURCE GUARD: PASS")
print("Required recovery contracts are present and runtime-contradicted ledger rows remain open.")
print("SOURCE-ONLY PASS. UE 5.8 build/playtest is still required before any runtime verification.")
