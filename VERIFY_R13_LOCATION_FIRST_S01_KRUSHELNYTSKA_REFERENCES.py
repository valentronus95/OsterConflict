from pathlib import Path
import math
import re

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
H = SRC / "Public" / "OCLocationSectorS01ReferenceData.h"
CPP = SRC / "Private" / "OCLocationSectorS01ReferenceData.cpp"
GEO_H = SRC / "Public" / "OCGeoReference.h"
GEO_CPP = SRC / "Private" / "OCGeoReference.cpp"
PLAN_CPP = SRC / "Private" / "OCLocationSectorPlan.cpp"
WORLD = SRC / "Private" / "OCWorldSectorOster.cpp"


def fail(message: str) -> None:
    raise SystemExit("R13 S01 KRUSHELNYTSKA REFERENCE VERIFY FAIL: " + message)


for path in (H, CPP, GEO_H, GEO_CPP, PLAN_CPP, WORLD):
    if not path.is_file():
        fail(f"missing source: {path.relative_to(ROOT)}")

h = H.read_text(encoding="utf-8", errors="replace")
cpp = CPP.read_text(encoding="utf-8", errors="replace")
geo_h = GEO_H.read_text(encoding="utf-8", errors="replace")
geo_cpp = GEO_CPP.read_text(encoding="utf-8", errors="replace")
plan_cpp = PLAN_CPP.read_text(encoding="utf-8", errors="replace")
world = WORLD.read_text(encoding="utf-8", errors="replace")

for token in [
    "FOCS01StreetAddressReference",
    "KrushelnytskaAddressReferences()",
    "FOCS01StreetExtentReference",
    "KrushelnytskaStreetExtentReference()",
    "reference evidence only",
    "not carriageway-center samples",
    "never a runtime waypoint",
    "never as a road centerline",
]:
    if token not in h + cpp:
        fail(f"reference-safety contract missing: {token}")

pattern = re.compile(
    r'\{ TEXT\("(S01_KR_REF_[A-Z0-9_]+)"\), TEXT\("([0-9A]+)"\),\s*'
    r'([0-9.]+),\s*([0-9.]+),\s*EOCReferenceConfidence::B,',
    flags=re.S,
)
records = pattern.findall(cpp)

expected = [
    ("S01_KR_REF_08", "8", 50.94759774583321, 30.876405160556917),
    ("S01_KR_REF_14", "14", 50.94843423662540, 30.878767729754150),
    ("S01_KR_REF_7A_COLLEGE", "7A", 50.949214117728445, 30.879129750813650),
    ("S01_KR_REF_28", "28", 50.94932787287711, 30.881081789926040),
    ("S01_KR_REF_40", "40", 50.95038900824071, 30.882703249013880),
    ("S01_KR_REF_42", "42", 50.95059613903775, 30.882914353105647),
    ("S01_KR_REF_74", "74", 50.953855214938635, 30.885876996912668),
    ("S01_KR_REF_78", "78", 50.95445505467416, 30.885954252027110),
    ("S01_KR_REF_98", "98", 50.957596730285466, 30.886583072725990),
]

if len(records) != len(expected):
    fail(f"expected {len(expected)} ordered Oster address references, parsed {len(records)}")

for actual, exp in zip(records, expected):
    aid, label, lat, lon = actual
    eid, elabel, elat, elon = exp
    if aid != eid or label != elabel:
        fail(f"reference order/id drift: actual={aid}/{label}, expected={eid}/{elabel}")
    if abs(float(lat) - elat) > 1e-11 or abs(float(lon) - elon) > 1e-11:
        fail(f"public-map coordinate drift for {aid}")

extent_pattern = re.compile(
    r'TEXT\("S01_KR_STREET_EXTENT_VISICOM"\),\s*'
    r'([0-9.]+),\s*([0-9.]+),\s*([0-9.]+),\s*([0-9.]+),\s*'
    r'([0-9.]+),\s*([0-9.]+),\s*EOCReferenceConfidence::B,',
    flags=re.S,
)
extent_match = extent_pattern.search(cpp)
if not extent_match:
    fail("cannot parse B-confidence whole-street extent evidence")
