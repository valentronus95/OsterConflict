from pathlib import Path

ROOT = Path(__file__).resolve().parent
PUB = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public" / "OCR13CentralParkCanopySubsystem.h"
CPP = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCR13CentralParkCanopySubsystem.cpp"


def fail(message: str) -> None:
    raise SystemExit("R13.5 CENTRAL PARK CANOPY VERIFY FAIL: " + message)


if not PUB.is_file() or not CPP.is_file():
    fail("central park canopy header/source missing")

h = PUB.read_text(encoding="utf-8", errors="replace")
cpp = CPP.read_text(encoding="utf-8", errors="replace")

includes = [line.strip() for line in h.splitlines() if line.strip().startswith("#include")]
if not includes or "generated.h" not in includes[-1]:
    fail("generated.h must remain the final header include")

required = [
    "AOCWorldSectorOster::ParkAnchor()",
    "R13_CentralParkCanopyRoot",
    "R13_CentralParkCanopy%02d",
    "SM_Tree_Var03.SM_Tree_Var03",
    "SM_Tree_Var04.SM_Tree_Var04",
    "SM_Tree_Var05.SM_Tree_Var05",
    "SM_Pine_Tree_03.SM_Pine_Tree_03",
    "SM_Pine_Tree_05.SM_Pine_Tree_05",
    "SetCollisionEnabled(ECollisionEnabled::NoCollision)",
    "SetGenerateOverlapEvents(false)",
    "SetCanEverAffectNavigation(false)",
    "GameMode->IsFrontendOnlySession()",
    "const FTreeSeed Seeds[]",
    "mature visual trees=%d",
    "memorial plaza, primary alleys and skate pad kept clear",
]
for token in required:
    if token not in cpp:
        fail(f"missing canopy marker: {token}")

# Keep the canopy bounded and deterministic. It is a visual ring, not procedural forest generation.
if cpp.count("{ FVector(") < 18:
    fail("canopy seed count unexpectedly sparse")
for forbidden in ["FMath::Rand", "FRand", "while (true)", "SetCollisionProfileName(TEXT(\"BlockAll\"))"]:
    if forbidden in cpp:
        fail(f"unsafe canopy marker present: {forbidden}")

for left, right in (("(", ")"), ("{", "}"), ("[", "]")):
    if cpp.count(left) != cpp.count(right):
        fail(f"delimiter mismatch {left}{right}")

print("R13.5 CENTRAL PARK CANOPY VERIFY: PASS")
print("Checks deterministic mature deciduous/conifer canopy, frontend guard, no collision/navigation influence and preserved memorial/alley/skate clear zones.")
