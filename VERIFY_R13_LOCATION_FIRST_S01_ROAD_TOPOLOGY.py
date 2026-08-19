from pathlib import Path
import math
import re

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
GEO_H = SRC / "Public" / "OCGeoReference.h"
GEO_CPP = SRC / "Private" / "OCGeoReference.cpp"
PLAN_CPP = SRC / "Private" / "OCLocationSectorPlan.cpp"
ROAD_H = SRC / "Public" / "OCLocationSectorS01RoadData.h"
ROAD_CPP = SRC / "Private" / "OCLocationSectorS01RoadData.cpp"
WORLD = SRC / "Private" / "OCWorldSectorOster.cpp"


def fail(message: str) -> None:
    raise SystemExit("R13 LOCATION-FIRST S01 ROAD TOPOLOGY VERIFY FAIL: " + message)


for path in (GEO_H, GEO_CPP, PLAN_CPP, ROAD_H, ROAD_CPP, WORLD):
    if not path.is_file():
        fail(f"missing file: {path.relative_to(ROOT)}")

geo_h = GEO_H.read_text(encoding="utf-8", errors="replace")
geo_cpp = GEO_CPP.read_text(encoding="utf-8", errors="replace")
plan_cpp = PLAN_CPP.read_text(encoding="utf-8", errors="replace")
road_h = ROAD_H.read_text(encoding="utf-8", errors="replace")
road_cpp = ROAD_CPP.read_text(encoding="utf-8", errors="replace")
world = WORLD.read_text(encoding="utf-8", errors="replace")

for token in [
    "EOCS01RoadAnchor", "EOCS01RoadRelation", "FOCS01RoadCorridorSeed", "FOCS01PathSeed",
    "OwnedInsideCorridors()", "SharedCrossingCorridors()", "KrushelnytskaSpineSegments()",
    "EastWest02Segments()", "OwnedCentralParkPaths()", "OwnedCollegePaths()",
]:
    if token not in road_h + road_cpp:
        fail(f"road/path ownership contract missing: {token}")

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


college = to_local_cm(*parse_anchor("College"))
park = to_local_cm(*parse_anchor("CentralPark"))
north_civic = to_local_cm(*parse_anchor("CultureParkNorth"))

for margin in ["- 12000.0f", "- 9000.0f", "+ 15000.0f", "+ 16000.0f"]:
    if margin not in plan_cpp:
        fail(f"S01 workflow-bound margin changed without topology re-audit: {margin}")

xmin = min(college[0], park[0]) - 12000.0
ymin = min(college[1], park[1]) - 9000.0
xmax = max(college[0], park[0]) + 15000.0
ymax = max(college[1], park[1]) + 16000.0


def resolve_center(anchor: str, ox: float, oy: float) -> tuple[float, float]:
    if anchor == "College":
        return college[0] + ox, college[1] + oy
    if anchor == "CentralPark":
        return park[0] + ox, park[1] + oy
    return ox, oy


def classify_oriented_rect(cx: float, cy: float, sx: float, sy: float, yaw_deg: float) -> str:
    angle = math.radians(yaw_deg)
    ux, uy = math.cos(angle), math.sin(angle)
    vx, vy = -uy, ux
    hx, hy = sx * 0.5, sy * 0.5

    corners = [
        (cx + a * hx * ux + b * hy * vx, cy + a * hx * uy + b * hy * vy)
        for a in (-1.0, 1.0) for b in (-1.0, 1.0)
    ]
    if all(xmin <= x <= xmax and ymin <= y <= ymax for x, y in corners):
        return "Inside"

    acx, acy = (xmin + xmax) * 0.5, (ymin + ymax) * 0.5
    aex, aey = (xmax - xmin) * 0.5, (ymax - ymin) * 0.5
    dx, dy = cx - acx, cy - acy

    separated = (
        abs(dx) > aex + hx * abs(ux) + hy * abs(vx)
        or abs(dy) > aey + hx * abs(uy) + hy * abs(vy)
        or abs(dx * ux + dy * uy) > hx + aex * abs(ux) + aey * abs(uy)
        or abs(dx * vx + dy * vy) > hy + aex * abs(vx) + aey * abs(vy)
    )
    return "Outside" if separated else "Crossing"


