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
        "museum layer validation class missing")
require("MuseumValidationRadiusCm = 5000.0f" in guard,
        "Museum ownership validation must cover the full 50 m site")
require("ValidationTimer" in guard_h and "RepairTimer" not in guard_h,
        "Pass45 Museum layer subsystem must be validation-only")
require("RunValidation" in guard_h + guard and "RunRepairPass" not in guard_h + guard,
        "late Museum repair pass survived Pass45")

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
    require(family in guard, f"source Museum overlap validation family missing: {family}")

for component in (
    'TEXT("R137Museum_Plinth")',
    'TEXT("R137Museum_BrickBody")',
    'TEXT("R137Museum_BlueGreyTimber")',
    'TEXT("R137Museum_SheetMetalRoof")',
    'TEXT("R137Museum_CarvedPaleTrim")',
    'TEXT("R137Museum_WindowGrilles")',
):
    require(component in guard, f"required visible R13.7 component validation missing: {component}")

for marker in (
    'MuseumVisibleOwnerTag(TEXT("R137_MuseumPhotoModel"))',
    'MuseumCollisionOwnerTag(TEXT("R138_MuseumInteractionCollision"))',
    "PASS45_MUSEUM_LAYER_VALIDATION_SCHEDULED",
    "PASS45_MUSEUM_LAYER_VALIDATION_READY",
    "PASS45_MUSEUM_LAYER_VALIDATION_FAIL",
    "primary_authoring_fix_required=1",
    "mutation=0",
):
    require(marker in guard, f"Pass45 Museum validation marker missing: {marker}")

for forbidden in (
    "RemoveInstance(",
    "SetVisibility(",
    "SetHiddenInGame(",
    "SetCollisionEnabled(",
    "SetCullDistances(",
    "MarkRenderStateDirty(",
):
    require(forbidden not in guard,
            f"Museum validation subsystem still mutates runtime state: {forbidden}")

# Pass45 ownership rules: R13.7 is the only visible exterior; R13.8 is hidden collision/interactivity.
require("PASS45_MUSEUM_R137_PRIMARY_EXTERIOR_READY" in r137 and
        "static_glass=0" in r137 and "prototype_doors=0" in r137 and "prototype_trees=0" in r137,
        "R13.7 single-visible-owner contract is incomplete")
for marker in (
    "R138_MuseumInteractionCollision",
    "MuseumInteractionCollision",
    "SetVisibility(false, true)",
    "SetHiddenInGame(true, true)",
    "PASS45_MUSEUM_R138_COLLISION_ONLY_READY",
):
    require(marker in r138, f"R13.8 hidden collision-only contract missing: {marker}")
require("PASS30_MUSEUM_SPECULATIVE_INTERIOR_REMOVED" not in r138,
        "retired visible-interior compatibility marker returned")

require("HideR137MuseumTrees" not in r145 and
        "PASS45_MUSEUM_R145_TREE_LAYOUT_READY" in r145,
        "R14.5 must own the current tree layout without hiding an older R13.7 pass")
require("PASS30_MUSEUM_WINDOW_FRAME_CLEAN_READY" in window and
        "Window_Frame_Part.Window_Frame_Part" not in window,
        "clean lightweight museum window frame must remain authoritative")

require(not (P / "Public" / "OCR141MuseumWindowReplacementSubsystem.h").exists() and
        not (P / "Private" / "OCR141MuseumWindowReplacementSubsystem.cpp").exists(),
        "obsolete R14.1 prototype-window replacement owner returned")

print("MUSEUM LAYER / RENDER BUDGET PASS 32/45 SOURCE CONTRACT PASS")
print("- R13.7 is the single visible Museum exterior owner")
print("- R13.8 owns hidden collision/interactivity only")
print("- source overlap is validated, never repaired late")
print("- R14.5 owns the current tree layout without hiding a prototype pass")
print("- obsolete R14.1 window replacement owner stays physically deleted")
print("STATUS: SOURCE VERIFIED; local UE 5.8 runtime remains required for visual/FPS acceptance")
