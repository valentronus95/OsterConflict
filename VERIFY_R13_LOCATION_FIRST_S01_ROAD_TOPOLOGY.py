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

TOL_CM = 0.02
CONTINUITY_TOL_CM = 0.002


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

accessors = [
    "OwnedInsideCorridors", "SharedCrossingCorridors", "KrushelnytskaSpineSegments",
    "EastWest02Segments", "WorldDiag01Segments", "WorldDiag02Segments", "ParkSouthSegments",
    "WorldNW01Segments", "ParkNorthLinkSegments", "OwnedCentralParkPaths", "OwnedCollegePaths",
]
for token in ["EOCS01RoadAnchor", "EOCS01RoadRelation", "FOCS01RoadCorridorSeed", "FOCS01PathSeed"]:
    if token not in road_h + road_cpp:
        fail(f"road/path ownership contract missing: {token}")
for accessor in accessors:
    if f"{accessor}()" not in road_h + road_cpp:
        fail(f"road/path accessor missing: {accessor}")

origin_lat_match = re.search(r"OriginLatitude\s*=\s*([0-9.]+)", geo_h)
origin_lon_match = re.search(r"OriginLongitude\s*=\s*([0-9.]+)", geo_h)
if not origin_lat_match or not origin_lon_match:
    fail("cannot parse georeference origin")
origin_lat = float(origin_lat_match.group(1))
origin_lon = float(origin_lon_match.group(1))


