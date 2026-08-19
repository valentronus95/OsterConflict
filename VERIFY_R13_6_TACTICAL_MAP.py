from pathlib import Path
import math
import re

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
FILES = {
    "widget_h": SRC / "Public" / "OCR13TacticalMapWidget.h",
    "widget": SRC / "Private" / "OCR13TacticalMapWidget.cpp",
    "sub_h": SRC / "Public" / "OCR13TacticalMapSubsystem.h",
    "sub": SRC / "Private" / "OCR13TacticalMapSubsystem.cpp",
    "geo_h": SRC / "Public" / "OCGeoReference.h",
    "geo": SRC / "Private" / "OCGeoReference.cpp",
}


def fail(message: str) -> None:
    raise SystemExit("R13.6 TACTICAL MAP VERIFY FAIL: " + message)


for label, path in FILES.items():
    if not path.is_file():
        fail(f"missing {label}: {path.relative_to(ROOT)}")

text = {label: path.read_text(encoding="utf-8", errors="replace") for label, path in FILES.items()}
for header in ("widget_h", "sub_h"):
    includes = [line.strip() for line in text[header].splitlines() if line.strip().startswith("#include")]
    if not includes or "generated.h" not in includes[-1]:
        fail(f"{header} generated.h must remain final include")

for token in [
    "ТАКТИЧНА КАРТА  ·  ОСТЕР",
    "M / Ь  ЗАКРИТИ",
    "CompactMinX = -70000.0f",
    "CompactMaxX =  25000.0f",
    "CompactMinY = -25000.0f",
    "CompactMaxY =  50000.0f",
    'FindISM(Sector, TEXT("Roads"))',
    "MaxRoadSegmentsOnMap = 96",
    "WorldToMap",
    "AOCCapturePoint",
    "GetPointId()",
    "AOCWorldSectorOster::MuseumAnchor()",
    "AOCWorldSectorOster::ParkAnchor()",
    "AOCWorldSectorOster::CollegeAnchor()",
    "AOCWorldSectorOster::StadiumAnchor()",
    "GetOwningPlayer()",
    "Pawn->GetActorLocation()",
    "PlayerMarker->SetRenderTransformAngle",
]:
    if token not in text["widget"]:
        fail(f"tactical-map rendering marker missing: {token}")

for token in [
    "UTickableWorldSubsystem",
    "PC->IsInputKeyDown(EKeys::M)",
    "PC->IsFrontendMenuVisible()",
    "PC->IsDeploymentPanelVisible()",
    "CreateWidget<UOCR13TacticalMapWidget>",
    "Widget->AddToViewport(850)",
    "Existing->RemoveFromParent()",
    "same key labelled/typed Ь",
    "RETURN_QUICK_DECLARE_CYCLE_STAT(UOCR13TacticalMapSubsystem",
]:
    if token not in text["sub"]:
        fail(f"tactical-map toggle marker missing: {token}")

# Validate that the canonical public-map anchors rendered by the widget remain inside the exact compact-map extent.
origin_match = re.search(r"OriginLatitude\s*=\s*([-0-9.]+).*?OriginLongitude\s*=\s*([-0-9.]+)", text["geo_h"], re.S)
if not origin_match:
    fail("cannot parse FOCGeoReference origin")
origin_lat = float(origin_match.group(1))
origin_lon = float(origin_match.group(2))
meters_per_degree_lon = 111320.0 * math.cos(math.radians(origin_lat))


def parse_ref(identifier: str) -> tuple[float, float]:
    match = re.search(
        rf'TEXT\("{re.escape(identifier)}"\)\s*,\s*([-0-9.]+)\s*,\s*([-0-9.]+)',
        text["geo"],
    )
    if not match:
        fail(f"cannot parse canonical landmark reference: {identifier}")
    return float(match.group(1)), float(match.group(2))


compact_min_x, compact_max_x = -70000.0, 25000.0
compact_min_y, compact_max_y = -25000.0, 50000.0
for identifier in ("MuseumSolonyna", "CentralCityPark", "OsterCollege", "StadionOster"):
    lat, lon = parse_ref(identifier)
    x_cm = (lon - origin_lon) * meters_per_degree_lon * 100.0
    y_cm = (lat - origin_lat) * 111320.0 * 100.0
    if not (compact_min_x <= x_cm <= compact_max_x and compact_min_y <= y_cm <= compact_max_y):
        fail(f"canonical landmark {identifier} fell outside tactical map: X={x_cm:.0f} Y={y_cm:.0f} cm")

for forbidden in [
    "Server",
    "SpawnActor",
]:
    if forbidden in text["sub_h"] + text["sub"]:
        fail(f"local tactical map must not own server/gameplay spawning: {forbidden}")

print("R13.6 TACTICAL MAP VERIFY: PASS")
print("Checks physical M/Ь local toggle, compact-road schematic, A/B/C, canonical museum/park/college/stadium bounds and live player position/orientation without gameplay mutation.")
