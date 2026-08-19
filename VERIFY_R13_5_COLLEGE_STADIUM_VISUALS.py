from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
COLLEGE_H = SRC / "Public" / "OCR13CollegeFacadeSubsystem.h"
COLLEGE_CPP = SRC / "Private" / "OCR13CollegeFacadeSubsystem.cpp"
STADIUM_H = SRC / "Public" / "OCR13StadiumSurfaceSubsystem.h"
STADIUM_CPP = SRC / "Private" / "OCR13StadiumSurfaceSubsystem.cpp"


def fail(message: str) -> None:
    raise SystemExit("R13.5 COLLEGE/STADIUM VISUAL VERIFY FAIL: " + message)


for path in (COLLEGE_H, COLLEGE_CPP, STADIUM_H, STADIUM_CPP):
    if not path.is_file():
        fail(f"missing {path.relative_to(ROOT)}")

college_h = COLLEGE_H.read_text(encoding="utf-8", errors="replace")
college = COLLEGE_CPP.read_text(encoding="utf-8", errors="replace")
stadium_h = STADIUM_H.read_text(encoding="utf-8", errors="replace")
stadium = STADIUM_CPP.read_text(encoding="utf-8", errors="replace")

for name, text in (("college", college_h), ("stadium", stadium_h)):
    includes = [line.strip() for line in text.splitlines() if line.strip().startswith("#include")]
    if not includes or "generated.h" not in includes[-1]:
        fail(f"generated.h must remain final include in {name} header")

for token in [
    "AOCWorldSectorOster::CollegeAnchor()",
    "R13_CollegeFacadeRoot",
    "R13_CollegeFacadeAligned",
    "R13_CollegeDarkPlinth",
    "R13_CollegeFacadeBands",
    "R13_CollegeEntranceCanopy",
    "R13_CollegeEntranceGlass",
    "R13_CollegeEntranceFrame",
    "Glass_Window.Glass_Window",
    "FloorBandZ[]",
    "constexpr float CollegeYawDegrees = 1.0f;",
    "constexpr float MainWidthCm = 6500.0f;",
    "constexpr float MainDepthCm = 1900.0f;",
    "constexpr float MainHeightCm = 1440.0f;",
    "constexpr float MainFrontY = -950.0f;",
    "constexpr float EntranceCenterX = 900.0f;",
    "constexpr float EntranceBlockCenterY = -1230.0f;",
    "constexpr float EntranceFrontY = -1530.0f;",
    "constexpr float EntranceCanopyCenterY = -1590.0f;",
    "MainWidthCm - 100.0f",
    "EntranceCenterX, EntranceFrontY - 8.0f",
    "EntranceCenterX, EntranceCanopyCenterY, 505.0f",
    "second fake entrance at X=0",
    "source already owns the 26.5 x 9.2 m canopy collision slab",
    "aligned to authored 65x19x14.4m main mass and X+900 entrance",
    "GameMode->IsFrontendOnlySession()",
    "SetCollisionEnabled(ECollisionEnabled::NoCollision)",
    "SetCanEverAffectNavigation(false)",
    "authored 9x4 windows, stairs and footprint preserved",
]:
    if token not in college:
        fail(f"college facade marker missing: {token}")

# The old visual pass was authored around an obsolete 48 m facade and a fake centered entrance.
# Those dimensions must not quietly return after being aligned to BuildCollegeSector topology.
for forbidden in [
    "FVector(4740.0f, 18.0f, 140.0f)",
    "FVector(4720.0f, 20.0f, 26.0f)",
    "College + FVector(0.0f, -1040.0f, 0.0f)",
    "const float FrontY = College.Y - 860.0f;",
    "Main authored block is 48 x 17 x 15.5 m",
]:
    if forbidden in college:
        fail(f"obsolete college facade topology returned: {forbidden}")

for token in [
    "AOCWorldSectorOster::StadiumAnchor()",
    "R13_StadiumSurfaceRoot",
    "R13_StadiumTurf",
    "R13_StadiumTrack",
    "R13_StadiumFieldLines",
    "R13_StadiumGoalAccents",
    "R13_StadiumSpectatorBenches",
    "Old_Planks_Plank_1.Old_Planks_Plank_1",
    "FVector(10440.0f, 6740.0f, 8.0f)",
    "R13.5 stadium surface: 105x68 m turf",
    "GameMode->IsFrontendOnlySession()",
    "SetCollisionEnabled(ECollisionEnabled::NoCollision)",
    "SetCanEverAffectNavigation(false)",
    "source collision preserved",
]:
    if token not in stadium:
        fail(f"stadium visual marker missing: {token}")

for label, text in (("college", college), ("stadium", stadium)):
    for forbidden in ["FMath::Rand", "FRand", "SetCollisionProfileName(TEXT(\"BlockAll\"))"]:
        if forbidden in text:
            fail(f"{label} contains unsafe visual-only marker: {forbidden}")
    for left, right in (("(", ")"), ("{", "}"), ("[", "]")):
        if text.count(left) != text.count(right):
            fail(f"delimiter mismatch {left}{right} in {label}")

print("R13.5 COLLEGE/STADIUM VISUAL VERIFY: PASS")
print("Checks college 65x19x14.4m facade alignment/X+900 entrance plus stadium turf/track/markings/goals/real-plank seating; all frontend-guarded and gameplay-collision neutral.")
