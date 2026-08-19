from pathlib import Path
import math
import re

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
GEO_H = SRC / "Public" / "OCGeoReference.h"
GEO_CPP = SRC / "Private" / "OCGeoReference.cpp"
PLAN_CPP = SRC / "Private" / "OCLocationSectorPlan.cpp"
ROAD_H = SRC / "Public" / "OCLocationSectorS01RoadData.h"
SPLIT_CPP = SRC / "Private" / "OCLocationSectorS01RoadPathSplitData.cpp"
WORLD_CPP = SRC / "Private" / "OCWorldSectorOster.cpp"

TOL = 0.03
CONTINUITY_TOL = 0.01


def fail(message: str) -> None:
    raise SystemExit("R13 S01 PARK-NORTH PATH VERIFY FAIL: " + message)


for path in (GEO_H, GEO_CPP, PLAN_CPP, ROAD_H, SPLIT_CPP, WORLD_CPP):
    if not path.is_file():
        fail(f"missing file: {path.relative_to(ROOT)}")

geo_h = GEO_H.read_text(encoding="utf-8", errors="replace")
geo_cpp = GEO_CPP.read_text(encoding="utf-8", errors="replace")
plan_cpp = PLAN_CPP.read_text(encoding="utf-8", errors="replace")
road_h = ROAD_H.read_text(encoding="utf-8", errors="replace")
split_cpp = SPLIT_CPP.read_text(encoding="utf-8", errors="replace")
world = WORLD_CPP.read_text(encoding="utf-8", errors="replace")

if "ParkNorthCivicPathSegments()" not in road_h or "ParkNorthCivicPathSegments()" not in split_cpp:
    fail("ParkNorthCivicPathSegments accessor/implementation missing")

origin_lat_match = re.search(r"OriginLatitude\s*=\s*([0-9.]+)", geo_h)
origin_lon_match = re.search(r"OriginLongitude\s*=\s*([0-9.]+)", geo_h)
if not origin_lat_match or not origin_lon_match:
    fail("cannot parse georeference origin")
origin_lat = float(origin_lat_match.group(1))
origin_lon = float(origin_lon_match.group(1))


def parse_anchor(function_name: str) -> tuple[float, float]:
    match = re.search(
        rf"FOCGeoReferencePoint\s+FOCGeoReference::{function_name}\(\)\s*\{{.*?return\s*\{{\s*TEXT\([^\n]+?\),\s*([0-9.]+),\s*([0-9.]+),",
        geo_cpp,
        flags=re.S,
    )
    if not match:
        fail(f"cannot parse {function_name} georeference")
    return float(match.group(1)), float(match.group(2))


def to_local_cm(lat: float, lon: float) -> tuple[float, float]:
    meters_per_degree_lat = 111320.0
    meters_per_degree_lon = 111320.0 * math.cos(math.radians(origin_lat))
    return (
        (lon - origin_lon) * meters_per_degree_lon * 100.0,
        (lat - origin_lat) * meters_per_degree_lat * 100.0,
    )


park = to_local_cm(*parse_anchor("CentralPark"))
north = to_local_cm(*parse_anchor("CultureParkNorth"))
college = to_local_cm(*parse_anchor("College"))

for margin in ["- 12000.0f", "- 9000.0f", "+ 15000.0f", "+ 16000.0f"]:
    if margin not in plan_cpp:
        fail(f"S01 workflow margin changed without re-audit: {margin}")

xmin = min(college[0], park[0]) - 12000.0
ymin = min(college[1], park[1]) - 9000.0
xmax = max(college[0], park[0]) + 15000.0
ymax = max(college[1], park[1]) + 16000.0

pattern = re.compile(
    r'\{ TEXT\("(S01_PATH_PARK_NORTH_CIVIC_[A-Z]+)"\), EOCS01RoadAnchor::Absolute,\s*'
    r'FVector\((-?[0-9.]+),\s*(-?[0-9.]+),\s*(-?[0-9.]+)\),\s*'
    r'FVector\(([0-9.]+),\s*([0-9.]+),\s*([0-9.]+)\),\s*'
    r'(-?[0-9.]+)f,\s*\n\s*EOCS01RoadRelation::(Inside|Crossing),\s*EOCReferenceConfidence::C,',
    flags=re.S,
)
records = pattern.findall(split_cpp)
if len(records) != 2:
    fail(f"expected two explicit north-civic path records, parsed {len(records)}")

by_id = {r[0]: r for r in records}
expected_ids = ["S01_PATH_PARK_NORTH_CIVIC_INSIDE", "S01_PATH_PARK_NORTH_CIVIC_SHARED"]
if set(by_id) != set(expected_ids):
    fail(f"unexpected path IDs: {sorted(by_id)}")