center_lat, center_lon, min_lat, min_lon, max_lat, max_lon = map(float, extent_match.groups())
expected_extent = (
    50.951601785552164,
    30.883556648533790,
    50.947336834596960,
    30.874850176800106,
    50.958347034213716,
    30.886361188850810,
)
for name, actual, expected_value in zip(
    ("center_lat", "center_lon", "min_lat", "min_lon", "max_lat", "max_lon"),
    (center_lat, center_lon, min_lat, min_lon, max_lat, max_lon),
    expected_extent,
):
    if abs(actual - expected_value) > 1e-11:
        fail(f"whole-street extent drift for {name}: {actual} != {expected_value}")

if not (min_lat < center_lat < max_lat and min_lon < center_lon < max_lon):
    fail("whole-street label center is no longer inside its recorded public-map extent")

origin_lat_match = re.search(r"OriginLatitude\s*=\s*([0-9.]+)", geo_h)
origin_lon_match = re.search(r"OriginLongitude\s*=\s*([0-9.]+)", geo_h)
if not origin_lat_match or not origin_lon_match:
    fail("cannot parse project georeference origin")
origin_lat = float(origin_lat_match.group(1))
origin_lon = float(origin_lon_match.group(1))

meters_per_degree_lat = 111320.0
meters_per_degree_lon = 111320.0 * math.cos(math.radians(origin_lat))


def local_cm(lat: float, lon: float) -> tuple[float, float]:
    return (
        (lon - origin_lon) * meters_per_degree_lon * 100.0,
        (lat - origin_lat) * meters_per_degree_lat * 100.0,
    )

points = {rid: local_cm(lat, lon) for rid, _label, lat, lon in expected}

# Visicom's whole-street bbox is metadata for the street object, not a surveyed envelope. Address 98 is about
# 15.6 m east of the returned bbox, so require consistency within a small explicit tolerance instead of pretending
# every independently geocoded property must be mathematically enclosed by that metadata box.
extent_pad_m = 25.0
lat_pad = extent_pad_m / meters_per_degree_lat
lon_pad = extent_pad_m / meters_per_degree_lon
max_extent_overrun_m = 0.0
for rid, _label, lat, lon in expected:
    south = max(0.0, min_lat - lat) * meters_per_degree_lat
    north = max(0.0, lat - max_lat) * meters_per_degree_lat
    west = max(0.0, min_lon - lon) * meters_per_degree_lon
    east = max(0.0, lon - max_lon) * meters_per_degree_lon
    overrun = math.hypot(max(west, east), max(south, north))
    max_extent_overrun_m = max(max_extent_overrun_m, overrun)
    if not (min_lat - lat_pad <= lat <= max_lat + lat_pad and min_lon - lon_pad <= lon <= max_lon + lon_pad):
        fail(f"address evidence {rid} exceeds the whole-street metadata bbox by more than {extent_pad_m:.0f} m")

# The ordered evidence must retain the macro east/north-east progression that invalidates the old near-vertical
# blockout as a factual street model. These are address-marker relationships, not road-center positions.
for prev, cur in zip(expected, expected[1:]):
    if cur[2] <= prev[2]:
        fail(f"south-to-north evidence ordering drifted between {prev[0]} and {cur[0]}")

x_7a, y_7a = points["S01_KR_REF_7A_COLLEGE"]
x_42, y_42 = points["S01_KR_REF_42"]
x_78, y_78 = points["S01_KR_REF_78"]
if x_42 - x_7a < 25000.0 or y_42 - y_7a < 14000.0:
    fail("7A -> 42 no longer demonstrates the verified east/north street progression")
if x_78 - x_42 < 20000.0 or y_78 - y_42 < 40000.0:
    fail("42 -> 78 no longer demonstrates the verified north-east street progression")

