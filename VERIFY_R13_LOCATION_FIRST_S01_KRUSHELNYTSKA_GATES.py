from pathlib import Path
import math
import re

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
REF_CPP = SRC / "Private" / "OCLocationSectorS01ReferenceData.cpp"
AUTHOR_H = SRC / "Public" / "OCLocationSectorS01KrushelnytskaAuthoringData.h"
AUTHOR_CPP = SRC / "Private" / "OCLocationSectorS01KrushelnytskaAuthoringData.cpp"
GEO_H = SRC / "Public" / "OCGeoReference.h"
GEO_CPP = SRC / "Private" / "OCGeoReference.cpp"
PLAN_CPP = SRC / "Private" / "OCLocationSectorPlan.cpp"
ROAD_CPP = SRC / "Private" / "OCLocationSectorS01RoadData.cpp"
WORLD = SRC / "Private" / "OCWorldSectorOster.cpp"

TOL = 0.05


def fail(message: str) -> None:
    raise SystemExit("R13 S01 KRUSHELNYTSKA GATE VERIFY FAIL: " + message)


for path in (REF_CPP, AUTHOR_H, AUTHOR_CPP, GEO_H, GEO_CPP, PLAN_CPP, ROAD_CPP, WORLD):
    if not path.is_file():
        fail(f"missing source: {path.relative_to(ROOT)}")

ref_cpp = REF_CPP.read_text(encoding="utf-8", errors="replace")
author_h = AUTHOR_H.read_text(encoding="utf-8", errors="replace")
author_cpp = AUTHOR_CPP.read_text(encoding="utf-8", errors="replace")
geo_h = GEO_H.read_text(encoding="utf-8", errors="replace")
geo_cpp = GEO_CPP.read_text(encoding="utf-8", errors="replace")
plan_cpp = PLAN_CPP.read_text(encoding="utf-8", errors="replace")
road_cpp = ROAD_CPP.read_text(encoding="utf-8", errors="replace")
world = WORLD.read_text(encoding="utf-8", errors="replace")

for token in [
    "FOCS01CenterlineAuthoringGate",
    "ReviewOnlyCenterlineGates()",
    "Review-only uncertainty gate",
    "not runtime road geometry",
    "must never render the gates themselves",
    "FOCS01ReferenceConflictRecord",
    "ReferenceConflictedRuntimeSegments()",
    "MaximumAllowedConfidence",
    "must not be promoted before replacement",
]:
    if token not in author_h + author_cpp:
        fail(f"review-only authoring/conflict contract missing: {token}")

origin_lat_match = re.search(r"OriginLatitude\s*=\s*([0-9.]+)", geo_h)
origin_lon_match = re.search(r"OriginLongitude\s*=\s*([0-9.]+)", geo_h)
if not origin_lat_match or not origin_lon_match:
    fail("cannot parse georeference origin")
origin_lat = float(origin_lat_match.group(1))
origin_lon = float(origin_lon_match.group(1))
meters_per_degree_lat = 111320.0
meters_per_degree_lon = 111320.0 * math.cos(math.radians(origin_lat))


def local_cm(lat: float, lon: float):
    return (
        (lon - origin_lon) * meters_per_degree_lon * 100.0,
        (lat - origin_lat) * meters_per_degree_lat * 100.0,
    )


def parse_address(ref_id: str):
    match = re.search(
        rf'TEXT\("{re.escape(ref_id)}"\),\s*TEXT\("[0-9A]+"\),\s*([0-9.]+),\s*([0-9.]+),\s*EOCReferenceConfidence::B,',
        ref_cpp,
    )
    if not match:
        fail(f"cannot parse source address reference {ref_id}")
    return local_cm(float(match.group(1)), float(match.group(2)))


def parse_geo(function_name: str, identifier: str):
    match = re.search(
        rf'FOCGeoReferencePoint FOCGeoReference::{function_name}\(\).*?return \{{ TEXT\("{identifier}"\),\s*([0-9.]+),\s*([0-9.]+),',
        geo_cpp,
        flags=re.S,
    )
    if not match:
        fail(f"cannot parse canonical {function_name} anchor")
    return local_cm(float(match.group(1)), float(match.group(2)))

college = parse_geo("College", "OsterCollege")
park = parse_geo("CentralPark", "CentralCityPark")
for token in ["- 12000.0f", "- 9000.0f", "+ 15000.0f", "+ 16000.0f"]:
    if token not in plan_cpp:
        fail(f"S01 workflow margin changed without gate re-audit: {token}")
xmin = min(college[0], park[0]) - 12000.0
ymin = min(college[1], park[1]) - 9000.0
xmax = max(college[0], park[0]) + 15000.0
ymax = max(college[1], park[1]) + 16000.0

p8 = parse_address("S01_KR_REF_08")
p14 = parse_address("S01_KR_REF_14")
p28 = parse_address("S01_KR_REF_28")
p40 = parse_address("S01_KR_REF_40")


def interpolate_at_y(a, b, target_y):
    if abs(b[1] - a[1]) < 1e-9:
        fail("cannot derive south gate from horizontal evidence chord")
    t = (target_y - a[1]) / (b[1] - a[1])
    return (a[0] + t * (b[0] - a[0]), target_y)


def interpolate_at_x(a, b, target_x):
    if abs(b[0] - a[0]) < 1e-9:
        fail("cannot derive east gate from vertical evidence chord")
    t = (target_x - a[0]) / (b[0] - a[0])
    return (target_x, a[1] + t * (b[1] - a[1]))

south_expected = interpolate_at_y(p8, p14, ymin)
east_expected = interpolate_at_x(p28, p40, xmax)