road_pattern = re.compile(
    r'\{ TEXT\("([A-Z0-9_]+)"\), EOCS01RoadAnchor::(Absolute|CentralPark|College),\s*'
    r'FVector\((-?[0-9.]+),\s*(-?[0-9.]+),\s*(-?[0-9.]+)\),\s*'
    r'FVector\(([0-9.]+),\s*([0-9.]+),\s*([0-9.]+)\),\s*'
    r'(-?[0-9.]+)f,\s*(true|false),\s*\n\s*EOCS01RoadRelation::(Inside|Crossing),\s*Provisional,',
    flags=re.S,
)
road_records = road_pattern.findall(road_cpp)
if len(road_records) != 11:
    fail(f"expected 11 road records after two ownership splits, parsed {len(road_records)}")

records_by_id = {record[0]: record for record in road_records}
if len(records_by_id) != len(road_records):
    fail("duplicate road corridor IDs")

expected_unsplit_ids = {
    "S01_ROAD_COLLEGE_APPROACH",
    "S01_CROSS_WORLD_DIAG_01",
    "S01_CROSS_WORLD_NW_01",
    "S01_CROSS_WORLD_DIAG_02",
    "S01_CROSS_PARK_SOUTH",
    "S01_CROSS_PARK_NORTH_LINK",
}
expected_spine_ids = {
    "S01_KR_SPINE_SOUTH_SHARED",
    "S01_KR_SPINE_INSIDE",
    "S01_KR_SPINE_NORTH_SHARED",
}
expected_ew02_ids = {
    "S01_EW02_INSIDE",
    "S01_EW02_EAST_SHARED",
}
if set(records_by_id) != expected_unsplit_ids | expected_spine_ids | expected_ew02_ids:
    fail(f"audited road ID set changed: {sorted(records_by_id)}")
for obsolete in ["S01_CROSS_KRUSHELNYTSKA_SPINE", "S01_CROSS_WORLD_EW_02"]:
    if obsolete in road_cpp:
        fail(f"obsolete unsplit audit record returned: {obsolete}")

# Five still-unsplit shared corridors remain Crossing; College approach remains wholly Inside.
for rid in expected_unsplit_ids:
    record = records_by_id[rid]
    _rid, anchor, sx0, sy0, _sz0, ssx, ssy, _ssz, syaw, _two_walks, declared = record
    cx, cy = resolve_center(anchor, float(sx0), float(sy0))
    actual = classify_oriented_rect(cx, cy, float(ssx), float(ssy), float(syaw))
    expected_relation = "Inside" if rid == "S01_ROAD_COLLEGE_APPROACH" else "Crossing"
    if declared != expected_relation or actual != expected_relation:
        fail(f"geometry classification drift for {rid}: declared={declared}, actual={actual}, expected={expected_relation}")

# ---- Krushelnytska spine continuity / ownership proof ----
original_center = (-33500.0, 25000.0)
original_length = 112000.0
original_width = 920.0
original_yaw = 91.5
road_half_width = original_width * 0.5
walk_center_offset = road_half_width + 260.0
walk_half_width = 260.0 * 0.5
full_lateral_envelope = walk_center_offset + walk_half_width  # 850 cm each side

angle = math.radians(original_yaw)
ux, uy = math.cos(angle), math.sin(angle)
vx, vy = -uy, ux


def t_interval_for_axis(c0: float, u_component: float, low: float, high: float) -> tuple[float, float]:
    if abs(u_component) < 1e-12:
        if low <= c0 <= high:
            return -math.inf, math.inf
        fail("corridor centerline cannot enter shrunken S01 bounds")
    a = (low - c0) / u_component
    b = (high - c0) / u_component
    return min(a, b), max(a, b)


