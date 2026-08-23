#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
P = ROOT / "OsterConflict" / "Source" / "OsterConflict"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS32 VERIFY FAIL: missing {path}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"PASS32 VERIFY FAIL: {message}")


guard_h = read(P / "Public" / "OCMuseumLayerPerformanceGuardSubsystem.h")
guard = read(P / "Private" / "OCMuseumLayerPerformanceGuardSubsystem.cpp")
r137 = read(P / "Private" / "OCR137MuseumPhotoModelSubsystem.cpp")
r138 = read(P / "Private" / "OCR138MuseumInteractiveArchitectureSubsystem.cpp")
r145 = read(P / "Private" / "OCR145MuseumTreeLayoutSubsystem.cpp")
window = read(P / "Private" / "OCMuseumBreakableWindow.cpp")

require("UOCMuseumLayerPerformanceGuardSubsystem" in guard_h,
        "museum layer performance guard class missing")
require("MuseumCleanupRadiusCm = 5000.0f" in guard,
        "museum single-site cleanup must cover the full 50 m site")

for family in (
    'TEXT("LandmarkBlocks")',
    'TEXT("LandmarkRoofs")',
    'TEXT("LandmarkWindows")',
    'TEXT("LandmarkDetails")',
    'TEXT("Buildings")',
    'TEXT("ResidentialRoofs")',
    'TEXT("ResidentialDetails")',
    'TEXT("Fences")',
    'TEXT("TreeTrunks")',
    'TEXT("TreeCrowns")',
    'TEXT("SovietPoplarTrunks")',
    'TEXT("SovietPoplarCrowns")',
):
    require(family in guard, f"source museum overlap family missing: {family}")

for component in (
    'TEXT("R137Museum_BrickBody")',
    'TEXT("R137Museum_BlueGreyTimber")',
    'TEXT("R137Museum_WindowGlass")',
    'TEXT("R137Museum_WindowGrilles")',
    'TEXT("R137Museum_CarvedPaleTrim")',
    'TEXT("R137Museum_GreyDoors")',
    'TEXT("R137Museum_Pine01")',
    'TEXT("R137Museum_Pine03")',
    'TEXT("R137Museum_Deciduous01")',
):
    require(component in guard, f"obsolete R13.7 component guard missing: {component}")

for marker in (
    "RemoveInstancesNear(*Component, Museum, MuseumCleanupRadiusCm)",
    "SetVisibility(false, true)",
    "SetHiddenInGame(true, true)",
    "SetCollisionEnabled(ECollisionEnabled::NoCollision)",
    "SetCanEverAffectNavigation(false)",
    "SetCastShadow(false)",
    "SetCullDistances(0, CullEndCm)",
    "PASS32_MUSEUM_LAYER_REPAIR",
    "PASS32_MUSEUM_LAYER_BUDGET_READY",
    "PASS32_MUSEUM_LAYER_BUDGET_FAIL",
):
    require(marker in guard, f"Pass 32 repair/budget marker missing: {marker}")

require("RunRepairPass();" in guard and "FinalValidationDelaySeconds = 1.25f" in guard,
        "late idempotent museum repair/validation pass missing")
require("PrototypeOwners == 1 && ArchitectureOwners == 1" in guard,
        "Pass 32 must require one R13.7 carrier and one R13.8 architecture owner")
require("Destroy()" not in guard,
        "Pass 32 must not destroy the R13.7 carrier that owns interactive child actors")

# Existing passes must retain the earlier source-of-truth corrections while Pass 32 hardens them.
require("SourceMuseumCleanupRadiusCm = 5000.0f" in r137,
        "R13.7 source landmark cleanup regressed below 50 m")
require("PASS30_MUSEUM_SPECULATIVE_INTERIOR_REMOVED" in r138,
        "speculative museum interior partitions must remain removed")
require("HideR137MuseumTrees" in r145 and "SetCollisionEnabled(ECollisionEnabled::NoCollision)" in r145,
        "R14.5 must continue suppressing the old R13.7 tree pass")
require("PASS30_MUSEUM_WINDOW_FRAME_CLEAN_READY" in window and
        "Window_Frame_Part.Window_Frame_Part" not in window,
        "clean lightweight museum window frame must remain authoritative")

print("MUSEUM LAYER / RENDER BUDGET PASS 32 SOURCE CONTRACT PASS")
print("- source landmark/generic building/fence/tree instances are forbidden inside the dedicated 50 m museum site")
print("- obsolete R13.7 solid/window/tree layers are hidden and non-colliding after R13.8/R14.x startup")
print("- museum decorative ISMs have shadows disabled and explicit cull budgets")
print("- the R13.7 carrier actor is preserved for interactive ownership while duplicate visible geometry is removed")
print("STATUS: SOURCE VERIFIED; local UE 5.8 runtime remains required for measured FPS and visual acceptance")