def parse_anchor(function_name: str) -> tuple[float, float]:
    match = re.search(
        rf"FOCGeoReferencePoint\s+FOCGeoReference::{function_name}\(\)\s*\{{.*?return\s*\{{\s*TEXT\([^\n]+?\),\s*([0-9.]+),\s*([0-9.]+),",
        geo_cpp, flags=re.S,
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


def axis_interval(c0: float, u_component: float, low: float, high: float) -> tuple[float, float]:
    if abs(u_component) < 1e-12:
        if low <= c0 <= high:
            return -math.inf, math.inf
        fail("corridor centerline cannot enter ownership-safe bounds")
    a = (low - c0) / u_component
    b = (high - c0) / u_component
    return min(a, b), max(a, b)


def symmetric_inside_interval(center: tuple[float, float], length: float, width: float, yaw: float) -> tuple[float, float, float]:
    envelope = width * 0.5 + 260.0 + 130.0
    angle = math.radians(yaw)
    ux, uy = math.cos(angle), math.sin(angle)
    vx, vy = -uy, ux
    x_interval = axis_interval(center[0], ux, xmin + envelope * abs(vx), xmax - envelope * abs(vx))
    y_interval = axis_interval(center[1], uy, ymin + envelope * abs(vy), ymax - envelope * abs(vy))
    half = length * 0.5
    lo = max(-half, x_interval[0], y_interval[0])
    hi = min(half, x_interval[1], y_interval[1])
    if lo >= hi:
        fail(f"corridor at {center} / yaw {yaw} has no symmetric inside interval")
    return lo, hi, envelope


def one_sided_inside_interval(center: tuple[float, float], length: float, width: float, yaw: float) -> tuple[float, float]:
    # bTwoWalks=false keeps only LateralA: road spans [-w/2,+w/2], sidewalk spans [+w/2+130,+w/2+390].
    # Ownership must therefore use the asymmetric lateral range [-w/2, +w/2+390].
    angle = math.radians(yaw)
    ux, uy = math.cos(angle), math.sin(angle)
    vx, vy = -uy, ux
    lateral = (-width * 0.5, width * 0.5 + 390.0)
    half = length * 0.5
    lo, hi = -half, half
    for c0, u_component, v_component, bound_lo, bound_hi in [
        (center[0], ux, vx, xmin, xmax),
        (center[1], uy, vy, ymin, ymax),
    ]:
        for lateral_offset in lateral:
            shifted = c0 + lateral_offset * v_component
            axis_lo, axis_hi = axis_interval(shifted, u_component, bound_lo, bound_hi)
            lo = max(lo, axis_lo)
            hi = min(hi, axis_hi)
    if lo >= hi:
        fail(f"corridor at {center} / yaw {yaw} has no one-sided inside interval")
    return lo, hi


def one_sided_relation(cx: float, cy: float, length: float, width: float, yaw: float) -> str:
    road_relation = classify_oriented_rect(cx, cy, length, width, yaw)
    angle = math.radians(yaw)
    vx, vy = -math.sin(angle), math.cos(angle)
    sidewalk_offset = width * 0.5 + 260.0
    sidewalk_relation = classify_oriented_rect(
        cx + vx * sidewalk_offset,
        cy + vy * sidewalk_offset,
        length,
        260.0,
        yaw,
    )
    if road_relation == "Inside" and sidewalk_relation == "Inside":
        return "Inside"
    if road_relation == "Outside" and sidewalk_relation == "Outside":
        return "Outside"
    return "Crossing"


road_pattern = re.compile(
    r'\{ TEXT\("([A-Z0-9_]+)"\), EOCS01RoadAnchor::(Absolute|CentralPark|College),\s*'
    r'FVector\((-?[0-9.]+),\s*(-?[0-9.]+),\s*(-?[0-9.]+)\),\s*'
    r'FVector\(([0-9.]+),\s*([0-9.]+),\s*([0-9.]+)\),\s*'
    r'(-?[0-9.]+)f,\s*(true|false),\s*\n\s*EOCS01RoadRelation::(Inside|Crossing),\s*Provisional,',
    flags=re.S,
)
road_records = road_pattern.findall(road_cpp)
if len(road_records) != 17:
    fail(f"expected 17 explicit road records after all audited road splits, parsed {len(road_records)}")
records_by_id = {record[0]: record for record in road_records}
if len(records_by_id) != len(road_records):
    fail("duplicate road corridor IDs")

college_id = "S01_ROAD_COLLEGE_APPROACH"
symmetric_specs = [
    ("KrushelnytskaSpineSegments", ["S01_KR_SPINE_SOUTH_SHARED", "S01_KR_SPINE_INSIDE", "S01_KR_SPINE_NORTH_SHARED"], (-33500.0, 25000.0), 112000.0, 920.0, 91.5, ["Crossing", "Inside", "Crossing"]),
    ("EastWest02Segments", ["S01_EW02_INSIDE", "S01_EW02_EAST_SHARED"], (-18000.0, 17000.0), 61000.0, 820.0, 0.0, ["Inside", "Crossing"]),
    ("WorldDiag01Segments", ["S01_DIAG01_INSIDE", "S01_DIAG01_EAST_SHARED"], (-23500.0, 40500.0), 51000.0, 760.0, 18.0, ["Inside", "Crossing"]),
    ("WorldDiag02Segments", ["S01_DIAG02_INSIDE", "S01_DIAG02_EAST_SHARED"], (-5000.0, 33500.0), 49000.0, 760.0, -34.0, ["Inside", "Crossing"]),
    ("ParkSouthSegments", ["S01_PARK_SOUTH_WEST_SHARED", "S01_PARK_SOUTH_INSIDE"], (park[0], park[1] - 8500.0), 43000.0, 720.0, 2.0, ["Crossing", "Inside"]),
]
one_sided_specs = [
    ("WorldNW01Segments", ["S01_NW01_INSIDE", "S01_NW01_NORTH_SHARED"], (-48000.0, 51000.0), 52000.0, 720.0, 63.0, ["Inside", "Crossing"]),
    ("ParkNorthLinkSegments", ["S01_PARK_NORTH_SOUTH_SHARED", "S01_PARK_NORTH_INSIDE", "S01_PARK_NORTH_NORTH_SHARED"], (park[0] - 9000.0, park[1] + 13500.0), 37000.0, 700.0, 79.0, ["Crossing", "Inside", "Crossing"]),
]
expected_ids = {college_id}
for _accessor, ids, *_rest in symmetric_specs + one_sided_specs:
    expected_ids.update(ids)
if set(records_by_id) != expected_ids:
    fail(f"audited road ID set changed: {sorted(records_by_id)}")

for obsolete in [
    "S01_CROSS_KRUSHELNYTSKA_SPINE", "S01_CROSS_WORLD_EW_02", "S01_CROSS_WORLD_DIAG_01",
    "S01_CROSS_WORLD_DIAG_02", "S01_CROSS_PARK_SOUTH", "S01_CROSS_WORLD_NW_01", "S01_CROSS_PARK_NORTH_LINK",
]:
    if obsolete in road_cpp:
        fail(f"obsolete unsplit audit record returned: {obsolete}")
if 'static const TArray<FOCS01RoadCorridorSeed> Corridors;' not in road_cpp:
    fail("SharedCrossingCorridors must remain explicitly empty after road ownership splitting")

college_record = records_by_id[college_id]
_, anchor, sx0, sy0, _sz0, ssx, ssy, _ssz, syaw, two_walks, declared = college_record
cx, cy = resolve_center(anchor, float(sx0), float(sy0))
if declared != "Inside" or two_walks != "true" or classify_oriented_rect(cx, cy, float(ssx), float(ssy), float(syaw)) != "Inside":
    fail("College approach ownership/profile drift")


def verify_partition(
    accessor: str,
    ids: list[str],
    original_center: tuple[float, float],
    original_length: float,
    original_width: float,
    original_yaw: float,
    expected_relations: list[str],
    two_walks: bool,
) -> None:
    angle = math.radians(original_yaw)
    ux, uy = math.cos(angle), math.sin(angle)
    vx, vy = -uy, ux
    half = original_length * 0.5
    if two_walks:
        safe_lo, safe_hi, envelope = symmetric_inside_interval(original_center, original_length, original_width, original_yaw)
    else:
        safe_lo, safe_hi = one_sided_inside_interval(original_center, original_length, original_width, original_yaw)
        envelope = None

    intervals = []
    total_length = 0.0
    inside_intervals = []
    for rid, expected_relation in zip(ids, expected_relations):
        record = records_by_id[rid]
        _rid, anchor, sx0, sy0, _sz0, ssx, ssy, ssz, syaw, stored_two_walks, declared = record
        if anchor != "Absolute":
            fail(f"split segment {rid} must remain absolute to preserve original world contour")
        cx, cy = float(sx0), float(sy0)
        length, width, height = float(ssx), float(ssy), float(ssz)
        yaw = float(syaw)
        if abs(width - original_width) > 1e-6 or abs(height - 16.0) > 1e-6 or abs(yaw - original_yaw) > 1e-6:
            fail(f"profile drift for {rid}: size=({length},{width},{height}), yaw={yaw}")
        if stored_two_walks != ("true" if two_walks else "false") or declared != expected_relation:
            fail(f"sidewalk/ownership drift for {rid}: walks={stored_two_walks}, relation={declared}")

        dx, dy = cx - original_center[0], cy - original_center[1]
        center_t = dx * ux + dy * uy
        perpendicular = dx * vx + dy * vy
        if abs(perpendicular) > TOL_CM:
            fail(f"split segment {rid} left original centerline by {perpendicular:.6f} cm")
        start_t, end_t = center_t - length * 0.5, center_t + length * 0.5
        intervals.append((rid, start_t, end_t))
        total_length += length

        relation = (
            classify_oriented_rect(cx, cy, length, envelope * 2.0, yaw)
            if two_walks
            else one_sided_relation(cx, cy, length, width, yaw)
        )
        if relation != expected_relation:
            fail(f"full generated geometry relation drift for {rid}: {relation} != {expected_relation}")
        if expected_relation == "Inside":
            inside_intervals.append((start_t, end_t))

    if abs(total_length - original_length) > CONTINUITY_TOL_CM:
        fail(f"{accessor} lengths sum to {total_length}, expected {original_length}")
    if abs(intervals[0][1] + half) > TOL_CM or abs(intervals[-1][2] - half) > TOL_CM:
        fail(f"{accessor} no longer preserves original corridor endpoints")
    for previous, current in zip(intervals, intervals[1:]):
        if abs(current[1] - previous[2]) > CONTINUITY_TOL_CM:
            fail(f"{accessor} gap/overlap between {previous[0]} and {current[0]}")
    if len(inside_intervals) != 1:
        fail(f"{accessor} must have exactly one S01-owned segment")
    actual_lo, actual_hi = inside_intervals[0]
    if abs(actual_lo - safe_lo) > 0.002 or abs(actual_hi - safe_hi) > 0.002:
        fail(f"{accessor} ownership cut drift: actual [{actual_lo:.6f},{actual_hi:.6f}] safe [{safe_lo:.6f},{safe_hi:.6f}]")
    if f"FOCLocationSectorS01RoadData::{accessor}()" not in world:
        fail(f"runtime does not consume split manifest {accessor}")


for spec in symmetric_specs:
    verify_partition(*spec, True)
for spec in one_sided_specs:
    verify_partition(*spec, False)

# Five explicit park/college paths remain wholly inside S01.
path_pattern = re.compile(
    r'\{ TEXT\("(S01_PATH_[A-Z0-9_]+)"\), EOCS01RoadAnchor::(CentralPark|College),\s*'
    r'FVector\((-?[0-9.]+),\s*(-?[0-9.]+),\s*(-?[0-9.]+)\),\s*'
    r'FVector\(([0-9.]+),\s*([0-9.]+),\s*([0-9.]+)\),\s*'
    r'(-?[0-9.]+)f,\s*\n\s*EOCS01RoadRelation::Inside,\s*Provisional,', flags=re.S,
)
path_records = path_pattern.findall(road_cpp)
expected_path_ids = {"S01_PATH_PARK_EW", "S01_PATH_PARK_NS", "S01_PATH_PARK_DIAG_E", "S01_PATH_PARK_DIAG_W", "S01_PATH_COLLEGE_CAMPUS"}
if len(path_records) != 5 or {record[0] for record in path_records} != expected_path_ids:
    fail("owned park/college path registry drift")
for pid, anchor, sx0, sy0, _sz0, ssx, ssy, _ssz, syaw in path_records:
    px, py = resolve_center(anchor, float(sx0), float(sy0))
    if classify_oriented_rect(px, py, float(ssx), float(ssy), float(syaw)) != "Inside":
        fail(f"owned path no longer fits wholly inside S01: {pid}")

# Park -> CultureParkNorth is the only remaining unsplit path crossing.
mid = ((park[0] + north_civic[0]) * 0.5, (park[1] + north_civic[1]) * 0.5)
delta = (north_civic[0] - park[0], north_civic[1] - park[1])
link_size = math.hypot(delta[0], delta[1])
link_yaw = math.degrees(math.atan2(delta[1], delta[0]))
if classify_oriented_rect(mid[0], mid[1], link_size, 260.0, link_yaw) != "Crossing":
    fail("CentralPark -> CultureParkNorth derived path is no longer Crossing; re-audit ownership")

for token in [
    '#include "OCLocationSectorS01RoadData.h"',
    "FOCLocationSectorS01RoadData::OwnedInsideCorridors()",
    "FOCLocationSectorS01RoadData::OwnedCentralParkPaths()",
    "FOCLocationSectorS01RoadData::OwnedCollegePaths()",
] + [f"FOCLocationSectorS01RoadData::{accessor}()" for accessor, *_rest in symmetric_specs + one_sided_specs]:
    if token not in world:
        fail(f"road/path runtime contract missing: {token}")
if "FOCLocationSectorS01RoadData::SharedCrossingCorridors()" in world:
    fail("empty SharedCrossingCorridors registry must not be rendered")

for forbidden in [
    "AddRoadWithWalks(College + FVector(-13500, 0, RoadZ), FVector(30000, 660, 14), 0.0f);",
    "AddRoadWithWalks(FVector(-33500, 25000, RoadZ), FVector(112000, 920, 16), 91.5f);",
    "AddRoadWithWalks(FVector(-18000, 17000, RoadZ), FVector(61000, 820, 16), 0.0f);",
    "AddRoadWithWalks(FVector(-23500, 40500, RoadZ), FVector(51000, 760, 16), 18.0f);",
    "AddRoadWithWalks(FVector(-5000, 33500, RoadZ), FVector(49000, 760, 16), -34.0f);",
    "AddRoadWithWalks(Park + FVector(0, -8500, RoadZ), FVector(43000, 720, 16), 2.0f);",
    "AddRoadWithWalks(FVector(-48000, 51000, RoadZ), FVector(52000, 720, 16), 63.0f, false);",
    "AddRoadWithWalks(Park + FVector(-9000, 13500, RoadZ), FVector(37000, 700, 16), 79.0f, false);",
]:
    if forbidden in world:
        fail(f"legacy unsplit S01 road call survived: {forbidden}")

if "AddBox(Sidewalks, Mid + FVector(0,0,15), FVector(Delta.Size2D(), 260, 18), LinkYaw);" not in world:
    fail("remaining Park->NorthCivic shared path changed before its path split audit")

print("R13 LOCATION-FIRST S01 ROAD TOPOLOGY VERIFY: PASS")
print(
    f"S01 bounds approx X[{xmin:.1f},{xmax:.1f}] Y[{ymin:.1f},{ymax:.1f}] cm; "
    "all 7 audited BuildRoadNetwork crossings are explicit continuity-preserving ownership splits; "
    "Park->NorthCivic is the only remaining unsplit crossing path."
)
