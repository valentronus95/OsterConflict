#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS45 MUSEUM LEGACY BLOCKOUT VERIFY FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        raise SystemExit(f"PASS45 MUSEUM LEGACY BLOCKOUT VERIFY FAIL: {label}: missing {needle!r}")


def forbid(text: str, needle: str, label: str) -> None:
    if needle in text:
        raise SystemExit(f"PASS45 MUSEUM LEGACY BLOCKOUT VERIFY FAIL: {label}: forbidden {needle!r}")


world = read(SRC / "OCWorldSectorOster.cpp")
r137 = read(SRC / "OCR137MuseumPhotoModelSubsystem.cpp")

museum_begin = world.find("void AOCWorldSectorOster::BuildMuseumAndStadium()")
stadium_begin = world.find("    // Stadium:", museum_begin)
if museum_begin < 0 or stadium_begin < 0:
    raise SystemExit("PASS45 MUSEUM LEGACY BLOCKOUT VERIFY FAIL: cannot isolate museum source section")
museum_source = world[museum_begin:stadium_begin]

# Canonical current world source must not author a second visible museum shell in the shared Landmark ISMs.
for forbidden in (
    "AddBox(LandmarkBlocks, Museum",
    "AddBox(LandmarkDetails, Museum",
    "AddGableRoof(LandmarkRoofs, Museum",
    "AddFacadeWindow(LandmarkWindows, Museum",
):
    forbid(museum_source, forbidden, "legacy world-sector museum presentation")

# Perimeter fences remain deliberately source-owned until a verified authored fence/site replacement exists.
if museum_source.count("AddBox(Fences, Museum") != 3:
    raise SystemExit(
        "PASS45 MUSEUM LEGACY BLOCKOUT VERIFY FAIL: museum must retain exactly three perimeter fence proxies"
    )
for needle in (
    "PASS45_MUSEUM_LEGACY_BLOCKOUT_SOURCE_RETIRED",
    "legacy_landmark_blocks=0",
    "legacy_landmark_roofs=0",
    "legacy_landmark_windows=0",
    "legacy_landmark_details=0",
    "perimeter_fence_proxies=3",
    "authoritative_presentation_owner=R137_MuseumPhotoModel",
    "runtime_visual_acceptance=pending",
):
    require(museum_source, needle, "source retirement marker")

# Shared Landmark families remain valid for College/other landmarks. This is a scoped museum retirement, not a
# global family deletion disguised as cleanup.
college_begin = world.find("void AOCWorldSectorOster::BuildCollegeSector()")
vegetation_begin = world.find("\nvoid AOCWorldSectorOster::BuildVegetation()", college_begin)
if college_begin < 0 or vegetation_begin < 0:
    raise SystemExit("PASS45 MUSEUM LEGACY BLOCKOUT VERIFY FAIL: cannot isolate college source section")
college_source = world[college_begin:vegetation_begin]
for needle in (
    "AddBox(LandmarkBlocks,",
    "AddBox(LandmarkRoofs,",
    "AddBox(LandmarkDetails,",
    "AddFacadeWindow(LandmarkWindows,",
):
    require(college_source, needle, "shared Landmark families must remain live outside Museum")

# R13.7 remains the authoritative visible museum shell and fails closed on missing authored content.
for needle in (
    "UOCR137MuseumPhotoModelSubsystem::OnWorldBeginPlay",
    "SuppressLegacyMuseum(World);",
    "BuildMuseum(World);",
    "IsSourceMuseumFamily",
    'Name == TEXT("LandmarkBlocks")',
    'Name == TEXT("LandmarkRoofs")',
    'Name == TEXT("LandmarkWindows")',
    'Name == TEXT("LandmarkDetails")',
    "SourceMuseumCleanupRadiusCm = 5000.0f",
    "PASS45_MUSEUM_AUTHORED_SHELL_FAIL",
    "basicshape_fallback=0",
    "PASS45_MUSEUM_R137_PRIMARY_EXTERIOR_READY",
    "runtime_photo_acceptance=0",
):
    require(r137, needle, "R13.7 authoritative/compatibility contract")

# Compatibility cleanup may remain for stale/older world variants, but it must not mutate Fences or hide the
# current source as a way to manufacture Gate K success.
forbid(r137, 'Name == TEXT("Fences")', "museum compatibility cleanup must not own perimeter fences")

print("PASS45 MUSEUM LEGACY BLOCKOUT RETIREMENT SOURCE PASS")
print("- AOCWorldSectorOster no longer authors a duplicate Landmark* museum shell")
print("- exactly three museum perimeter fence proxies remain source-owned")
print("- shared Landmark* families remain available to College/other landmarks")
print("- R13.7 remains the authoritative authored visible museum owner")
print("- R13.7 compatibility suppression remains scoped to stale Landmark* museum instances")
print("- authored shell failure remains fail-closed with no BasicShape fallback")
print("- runtime visual acceptance remains pending")