def record_values(record):
    _id, x, y, z, length, width, height, yaw, relation = record
    return float(x), float(y), float(z), float(length), float(width), float(height), float(yaw), relation

inside = record_values(by_id[expected_ids[0]])
shared = record_values(by_id[expected_ids[1]])

original_dx = north[0] - park[0]
original_dy = north[1] - park[1]
original_length = math.hypot(original_dx, original_dy)
original_yaw = math.degrees(math.atan2(original_dy, original_dx))
ux = original_dx / original_length
uy = original_dy / original_length

for name, value in (("inside width", inside[4]), ("shared width", shared[4])):
    if abs(value - 260.0) > TOL:
        fail(f"{name} drifted: {value}")
for name, value in (("inside height", inside[5]), ("shared height", shared[5])):
    if abs(value - 18.0) > TOL:
        fail(f"{name} drifted: {value}")
for name, value in (("inside z", inside[2]), ("shared z", shared[2])):
    if abs(value - 15.0) > TOL:
        fail(f"{name} drifted: {value}")
for name, value in (("inside yaw", inside[6]), ("shared yaw", shared[6])):
    if abs(value - original_yaw) > 0.001:
        fail(f"{name} does not preserve derived path yaw: stored={value}, expected={original_yaw}")
if inside[7] != "Inside" or shared[7] != "Crossing":
    fail(f"ownership relation drift: inside={inside[7]}, shared={shared[7]}")

if abs((inside[3] + shared[3]) - original_length) > TOL:
    fail("split lengths no longer preserve original park-to-north path length")


def endpoints(record):
    x, y, _z, length, _width, _height, yaw, _relation = record
    a = math.radians(yaw)
    vx, vy = math.cos(a), math.sin(a)
    half = length * 0.5
    return (x - vx * half, y - vy * half), (x + vx * half, y + vy * half)

inside_start, inside_end = endpoints(inside)
shared_start, shared_end = endpoints(shared)

if math.hypot(inside_start[0] - park[0], inside_start[1] - park[1]) > TOL:
    fail("inside segment start no longer matches CentralPark anchor")
if math.hypot(shared_end[0] - north[0], shared_end[1] - north[1]) > TOL:
    fail("shared segment end no longer matches CultureParkNorth anchor")
if math.hypot(inside_end[0] - shared_start[0], inside_end[1] - shared_start[1]) > CONTINUITY_TOL:
    fail("north-civic path split has a gap or overlap")


def classify_rect(record) -> str:
    x, y, _z, length, width, _height, yaw, _relation = record
    angle = math.radians(yaw)
    ax, ay = math.cos(angle), math.sin(angle)
    bx, by = -ay, ax
    hx, hy = length * 0.5, width * 0.5
    corners = [
        (x + sx * hx * ax + sy * hy * bx, y + sx * hx * ay + sy * hy * by)
        for sx in (-1.0, 1.0) for sy in (-1.0, 1.0)
    ]
    if all(xmin <= px <= xmax and ymin <= py <= ymax for px, py in corners):
        return "Inside"
    if all(px < xmin or px > xmax or py < ymin or py > ymax for px, py in corners):
        return "Outside"
    return "Crossing"

if classify_rect(inside) != "Inside":
    fail("declared inside path piece is not fully inside S01 bounds")
if classify_rect(shared) == "Inside":
    fail("declared shared path piece unexpectedly fits fully inside S01 bounds")

runtime_required = [
    "FOCLocationSectorS01RoadData::ParkNorthCivicPathSegments()",
    "AddBox(Sidewalks, Path.LocalOffset, Path.SizeCm, Path.Yaw);",
]
for token in runtime_required:
    if token not in world:
        fail(f"runtime ownership marker missing: {token}")

for forbidden in [
    "const FVector Mid = (Park + NorthCivic) * 0.5f;",
    "const FVector Delta = NorthCivic - Park;",
    "const float LinkYaw = FMath::RadiansToDegrees(FMath::Atan2(Delta.Y, Delta.X));",
    "AddBox(Sidewalks, Mid + FVector(0,0,15), FVector(Delta.Size2D(), 260, 18), LinkYaw);",
]:
    if forbidden in world:
        fail(f"legacy derived path construction returned: {forbidden}")

print("R13 S01 PARK-NORTH PATH VERIFY: PASS")
print(f"Checks two-piece explicit ownership with zero layout drift: {original_length:.2f} cm, yaw {original_yaw:.3f} deg.")