shrunk_xmin = xmin + full_lateral_envelope * abs(vx)
shrunk_xmax = xmax - full_lateral_envelope * abs(vx)
shrunk_ymin = ymin + full_lateral_envelope * abs(vy)
shrunk_ymax = ymax - full_lateral_envelope * abs(vy)
xt = t_interval_for_axis(original_center[0], ux, shrunk_xmin, shrunk_xmax)
yt = t_interval_for_axis(original_center[1], uy, shrunk_ymin, shrunk_ymax)
inside_lo = max(-original_length * 0.5, xt[0], yt[0])
inside_hi = min(original_length * 0.5, xt[1], yt[1])
if inside_lo >= inside_hi:
    fail("computed Krushelnytska inside interval is empty")

expected_spine_intervals = {
    "S01_KR_SPINE_SOUTH_SHARED": (-original_length * 0.5, inside_lo, "Crossing"),
    "S01_KR_SPINE_INSIDE": (inside_lo, inside_hi, "Inside"),
    "S01_KR_SPINE_NORTH_SHARED": (inside_hi, original_length * 0.5, "Crossing"),
}
parsed_spine_intervals = {}
for rid, (expected_start, expected_end, expected_relation) in expected_spine_intervals.items():
    record = records_by_id[rid]
    _rid, anchor, sx0, sy0, _sz0, ssx, ssy, ssz, syaw, two_walks, declared = record
    if anchor != "Absolute":
        fail(f"spine segment {rid} must remain absolute while preserving original world contour")
    cx, cy = float(sx0), float(sy0)
    length, width, height = float(ssx), float(ssy), float(ssz)
    yaw = float(syaw)
    if abs(width - original_width) > 1e-6 or abs(height - 16.0) > 1e-6 or abs(yaw - original_yaw) > 1e-6:
        fail(f"spine section/profile drift for {rid}: size=({length},{width},{height}), yaw={yaw}")
    if two_walks != "true" or declared != expected_relation:
        fail(f"spine configuration/ownership drift for {rid}")

    center_t = (cx - original_center[0]) * ux + (cy - original_center[1]) * uy
    start_t = center_t - length * 0.5
    end_t = center_t + length * 0.5
    parsed_spine_intervals[rid] = (start_t, end_t)
    if abs(start_t - expected_start) > 0.02 or abs(end_t - expected_end) > 0.02:
        fail(f"spine split drift for {rid}: [{start_t:.6f},{end_t:.6f}] != [{expected_start:.6f},{expected_end:.6f}]")

    envelope_relation = classify_oriented_rect(cx, cy, length, full_lateral_envelope * 2.0, yaw)
    if envelope_relation != expected_relation:
        fail(f"spine sidewalk-envelope ownership drift for {rid}: {envelope_relation} != {expected_relation}")

if abs(sum(float(records_by_id[rid][5]) for rid in expected_spine_ids) - original_length) > 0.002:
    fail("spine split lengths no longer sum to the original 112000 cm")
if abs(parsed_spine_intervals["S01_KR_SPINE_SOUTH_SHARED"][1] - parsed_spine_intervals["S01_KR_SPINE_INSIDE"][0]) > 0.002:
    fail("south/shared -> inside spine boundary developed a gap/overlap")
if abs(parsed_spine_intervals["S01_KR_SPINE_INSIDE"][1] - parsed_spine_intervals["S01_KR_SPINE_NORTH_SHARED"][0]) > 0.002:
    fail("inside -> north/shared spine boundary developed a gap/overlap")

# ---- East-west corridor continuity / ownership proof ----
ew_center = (-18000.0, 17000.0)
ew_length = 61000.0
ew_width = 820.0
ew_yaw = 0.0
ew_half_width = ew_width * 0.5
ew_lateral_envelope = ew_half_width + 260.0 + 130.0  # 800 cm each side including sidewalks
original_west = ew_center[0] - ew_length * 0.5
original_east = ew_center[0] + ew_length * 0.5
if not (ymin <= ew_center[1] - ew_lateral_envelope and ew_center[1] + ew_lateral_envelope <= ymax):
    fail("EW02 road + sidewalk envelope no longer fits S01 Y bounds; split requires re-audit")
inside_west = max(original_west, xmin)
inside_east = min(original_east, xmax)
if inside_west >= inside_east:
    fail("EW02 has no inside interval")