# The official College identity/address and the public-map 7A marker should remain essentially co-located.
def parse_geo_point(function_name: str, identifier: str):
    match = re.search(
        rf'FOCGeoReferencePoint FOCGeoReference::{function_name}\(\).*?return \{{ TEXT\("{identifier}"\),\s*'
        r'([0-9.]+),\s*([0-9.]+),',
        geo_cpp,
        flags=re.S,
    )
    if not match:
        fail(f"cannot parse canonical {function_name} anchor")
    return local_cm(float(match.group(1)), float(match.group(2)))

college = parse_geo_point("College", "OsterCollege")
park = parse_geo_point("CentralPark", "CentralCityPark")
college_marker_distance = math.hypot(x_7a - college[0], y_7a - college[1])
if college_marker_distance > 1000.0:
    fail(f"College anchor and public-map 7A marker diverged by {college_marker_distance / 100.0:.1f} m")

street_min = local_cm(min_lat, min_lon)
street_max = local_cm(max_lat, max_lon)
street_span_x_m = abs(street_max[0] - street_min[0]) / 100.0
street_span_y_m = abs(street_max[1] - street_min[1]) / 100.0
if street_span_x_m < 750.0 or street_span_y_m < 1100.0:
    fail("recorded whole-street extent became implausibly small for the locked public-map object")

# S01 is intentionally a workflow rectangle around College + Central Park, not the complete Krushelnytska street.
# Reference evidence now proves the street enters from the south, crosses the College slice, then bends east out of S01.
for token in ["- 12000.0f", "- 9000.0f", "+ 15000.0f", "+ 16000.0f"]:
    if token not in plan_cpp:
        fail(f"S01 workflow margin changed without Krushelnytska reference re-audit: {token}")
xmin = min(college[0], park[0]) - 12000.0
ymin = min(college[1], park[1]) - 9000.0
xmax = max(college[0], park[0]) + 15000.0
ymax = max(college[1], park[1]) + 16000.0


def inside_s01(point):
    return xmin <= point[0] <= xmax and ymin <= point[1] <= ymax

if not points["S01_KR_REF_08"][1] < ymin:
    fail("address 8 no longer demonstrates the south-side approach outside S01")
for rid in ("S01_KR_REF_14", "S01_KR_REF_7A_COLLEGE", "S01_KR_REF_28"):
    if not inside_s01(points[rid]):
        fail(f"expected College-slice evidence {rid} to remain inside S01")
for rid in ("S01_KR_REF_40", "S01_KR_REF_42"):
    if not points[rid][0] > xmax:
        fail(f"expected east-bend evidence {rid} to remain east of S01")

address28_east_margin_m = (xmax - points["S01_KR_REF_28"][0]) / 100.0
address40_east_overrun_m = (points["S01_KR_REF_40"][0] - xmax) / 100.0
if not (0.0 < address28_east_margin_m < 30.0):
    fail(f"address 28 is no longer a useful near-east-edge S01 marker: margin={address28_east_margin_m:.1f} m")
if address40_east_overrun_m < 75.0:
    fail(f"address 40 no longer provides strong east-exit evidence: overrun={address40_east_overrun_m:.1f} m")

# Critical safety rule: public reference evidence is not allowed to become runtime road placement by accidental use.
for forbidden in [
    '#include "OCLocationSectorS01ReferenceData.h"',
    "FOCLocationSectorS01ReferenceData::KrushelnytskaAddressReferences()",
    "FOCLocationSectorS01ReferenceData::KrushelnytskaStreetExtentReference()",
]:
    if forbidden in world:
        fail(f"reference evidence leaked directly into runtime road construction: {forbidden}")

print("R13 S01 KRUSHELNYTSKA REFERENCE VERIFY: PASS")
print(
    f"Locks 9 Oster-specific address markers plus the whole-street B-confidence metadata extent; College/7A delta "
    f"{college_marker_distance / 100.0:.1f} m, bbox about {street_span_x_m:.0f} x {street_span_y_m:.0f} m with max address overrun "
    f"{max_extent_overrun_m:.1f} m. Evidence enters S01 from south, address 28 sits ~{address28_east_margin_m:.1f} m inside the east workflow edge, "
    f"and address 40 is ~{address40_east_overrun_m:.1f} m east of it. Runtime road centerline remains separate."
)
