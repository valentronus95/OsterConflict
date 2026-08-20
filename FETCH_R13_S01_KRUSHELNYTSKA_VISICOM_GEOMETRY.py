from __future__ import annotations

import csv
import json
import math
import os
import re
import sys
from pathlib import Path
from urllib.parse import quote
from urllib.request import Request, urlopen

ROOT = Path(__file__).resolve().parent
GEO_H = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public" / "OCGeoReference.h"
OUT_DIR = ROOT / "OsterConflict" / "Saved" / "ReferenceEvidence"
RAW_OUT = OUT_DIR / "S01_Krushelnytska_Visicom_feature.json"
CSV_OUT = OUT_DIR / "S01_Krushelnytska_Visicom_local_cm.csv"
SUMMARY_OUT = OUT_DIR / "S01_Krushelnytska_Visicom_summary.txt"

FEATURE_ID = "STR3KJXJOBMQ"
API_BASE = "https://api.visicom.ua/data-api/5.0/uk/feature"


def fail(message: str) -> None:
    raise SystemExit("R13 S01 KRUSHELNYTSKA VISICOM FETCH FAIL: " + message)


def parse_origin() -> tuple[float, float]:
    if not GEO_H.is_file():
        fail(f"missing georeference header: {GEO_H.relative_to(ROOT)}")
    text = GEO_H.read_text(encoding="utf-8", errors="replace")
    lat = re.search(r"OriginLatitude\s*=\s*([-0-9.]+)", text)
    lon = re.search(r"OriginLongitude\s*=\s*([-0-9.]+)", text)
    if not lat or not lon:
        fail("cannot parse FOCGeoReference origin")
    return float(lat.group(1)), float(lon.group(1))


def feature_id_from_object(value) -> str | None:
    if isinstance(value, dict):
        for key in ("id", "featureId", "feature_id"):
            candidate = value.get(key)
            if isinstance(candidate, str) and candidate:
                return candidate
        properties = value.get("properties")
        if isinstance(properties, dict):
            for key in ("id", "featureId", "feature_id"):
                candidate = properties.get(key)
                if isinstance(candidate, str) and candidate:
                    return candidate
    return None


def find_feature_with_geometry(value):
    if isinstance(value, dict):
        geometry = value.get("geometry")
        if isinstance(geometry, dict) and geometry.get("coordinates") is not None:
            object_id = feature_id_from_object(value)
            if object_id == FEATURE_ID or object_id is None:
                return value
        for child in value.values():
            found = find_feature_with_geometry(child)
            if found is not None:
                return found
    elif isinstance(value, list):
        for child in value:
            found = find_feature_with_geometry(child)
            if found is not None:
                return found
    return None


def iter_lon_lat(geometry: dict):
    geometry_type = geometry.get("type")
    coordinates = geometry.get("coordinates")

    if geometry_type == "LineString":
        for point in coordinates or []:
            if isinstance(point, list) and len(point) >= 2:
                yield float(point[0]), float(point[1])
        return

    if geometry_type == "MultiLineString":
        for line in coordinates or []:
            for point in line or []:
                if isinstance(point, list) and len(point) >= 2:
                    yield float(point[0]), float(point[1])
        return

    if geometry_type == "GeometryCollection":
        for child in geometry.get("geometries") or []:
            yield from iter_lon_lat(child)
        return

    fail(f"unsupported street geometry type: {geometry_type!r}")


def main() -> None:
    api_key = os.environ.get("VISICOM_API_KEY", "").strip()
    if not api_key:
        fail("VISICOM_API_KEY is not set; obtain a Visicom Data API key and keep it outside Git")

    url = f"{API_BASE}/{FEATURE_ID}.json?key={quote(api_key, safe='')}"
    request = Request(url, headers={"User-Agent": "OsterConflict-R13-reference-review/1.0"})
    try:
        with urlopen(request, timeout=30) as response:
            payload = response.read()
    except Exception as exc:
        fail(f"feature request failed: {exc}")

    try:
        data = json.loads(payload.decode("utf-8"))
    except Exception as exc:
        fail(f"response is not valid UTF-8 JSON: {exc}")

    feature = find_feature_with_geometry(data)
    if not isinstance(feature, dict):
        fail("response contains no feature geometry")
    returned_id = feature_id_from_object(feature)
    if returned_id not in (None, FEATURE_ID):
        fail(f"unexpected feature id: {returned_id}")

    geometry = feature.get("geometry")
    points = list(iter_lon_lat(geometry))
    if len(points) < 2:
        fail(f"street geometry is too short: {len(points)} point(s)")

    origin_lat, origin_lon = parse_origin()
    meters_per_degree_lat = 111320.0
    meters_per_degree_lon = 111320.0 * math.cos(math.radians(origin_lat))

    local_points: list[tuple[int, float, float, float, float]] = []
    for index, (lon, lat) in enumerate(points):
        x_cm = (lon - origin_lon) * meters_per_degree_lon * 100.0
        y_cm = (lat - origin_lat) * meters_per_degree_lat * 100.0
        local_points.append((index, lon, lat, x_cm, y_cm))

    OUT_DIR.mkdir(parents=True, exist_ok=True)
    RAW_OUT.write_text(json.dumps(data, ensure_ascii=False, indent=2), encoding="utf-8")
    with CSV_OUT.open("w", encoding="utf-8", newline="") as handle:
        writer = csv.writer(handle)
        writer.writerow(["vertex", "longitude", "latitude", "local_x_cm", "local_y_cm"])
        writer.writerows(local_points)

    min_x = min(point[3] for point in local_points)
    max_x = max(point[3] for point in local_points)
    min_y = min(point[4] for point in local_points)
    max_y = max(point[4] for point in local_points)
    summary = (
        f"feature={FEATURE_ID}\n"
        f"geometry_type={geometry.get('type')}\n"
        f"vertices={len(local_points)}\n"
        f"local_bounds_cm=X[{min_x:.1f},{max_x:.1f}] Y[{min_y:.1f},{max_y:.1f}]\n"
        "status=REVIEW_ONLY; no runtime/source geometry was modified\n"
    )
    SUMMARY_OUT.write_text(summary, encoding="utf-8")

    print("R13 S01 KRUSHELNYTSKA VISICOM FETCH: PASS")
    print(summary, end="")
    print(f"raw: {RAW_OUT.relative_to(ROOT)}")
    print(f"local vertices: {CSV_OUT.relative_to(ROOT)}")


if __name__ == "__main__":
    main()