expected_ew_intervals = {
    "S01_EW02_INSIDE": (inside_west, inside_east, "Inside"),
    "S01_EW02_EAST_SHARED": (inside_east, original_east, "Crossing"),
}
parsed_ew_intervals = {}
for rid, (expected_start_x, expected_end_x, expected_relation) in expected_ew_intervals.items():
    record = records_by_id[rid]
    _rid, anchor, sx0, sy0, _sz0, ssx, ssy, ssz, syaw, two_walks, declared = record
    cx, cy = float(sx0), float(sy0)
    length, width, height = float(ssx), float(ssy), float(ssz)
    yaw = float(syaw)
    if anchor != "Absolute" or abs(cy - ew_center[1]) > 1e-6:
        fail(f"EW02 anchor/centerline drift for {rid}")
    if abs(width - ew_width) > 1e-6 or abs(height - 16.0) > 1e-6 or abs(yaw - ew_yaw) > 1e-6:
        fail(f"EW02 section/profile drift for {rid}: size=({length},{width},{height}), yaw={yaw}")
    if two_walks != "true" or declared != expected_relation:
        fail(f"EW02 configuration/ownership drift for {rid}")

    start_x = cx - length * 0.5
    end_x = cx + length * 0.5
    parsed_ew_intervals[rid] = (start_x, end_x)
    if abs(start_x - expected_start_x) > 0.002 or abs(end_x - expected_end_x) > 0.002:
        fail(f"EW02 split drift for {rid}: [{start_x:.6f},{end_x:.6f}] != [{expected_start_x:.6f},{expected_end_x:.6f}]")

    envelope_relation = classify_oriented_rect(cx, cy, length, ew_lateral_envelope * 2.0, yaw)
    if envelope_relation != expected_relation:
        fail(f"EW02 sidewalk-envelope ownership drift for {rid}: {envelope_relation} != {expected_relation}")

if abs(sum(float(records_by_id[rid][5]) for rid in expected_ew02_ids) - ew_length) > 0.002:
    fail("EW02 split lengths no longer sum to the original 61000 cm")
if abs(parsed_ew_intervals["S01_EW02_INSIDE"][1] - parsed_ew_intervals["S01_EW02_EAST_SHARED"][0]) > 0.002:
    fail("EW02 split developed a longitudinal gap/overlap")

# Five explicit park/college paths remain wholly inside S01.
path_pattern = re.compile(
    r'\{ TEXT\("(S01_PATH_[A-Z0-9_]+)"\), EOCS01RoadAnchor::(CentralPark|College),\s*'
    r'FVector\((-?[0-9.]+),\s*(-?[0-9.]+),\s*(-?[0-9.]+)\),\s*'
    r'FVector\(([0-9.]+),\s*([0-9.]+),\s*([0-9.]+)\),\s*'
    r'(-?[0-9.]+)f,\s*\n\s*EOCS01RoadRelation::Inside,\s*Provisional,',
    flags=re.S,
)
path_records = path_pattern.findall(road_cpp)
expected_path_ids = {
    "S01_PATH_PARK_EW",
    "S01_PATH_PARK_NS",
    "S01_PATH_PARK_DIAG_E",
    "S01_PATH_PARK_DIAG_W",
    "S01_PATH_COLLEGE_CAMPUS",
}
path_ids = [record[0] for record in path_records]
if len(path_records) != 5 or set(path_ids) != expected_path_ids or len(path_ids) != len(set(path_ids)):
    fail(f"expected 5 unique owned inside path records, found {path_ids}")

for record in path_records:
    pid, anchor, sx0, sy0, _sz0, ssx, ssy, _ssz, syaw = record
    cx, cy = resolve_center(anchor, float(sx0), float(sy0))
    actual = classify_oriented_rect(cx, cy, float(ssx), float(ssy), float(syaw))
    if actual != "Inside":
        fail(f"owned path no longer fits wholly inside S01: {pid} => {actual}")

# Park -> CultureParkNorth derived path remains shared/crossing.
mid = ((park[0] + north_civic[0]) * 0.5, (park[1] + north_civic[1]) * 0.5)
delta = (north_civic[0] - park[0], north_civic[1] - park[1])
link_size = math.hypot(delta[0], delta[1])
link_yaw = math.degrees(math.atan2(delta[1], delta[0]))
if classify_oriented_rect(mid[0], mid[1], link_size, 260.0, link_yaw) != "Crossing":
    fail("CentralPark -> CultureParkNorth derived path is no longer Crossing; re-audit ownership")