gate_pattern = re.compile(
    r'TEXT\("(S01_KR_GATE_[A-Z_]+)"\),\s*'
    r'FVector2D\((-?[0-9.]+)f,\s*(-?[0-9.]+)f\),\s*'
    r'FVector2D\(([0-9.]+)f,\s*([0-9.]+)f\),\s*'
    r'EOCReferenceConfidence::C,',
    flags=re.S,
)
records = gate_pattern.findall(author_cpp)
if len(records) != 2:
    fail(f"expected exactly two review-only S01 gates, parsed {len(records)}")
by_id = {record[0]: record for record in records}
expected_ids = {"S01_KR_GATE_SOUTH_ENTRY", "S01_KR_GATE_EAST_EXIT"}
if set(by_id) != expected_ids:
    fail(f"unexpected gate IDs: {sorted(by_id)}")


def values(record):
    _id, x, y, hx, hy = record
    return float(x), float(y), float(hx), float(hy)

south = values(by_id["S01_KR_GATE_SOUTH_ENTRY"])
east = values(by_id["S01_KR_GATE_EAST_EXIT"])

for name, actual, expected_value in [
    ("south x", south[0], south_expected[0]),
    ("south y", south[1], south_expected[1]),
    ("east x", east[0], east_expected[0]),
    ("east y", east[1], east_expected[1]),
]:
    if abs(actual - expected_value) > TOL:
        fail(f"derived gate drift for {name}: stored={actual:.6f}, expected={expected_value:.6f}")

if abs(south[1] - ymin) > TOL or abs(east[0] - xmax) > TOL:
    fail("gate centers no longer lie on the intended S01 workflow boundaries")

# Keep uncertainty deliberately broad and oriented along the boundary. This is a review aid, not precision theatre.
if south[2] < 5000.0 or south[3] > 1500.0:
    fail("south-entry gate lost its broad lateral / narrow boundary uncertainty profile")
if east[3] < 5000.0 or east[2] > 1500.0:
    fail("east-exit gate lost its broad longitudinal / narrow boundary uncertainty profile")

if not (xmin <= south[0] <= xmax and ymin <= south[1] <= ymax):
    fail("south gate left S01 bounds")
if not (xmin <= east[0] <= xmax and ymin <= east[1] <= ymax):
    fail("east gate left S01 bounds")

# The three retained Krushelnytska spine pieces are traversable migration geometry, not verified geography.
# The road registry intentionally spells confidence through its shared Provisional alias, so verify the alias itself is
# exactly C and then verify every conflicted record uses that alias. This checks semantics without depending on style.
if "constexpr EOCReferenceConfidence Provisional = EOCReferenceConfidence::C;" not in road_cpp:
    fail("S01 road Provisional confidence alias is no longer locked to C")

conflict_pattern = re.compile(
    r'TEXT\("(S01_KR_SPINE_(?:SOUTH_SHARED|INSIDE|NORTH_SHARED))"\),\s*'
    r'EOCReferenceConfidence::([ABC]),\s*TEXT\("([^"]+)"\)',
    flags=re.S,
)
conflicts = conflict_pattern.findall(author_cpp)
expected_conflicts = {
    "S01_KR_SPINE_SOUTH_SHARED",
    "S01_KR_SPINE_INSIDE",
    "S01_KR_SPINE_NORTH_SHARED",
}
if len(conflicts) != 3 or {item[0] for item in conflicts} != expected_conflicts:
    fail(f"expected exactly three reference-conflicted Krushelnytska spine records, found {[item[0] for item in conflicts]}")

for runtime_id, maximum_confidence, reason in conflicts:
    if maximum_confidence != "C":
        fail(f"reference-conflicted segment {runtime_id} was allowed above C confidence")
    if len(reason.strip()) < 30:
        fail(f"reference conflict reason is too weak for {runtime_id}")

    marker = f'TEXT("{runtime_id}")'
    if road_cpp.count(marker) != 1:
        fail(f"expected one actual road record for reference-conflicted segment {runtime_id}")
    start = road_cpp.index(marker)
    next_record = road_cpp.find('TEXT("S01_', start + len(marker))
    end = next_record if next_record >= 0 else min(len(road_cpp), start + 3000)
    snippet = road_cpp[start:end]
    if not re.search(r'EOCS01RoadRelation::(?:Inside|Crossing),\s*Provisional,\s*TEXT\(', snippet, flags=re.S):
        fail(f"reference-conflicted road {runtime_id} does not use the C-locked Provisional alias")

# The authoring gates/conflict metadata are deliberately forbidden from runtime until a separately reviewed
# carriageway skeleton exists. Conflict records explain the old geometry; they do not become a new runtime owner.
for forbidden in [
    '#include "OCLocationSectorS01KrushelnytskaAuthoringData.h"',
    "FOCLocationSectorS01KrushelnytskaAuthoringData::ReviewOnlyCenterlineGates()",
    "FOCLocationSectorS01KrushelnytskaAuthoringData::ReferenceConflictedRuntimeSegments()",
]:
    if forbidden in world:
        fail(f"review-only Krushelnytska authoring data leaked into runtime world construction: {forbidden}")

print("R13 S01 KRUSHELNYTSKA GATE VERIFY: PASS")
print(
    f"Derived review-only S01 gates from source evidence: south entry ({south[0]:.1f},{south[1]:.1f}) cm, "
    f"east exit ({east[0]:.1f},{east[1]:.1f}) cm. Both remain C-confidence uncertainty windows; the three retained "
    "Krushelnytska spine pieces are explicitly reference-conflicted and verified through the C-locked Provisional alias."
)
