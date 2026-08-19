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
    "OwnedInsideCorridors()", "SharedCrossingCorridors()",
    "OwnedCentralParkPaths()", "OwnedCollegePaths()",
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
if len(road_records) != 8:
    fail(f"expected 8 audited BuildRoadNetwork corridors, parsed {len(road_records)}")

road_ids = [record[0] for record in road_records]
expected_road_ids = {
    "S01_ROAD_COLLEGE_APPROACH",
    "S01_CROSS_WORLD_EW_02",
    "S01_CROSS_KRUSHELNYTSKA_SPINE",
    "S01_CROSS_WORLD_DIAG_01",
    "S01_CROSS_WORLD_NW_01",
    "S01_CROSS_WORLD_DIAG_02",
    "S01_CROSS_PARK_SOUTH",
    "S01_CROSS_PARK_NORTH_LINK",
}
if set(road_ids) != expected_road_ids or len(road_ids) != len(set(road_ids)):
    fail(f"audited corridor ID set changed: {road_ids}")

inside_count = 0
crossing_count = 0
for record in road_records:
    rid, anchor, sx0, sy0, _sz0, ssx, ssy, _ssz, syaw, _two_walks, declared = record
    cx, cy = resolve_center(anchor, float(sx0), float(sy0))
    actual = classify_oriented_rect(cx, cy, float(ssx), float(ssy), float(syaw))
    if actual != declared:
        fail(f"geometry classification drift for {rid}: declared={declared}, actual={actual}")
    inside_count += actual == "Inside"
    crossing_count += actual == "Crossing"

if inside_count != 1 or crossing_count != 7:
    fail(f"expected 1 inside and 7 crossing BuildRoadNetwork corridors, got inside={inside_count}, crossing={crossing_count}")

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

# The park-to-NorthCivic path is derived from two anchors and crosses the S01 boundary. It must stay shared.
mid = ((park[0] + north_civic[0]) * 0.5, (park[1] + north_civic[1]) * 0.5)
delta = (north_civic[0] - park[0], north_civic[1] - park[1])
link_size = math.hypot(delta[0], delta[1])
link_yaw = math.degrees(math.atan2(delta[1], delta[0]))
if classify_oriented_rect(mid[0], mid[1], link_size, 260.0, link_yaw) != "Crossing":
    fail("CentralPark -> CultureParkNorth derived path is no longer a crossing; re-audit ownership")

# Runtime ownership: only inside road/path records are consumed by S01. Shared crossing records stay source-owned.
for token in [
    '#include "OCLocationSectorS01RoadData.h"',
    "FOCLocationSectorS01RoadData::OwnedInsideCorridors()",
    "ResolveS01RoadAnchor(Road.Anchor) + Road.LocalOffset",
    "Road.SizeCm, Road.Yaw, Road.bTwoWalks",
    "FOCLocationSectorS01RoadData::OwnedCentralParkPaths()",
    "AddBox(Sidewalks, Park + Path.LocalOffset, Path.SizeCm, Path.Yaw)",
    "FOCLocationSectorS01RoadData::OwnedCollegePaths()",
    "AddBox(Sidewalks, College + Path.LocalOffset, Path.SizeCm, Path.Yaw)",
]:
    if token not in world:
        fail(f"owned road/path runtime contract missing: {token}")
if "FOCLocationSectorS01RoadData::SharedCrossingCorridors()" in world:
    fail("shared crossing audit records must not be rendered by S01 before corridor splitting")

for forbidden in [
    "AddRoadWithWalks(College + FVector(-13500, 0, RoadZ), FVector(30000, 660, 14), 0.0f);",
    "AddBox(Sidewalks, Park + FVector(0, 0, 14), FVector(17800, 360, 18));",
    "AddBox(Sidewalks, Park + FVector(0, -300, 14), FVector(360, 13200, 18));",
    "AddBox(Sidewalks, Park + FVector(1800, 900, 14), FVector(11800, 260, 18), 31.0f);",
    "AddBox(Sidewalks, Park + FVector(-2300, 1300, 14), FVector(9300, 240, 18), -28.0f);",
    "AddBox(Sidewalks, College + FVector(900, 5200, 12), FVector(8000, 5900, 18), Yaw);",
]:
    if forbidden in world:
        fail(f"legacy direct S01 road/path call survived ownership migration: {forbidden}")

# The seven shared roads and one derived shared path remain in current builders until explicit split work.
for token in [
    "AddRoadWithWalks(FVector(-18000, 17000, RoadZ), FVector(61000, 820, 16), 0.0f);",
    "AddRoadWithWalks(FVector(-33500, 25000, RoadZ), FVector(112000, 920, 16), 91.5f);",
    "AddRoadWithWalks(FVector(-23500, 40500, RoadZ), FVector(51000, 760, 16), 18.0f);",
    "AddRoadWithWalks(FVector(-48000, 51000, RoadZ), FVector(52000, 720, 16), 63.0f, false);",
    "AddRoadWithWalks(FVector(-5000, 33500, RoadZ), FVector(49000, 760, 16), -34.0f);",
    "AddRoadWithWalks(Park + FVector(0, -8500, RoadZ), FVector(43000, 720, 16), 2.0f);",
    "AddRoadWithWalks(Park + FVector(-9000, 13500, RoadZ), FVector(37000, 700, 16), 79.0f, false);",
    "AddBox(Sidewalks, Mid + FVector(0,0,15), FVector(Delta.Size2D(), 260, 18), LinkYaw);",
]:
    if token not in world:
        fail(f"shared crossing geometry moved/changed without split audit: {token}")

print("R13 LOCATION-FIRST S01 ROAD TOPOLOGY VERIFY: PASS")
print(
    f"S01 bounds approx X[{xmin:.1f},{xmax:.1f}] Y[{ymin:.1f},{ymax:.1f}] cm; "
    "1 road corridor + 5 internal paths owned Inside; 7 road corridors + Park->NorthCivic path remain Crossing/shared."
)
