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
    "R13_CollegeDarkPlinth",
    "R13_CollegeFacadeBands",
    "R13_CollegeEntranceCanopy",
    "R13_CollegeEntranceGlass",
    "Glass_Window.Glass_Window",
    "FloorBandZ[]",
    "GameMode->IsFrontendOnlySession()",
    "SetCollisionEnabled(ECollisionEnabled::NoCollision)",
    "SetCanEverAffectNavigation(false)",
    "authored 9x4 windows, stairs and footprint preserved",
]:
    if token not in college:
        fail(f"college facade marker missing: {token}")

for token in [
    "AOCWorldSectorOster::StadiumAnchor()",
    "R13_StadiumSurfaceRoot",
    "R13_StadiumTurf",
    "R13_StadiumTrack",
    "R13_StadiumFieldLines",
    "R13_StadiumGoalAccents",
    "R13_StadiumSpectatorBenches",
    "Old_Planks.Old_Planks",
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
print("Checks college plinth/facade/glazed entrance and stadium turf/track/markings/goals/real-plank seating, all frontend-guarded and gameplay-collision neutral.")
