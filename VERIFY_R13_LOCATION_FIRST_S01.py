from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
DOC = ROOT / "OsterConflict" / "Docs" / "OSTER_SECTOR_S01_KRUSHELNYTSKA_COLLEGE_PARK.md"
PLAN_H = SRC / "Public" / "OCLocationSectorPlan.h"
PLAN_CPP = SRC / "Private" / "OCLocationSectorPlan.cpp"
DATA_H = SRC / "Public" / "OCLocationSectorS01Data.h"
DATA_CPP = SRC / "Private" / "OCLocationSectorS01Data.cpp"
WORLD = SRC / "Private" / "OCWorldSectorOster.cpp"
INFILL = SRC / "Private" / "OCR13ResidentialInfillSubsystem.cpp"
DRESSING = SRC / "Private" / "OCR13EnvironmentDressingSubsystem.cpp"


def fail(message: str) -> None:
    raise SystemExit("R13 LOCATION-FIRST S01 VERIFY FAIL: " + message)


for path in (DOC, PLAN_H, PLAN_CPP, DATA_H, DATA_CPP, WORLD, INFILL, DRESSING):
    if not path.is_file():
        fail(f"missing file: {path.relative_to(ROOT)}")

plan_h = PLAN_H.read_text(encoding="utf-8", errors="replace")
plan_cpp = PLAN_CPP.read_text(encoding="utf-8", errors="replace")
data_h = DATA_H.read_text(encoding="utf-8", errors="replace")
data_cpp = DATA_CPP.read_text(encoding="utf-8", errors="replace")
world = WORLD.read_text(encoding="utf-8", errors="replace")
infill = INFILL.read_text(encoding="utf-8", errors="replace")
dressing = DRESSING.read_text(encoding="utf-8", errors="replace")
doc = DOC.read_text(encoding="utf-8", errors="replace")

for token in [
    "KrushelnytskaCollegePark()",
    "IsInsideKrushelnytskaCollegePark",
    'TEXT("S01_Krushelnytska_College_Park")',
    "FOCGeoReference::College()",
    "FOCGeoReference::CentralPark()",
]:
    if token not in plan_h + plan_cpp:
        fail(f"sector-plan contract missing: {token}")

for token in [
    "FOCS01ResidentialPlotSeed",
    "ProvisionalResidentialPlots()",
    "EOCReferenceConfidence::C",
    "bHasPrimaryHouse",
    "bHasOutbuilding",
]:
    if token not in data_h + data_cpp:
        fail(f"plot-registry contract missing: {token}")

ids = re.findall(r'TEXT\("(S01_KR_[WE]_\d\d)"\)', data_cpp)
if len(ids) != 16:
    fail(f"expected 16 individually addressable migrated plots, found {len(ids)}")
if len(set(ids)) != len(ids):
    fail("duplicate S01 plot IDs")
for side in ("W", "E"):
    expected = {f"S01_KR_{side}_{i:02d}" for i in range(1, 9)}
    actual = {value for value in ids if f"_{side}_" in value}
    if actual != expected:
        fail(f"{side}-side plot IDs do not cover 01..08")

if 'TEXT("S01_KR_E_03")' not in data_cpp or "-88.0f, false" not in data_cpp:
    fail("legacy east-side slot 03 absence was not preserved explicitly")

# Location-first guardrails: no city-wide generator is allowed to fill or dress S01.
if "procedural residential infill disabled" not in infill:
    fail("procedural residential infill is not disabled")
if "FOCLocationSectorPlan::IsInsideKrushelnytskaCollegePark(Block.Origin)" not in world:
    fail("generic source residential blocks are not excluded from S01")
for token in [
    '#include "OCLocationSectorPlan.h"',
    "IsProtectedFromGenericDressing",
    "FOCLocationSectorPlan::IsInsideKrushelnytskaCollegePark(Location)",
]:
    if token not in dressing:
        fail(f"generic environment dressing is not guarded from S01: {token}")

for token in [
    "ACTIVE RECONSTRUCTION SECTOR",
    "confidence C",
    "S01.1 — topology lock",
    "S01.2 — plot registry",
    "S01.5 — vegetation",
    "S01 LOCKED",
]:
    if token not in doc:
        fail(f"S01 execution document missing: {token}")

print("R13 LOCATION-FIRST S01 VERIFY: PASS")
print("Checks canonical sector ownership, 16 explicit C-confidence plot slots and exclusion from generic infill/dressing.")
