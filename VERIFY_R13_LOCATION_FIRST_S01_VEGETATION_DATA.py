from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
DATA_H = SRC / "Public" / "OCLocationSectorS01Data.h"
DATA_CPP = SRC / "Private" / "OCLocationSectorS01Data.cpp"


def fail(message: str) -> None:
    raise SystemExit("R13 LOCATION-FIRST S01 VEGETATION DATA VERIFY FAIL: " + message)


for path in (DATA_H, DATA_CPP):
    if not path.is_file():
        fail(f"missing file: {path.relative_to(ROOT)}")

header = DATA_H.read_text(encoding="utf-8", errors="replace")
data = DATA_CPP.read_text(encoding="utf-8", errors="replace")

for token in [
    "EOCS01VegetationAnchor",
    "EOCS01TreeFamily",
    "FOCS01TreeSeed",
    "FOCS01GrassPatchSeed",
    "ProvisionalVegetationTrees()",
    "ProvisionalGrassPatches()",
]:
    if token not in header + data:
        fail(f"vegetation contract missing: {token}")

park_tree_ids = re.findall(r'TEXT\("(S01_PARK_TREE_\d\d)"\)', data)
expected_park = [f"S01_PARK_TREE_{i:02d}" for i in range(1, 55)]
if park_tree_ids != expected_park:
    fail(f"expected ordered park tree IDs 01..54, found {len(park_tree_ids)} records")

college_tree_ids = re.findall(r'TEXT\("(S01_COLLEGE_TREE_\d\d)"\)', data)
expected_college = [f"S01_COLLEGE_TREE_{i:02d}" for i in range(1, 5)]
if college_tree_ids != expected_college:
    fail(f"expected ordered college tree IDs 01..04, found {college_tree_ids}")

grass_ids = re.findall(r'TEXT\("(S01_(?:PARK|COLLEGE)_GRASS_\d\d)"\)', data)
if grass_ids != ["S01_PARK_GRASS_01", "S01_COLLEGE_GRASS_01"]:
    fail(f"expected two explicit S01 grass records, found {grass_ids}")

if data.count("EOCS01VegetationAnchor::CentralPark") < 55:
    fail("central-park vegetation is not explicitly anchored")
if data.count("EOCS01VegetationAnchor::College") < 5:
    fail("college vegetation is not explicitly anchored")

for family in ("Broadleaf", "Poplar", "Birch", "Pine"):
    if f"EOCS01TreeFamily::{family}" not in data:
        fail(f"missing migrated tree family: {family}")

# 58 tree records + 2 grass records must remain explicitly provisional until individually re-referenced.
vegetation_section = data[data.index("ProvisionalVegetationTrees()"):]
if vegetation_section.count("Provisional,") != 60:
    fail("all 60 migrated S01 vegetation records must remain explicit C-confidence entries")

print("R13 LOCATION-FIRST S01 VEGETATION DATA VERIFY: PASS")
print("Checks 54 park trees, 4 college trees and 2 mown-grass records as explicit C-confidence anchor-relative data.")
