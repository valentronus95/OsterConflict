from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
DATA_H = SRC / "Public" / "OCLocationSectorS01Data.h"
DATA_CPP = SRC / "Private" / "OCLocationSectorS01Data.cpp"


def fail(message: str) -> None:
    raise SystemExit("R13 LOCATION-FIRST S01 VEGETATION DATA VERIFY FAIL: " + message)


def cxx_remainder(value: int, divisor: int) -> int:
    # C++ integer division truncates toward zero; Python // floors for negatives.
    return value - int(value / divisor) * divisor


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
expected_park_ids = [f"S01_PARK_TREE_{i:02d}" for i in range(1, 55)]
if park_tree_ids != expected_park_ids:
    fail(f"expected ordered park tree IDs 01..54, found {len(park_tree_ids)} records")

college_tree_ids = re.findall(r'TEXT\("(S01_COLLEGE_TREE_\d\d)"\)', data)
expected_college_ids = [f"S01_COLLEGE_TREE_{i:02d}" for i in range(1, 5)]
if college_tree_ids != expected_college_ids:
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

# Parse explicit park-tree records and prove they reproduce the former C++ Row/Col loop exactly.
park_pattern = re.compile(
    r'\{ TEXT\("(S01_PARK_TREE_\d\d)"\), EOCS01VegetationAnchor::CentralPark, '
    r'FVector\((-?\d+),\s*(-?\d+),\s*(-?\d+)\),\s*([0-9.]+)f, '
    r'EOCS01TreeFamily::(Broadleaf|Poplar|Birch|Pine), Provisional,'
)
park_records = park_pattern.findall(data)
if len(park_records) != 54:
    fail(f"could not parse all 54 explicit park tree records, parsed {len(park_records)}")

expected_park = []
index = 1
for row in range(-3, 4):
    for col in range(-4, 5):
        if abs(row) <= 1 and abs(col) <= 1:
            continue
        jitter_x = (cxx_remainder(row * 7 + col * 3, 5) - 2) * 180
        jitter_y = (cxx_remainder(row * 5 + col * 11, 5) - 2) * 160
        roll = abs(row * 9 + col * 5) % 12
        family = "Broadleaf"
        if roll <= 2:
            family = "Poplar"
        elif roll in (3, 4):
            family = "Birch"
        elif roll == 5:
            family = "Pine"
        x = col * 1850 + jitter_x
        y = row * 1700 + jitter_y
        scale = 0.85 + 0.05 * cxx_remainder(row + col + 8, 4)
        expected_park.append((f"S01_PARK_TREE_{index:02d}", x, y, 0, scale, family))
        index += 1

for parsed, expected in zip(park_records, expected_park):
    pid, sx, sy, sz, sscale, family = parsed
    eid, ex, ey, ez, escale, efamily = expected
    actual = (pid, int(sx), int(sy), int(sz), float(sscale), family)
    wanted = (eid, ex, ey, ez, escale, efamily)
    if actual[:4] != wanted[:4] or actual[5] != wanted[5] or abs(actual[4] - wanted[4]) > 1e-6:
        fail(f"park tree migration drift for {eid}: actual={actual}, expected={wanted}")

# College tree records were direct calls rather than an arithmetic loop; preserve those four values exactly.
expected_college = {
    "S01_COLLEGE_TREE_01": (-3800, -1100, 0, 1.20, "Pine"),
    "S01_COLLEGE_TREE_02": (3900, -950, 0, 1.15, "Pine"),
    "S01_COLLEGE_TREE_03": (-4600, 1500, 0, 1.00, "Pine"),
    "S01_COLLEGE_TREE_04": (4700, 2100, 0, 0.90, "Birch"),
}
college_pattern = re.compile(
    r'\{ TEXT\("(S01_COLLEGE_TREE_\d\d)"\), EOCS01VegetationAnchor::College, '
    r'FVector\((-?\d+),\s*(-?\d+),\s*(-?\d+)\),\s*([0-9.]+)f, '
    r'EOCS01TreeFamily::(Broadleaf|Poplar|Birch|Pine), Provisional,'
)
college_records = college_pattern.findall(data)
if len(college_records) != 4:
    fail(f"could not parse all 4 college tree records, parsed {len(college_records)}")
for pid, sx, sy, sz, sscale, family in college_records:
    actual = (int(sx), int(sy), int(sz), float(sscale), family)
    wanted = expected_college[pid]
    if actual[:3] != wanted[:3] or actual[4] != wanted[4] or abs(actual[3] - wanted[3]) > 1e-6:
        fail(f"college tree migration drift for {pid}: actual={actual}, expected={wanted}")

for exact in [
    '{ TEXT("S01_PARK_GRASS_01"), EOCS01VegetationAnchor::CentralPark, FVector(0, 0, 0), FVector(19000, 14500, 4), 6.0f, Provisional,',
    '{ TEXT("S01_COLLEGE_GRASS_01"), EOCS01VegetationAnchor::College, FVector(0, 5200, 0), FVector(12500, 7600, 4), 2.0f, Provisional,',
]:
    if exact not in data:
        fail(f"mown-grass migration drift: {exact}")

# 58 tree records + 2 grass records must remain explicitly provisional until individually re-referenced.
vegetation_section = data[data.index("ProvisionalVegetationTrees()"):]
if vegetation_section.count("Provisional,") != 60:
    fail("all 60 migrated S01 vegetation records must remain explicit C-confidence entries")

print("R13 LOCATION-FIRST S01 VEGETATION DATA VERIFY: PASS")
print("Checks 54 park trees against the former C++ placement formula, 4 direct college trees and 2 mown-grass records with zero migration drift.")