# Runtime ownership contracts.
for token in [
    '#include "OCLocationSectorS01RoadData.h"',
    "FOCLocationSectorS01RoadData::OwnedInsideCorridors()",
    "ResolveS01RoadAnchor(Road.Anchor) + Road.LocalOffset",
    "Road.SizeCm, Road.Yaw, Road.bTwoWalks",
    "FOCLocationSectorS01RoadData::KrushelnytskaSpineSegments()",
    "FOCLocationSectorS01RoadData::EastWest02Segments()",
    "AddRoadWithWalks(Segment.LocalOffset, Segment.SizeCm, Segment.Yaw, Segment.bTwoWalks)",
    "FOCLocationSectorS01RoadData::OwnedCentralParkPaths()",
    "AddBox(Sidewalks, Park + Path.LocalOffset, Path.SizeCm, Path.Yaw)",
    "FOCLocationSectorS01RoadData::OwnedCollegePaths()",
    "AddBox(Sidewalks, College + Path.LocalOffset, Path.SizeCm, Path.Yaw)",
]:
    if token not in world:
        fail(f"owned road/path runtime contract missing: {token}")
if "FOCLocationSectorS01RoadData::SharedCrossingCorridors()" in world:
    fail("still-unsplit shared crossing audit records must not be rendered from the registry")

for forbidden in [
    "AddRoadWithWalks(College + FVector(-13500, 0, RoadZ), FVector(30000, 660, 14), 0.0f);",
    "AddRoadWithWalks(FVector(-33500, 25000, RoadZ), FVector(112000, 920, 16), 91.5f);",
    "AddRoadWithWalks(FVector(-18000, 17000, RoadZ), FVector(61000, 820, 16), 0.0f);",
    "AddBox(Sidewalks, Park + FVector(0, 0, 14), FVector(17800, 360, 18));",
    "AddBox(Sidewalks, Park + FVector(0, -300, 14), FVector(360, 13200, 18));",
    "AddBox(Sidewalks, Park + FVector(1800, 900, 14), FVector(11800, 260, 18), 31.0f);",
    "AddBox(Sidewalks, Park + FVector(-2300, 1300, 14), FVector(9300, 240, 18), -28.0f);",
    "AddBox(Sidewalks, College + FVector(900, 5200, 12), FVector(8000, 5900, 18), Yaw);",
]:
    if forbidden in world:
        fail(f"legacy direct S01 road/path call survived ownership migration: {forbidden}")

# Five still-unsplit shared roads and one derived shared path remain untouched until their own split pass.
for token in [
    "AddRoadWithWalks(FVector(-23500, 40500, RoadZ), FVector(51000, 760, 16), 18.0f);",
    "AddRoadWithWalks(FVector(-48000, 51000, RoadZ), FVector(52000, 720, 16), 63.0f, false);",
    "AddRoadWithWalks(FVector(-5000, 33500, RoadZ), FVector(49000, 760, 16), -34.0f);",
    "AddRoadWithWalks(Park + FVector(0, -8500, RoadZ), FVector(43000, 720, 16), 2.0f);",
    "AddRoadWithWalks(Park + FVector(-9000, 13500, RoadZ), FVector(37000, 700, 16), 79.0f, false);",
    "AddBox(Sidewalks, Mid + FVector(0,0,15), FVector(Delta.Size2D(), 260, 18), LinkYaw);",
]:
    if token not in world:
        fail(f"shared crossing geometry moved/changed without its split audit: {token}")

print("R13 LOCATION-FIRST S01 ROAD TOPOLOGY VERIFY: PASS")
print(
    f"S01 bounds approx X[{xmin:.1f},{xmax:.1f}] Y[{ymin:.1f},{ymax:.1f}] cm; "
    "College approach + Krushelnytska middle + EW02 west + 5 internal paths are Inside; "
    "5 unsplit road crossings + 2 shared spine remainders + EW02 east remainder + Park->NorthCivic path remain shared."
)
