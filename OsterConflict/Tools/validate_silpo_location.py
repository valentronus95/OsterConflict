from __future__ import annotations

from pathlib import Path
import math
import re
import sys

ROOT = Path(__file__).resolve().parents[2]

GEO_H = ROOT / "OsterConflict/Source/OsterConflict/Public/OCGeoReference.h"
GEO_CPP = ROOT / "OsterConflict/Source/OsterConflict/Private/OCGeoReference.cpp"
R140_H = ROOT / "OsterConflict/Source/OsterConflict/Public/OCR140SilpoPhotoModelSubsystem.h"
R140_CPP = ROOT / "OsterConflict/Source/OsterConflict/Private/OCR140SilpoPhotoModelSubsystem.cpp"
R141_H = ROOT / "OsterConflict/Source/OsterConflict/Public/OCR141SilpoDetailSubsystem.h"
R141_CPP = ROOT / "OsterConflict/Source/OsterConflict/Private/OCR141SilpoDetailSubsystem.cpp"
R142_H = ROOT / "OsterConflict/Source/OsterConflict/Public/OCR142SilpoInteriorDetailSubsystem.h"
R142_CPP = ROOT / "OsterConflict/Source/OsterConflict/Private/OCR142SilpoInteriorDetailSubsystem.cpp"
R143_H = ROOT / "OsterConflict/Source/OsterConflict/Public/OCR143SilpoFacadeIdentitySubsystem.h"
R143_CPP = ROOT / "OsterConflict/Source/OsterConflict/Private/OCR143SilpoFacadeIdentitySubsystem.cpp"
TZ = ROOT / "OsterConflict/Docs/Locations/SILPO_OSTER_TZ.md"
SCOPE = ROOT / "OsterConflict/Docs/Locations/SILPO_OSTER_BRANCH_SCOPE.md"
REFS = ROOT / "OsterConflict/SourceReferences/Locations/Silpo_Oster/README.md"
PHOTOS = ROOT / "OsterConflict/SourceReferences/Locations/Silpo_Oster/Photos"

failures: list[str] = []


def require(condition: bool, message: str) -> None:
    if not condition:
        failures.append(message)


required_files = (
    GEO_H, GEO_CPP,
    R140_H, R140_CPP,
    R141_H, R141_CPP,
    R142_H, R142_CPP,
    R143_H, R143_CPP,
    TZ, SCOPE, REFS,
)
for path in required_files:
    require(path.is_file(), f"missing required Silpo file: {path.relative_to(ROOT)}")

geo_h = GEO_H.read_text(encoding="utf-8") if GEO_H.is_file() else ""
geo_cpp = GEO_CPP.read_text(encoding="utf-8") if GEO_CPP.is_file() else ""
r140 = R140_CPP.read_text(encoding="utf-8") if R140_CPP.is_file() else ""
r141 = R141_CPP.read_text(encoding="utf-8") if R141_CPP.is_file() else ""
r142 = R142_CPP.read_text(encoding="utf-8") if R142_CPP.is_file() else ""
r143 = R143_CPP.read_text(encoding="utf-8") if R143_CPP.is_file() else ""
tz = TZ.read_text(encoding="utf-8") if TZ.is_file() else ""
scope = SCOPE.read_text(encoding="utf-8") if SCOPE.is_file() else ""
refs = REFS.read_text(encoding="utf-8") if REFS.is_file() else ""

require("static FOCGeoReferencePoint Silpo();" in geo_h, "Silpo georeference declaration missing")
require("FOCGeoReferencePoint FOCGeoReference::Silpo()" in geo_cpp, "Silpo georeference implementation missing")
require("50.948833799986254" in geo_cpp and "30.87572244094098" in geo_cpp,
        "verified Silpo WGS84 anchor changed")

lat0 = 50.948239
lon0 = 30.883865
lat = 50.948833799986254
lon = 30.87572244094098
meters_per_degree_lon = 111320.0 * math.cos(math.radians(lat0))
x_cm = (lon - lon0) * meters_per_degree_lon * 100.0
y_cm = (lat - lat0) * 111320.0 * 100.0
require(abs(x_cm - (-57107.1)) < 10.0, f"unexpected Silpo X anchor: {x_cm:.1f} cm")
require(abs(y_cm - 6621.3) < 10.0, f"unexpected Silpo Y anchor: {y_cm:.1f} cm")

for label, source in (
    ("R14.0", r140),
    ("R14.1", r141),
    ("R14.2", r142),
    ("R14.3", r143),
):
    require("OsterConflict_Runtime" in source, f"{label} runtime-map guard missing")
    require("IsFrontendOnlySession()" in source, f"{label} frontend guard missing")
    require("FOCGeoReference::Silpo()" in source, f"{label} no longer uses Silpo geo anchor")
    require("Roads" not in source and "Sidewalks" not in source,
            f"{label} must not modify road/sidewalk source families")

require("R140_SilpoPhotoModel" in r140, "R14.0 model tag missing")
require("SilpoOster_BohdanaKhmelnytskoho54" in r140, "R14.0 address identity tag missing")
require("EntranceCenterX = -1315.0f" in r140, "photo-derived left entrance position changed")
require("EntranceWidthCm = 140.0f" in r140, "public entrance width contract changed")
require(r140.count("World.SpawnActor<AOCInteractableDoor>") == 1,
        "R14.0 must spawn exactly one public entrance door")
require("SilpoEntranceMain" in r140, "main entrance tag missing")
require("World.GetNetMode() != NM_Client" in r140, "entrance authority guard missing")
require("R140Silpo_EmptyShelves" in r140, "empty shelf geometry missing")
require("R140Silpo_Checkouts" in r140, "checkout geometry missing")
require("R140Silpo_EmptyProduceIsland" in r140, "empty produce island missing")
require("R140Silpo_CeilingFixtures" in r140, "ceiling fixture geometry missing")
require("SetMobility(EComponentMobility::Movable)" in r140, "runtime-created store lights must be movable")
require(re.search(r"constexpr float BuildingLengthCm\s*=\s*3000\.0f", r140) is not None,
        "R14.0 phase-one building length changed")
require(re.search(r"constexpr float BuildingDepthCm\s*=\s*1750\.0f", r140) is not None,
        "R14.0 phase-one building depth changed")
require(re.search(r"constexpr float WallHeightCm\s*=\s*390\.0f", r140) is not None,
        "R14.0 photo-refined wall height changed")

require("R141_SilpoPhotoDetails" in r141, "R14.1 detail actor tag missing")
require("R141Silpo_SuspendedCeiling" in r141, "R14.1 suspended ceiling missing")
require("R141Silpo_CeilingGrid" in r141, "R14.1 ceiling grid missing")
require("R141Silpo_CheckoutLaneSigns" in r141, "R14.1 checkout lane signs missing")
require("R141Silpo_AsphaltCorrection" in r141, "R14.1 plain asphalt correction missing")
require("R141Silpo_SideMarketEdge" in r141, "R14.1 immediate side-market context missing")
require("R141Silpo_FacadeTrim" in r141, "R14.1 facade trim missing")

require("R142_SilpoInteriorDetails" in r142, "R14.2 interior detail actor tag missing")
require("R142Silpo_FloorTileGrout" in r142, "R14.2 floor tile grid missing")
require("R142Silpo_ShelfEndTrim" in r142, "R14.2 shelf detail missing")
require("R142Silpo_CoolerDoorGlass" in r142, "R14.2 cooler glass detail missing")
require("R142Silpo_CoolerDoorFrames" in r142, "R14.2 cooler frame detail missing")
require("R142Silpo_CheckoutBelts" in r142, "R14.2 checkout belt detail missing")
require("R142Silpo_ProduceBinDividers" in r142, "R14.2 produce-bin detail missing")
require("R142Silpo_EntranceMat" in r142, "R14.2 entrance mat missing")
require("ECollisionEnabled::NoCollision" in r142,
        "R14.2 visual detail pass must remain non-colliding")

require("R143_SilpoFacadeIdentity" in r143, "R14.3 facade identity actor tag missing")
require("/Engine/BasicShapes/Cylinder.Cylinder" in r143, "R14.3 layered oval logo mesh missing")
require("R143Silpo_LogoBlueOutline" in r143, "R14.3 blue logo outline missing")
require("R143Silpo_LogoOrangeFace" in r143, "R14.3 orange logo face missing")
require("R143Silpo_LogoText" in r143 and 'TEXT("Сільпо")' in r143,
        "R14.3 facade logo text missing")
require("R143Silpo_ParapetDarkRails" in r143, "R14.3 dark parapet rails missing")
require("R143Silpo_ParkingSign" in r143 and "R143Silpo_ParkingText" in r143,
        "R14.3 photo-supported parking sign missing")
require("ECollisionEnabled::NoCollision" in r143,
        "R14.3 facade identity details must remain non-colliding")

expected_photo_names = {
    "01_exterior_facade_front.jpg",
    "02_interior_entry_vertical.jpg",
    "03_interior_produce_aisle.jpg",
    "04_exterior_facade_sign_close.jpg",
    "05_interior_beverage_shelf.jpg",
    "06_interior_refrigerated_counter.jpg",
    "07_interior_checkout_zone.jpg",
    "08_exterior_entrance_sidewalk_oblique.jpg",
    "09_exterior_entrance_corner.jpg",
    "10_exterior_side_wall_posters.jpg",
    "11_interior_main_aisles.jpg",
    "12_exterior_entrance_close.jpg",
    "13_exterior_facade_across_road.jpg",
    "14_exterior_facade_market_side.jpg",
    "15_exterior_side_market_activity.jpg",
    "16_exterior_entrance_porch_close.jpg",
    "17_context_opposite_building_annotation.jpg",
    "18_exterior_side_facade_warm_light.jpg",
    "19_context_facade_street_wide.jpg",
    "20_context_bohdana_khmelnytskoho_street.jpg",
}

if PHOTOS.is_dir():
    actual_photos = {p.name for p in PHOTOS.iterdir() if p.is_file() and p.suffix.lower() in {".jpg", ".jpeg", ".png"}}
    require(expected_photo_names.issubset(actual_photos),
            f"photo reference pack incomplete; missing: {sorted(expected_photo_names - actual_photos)}")
    for name in expected_photo_names & actual_photos:
        require((PHOTOS / name).stat().st_size >= 1000, f"photo reference appears invalid or empty: {name}")
else:
    failures.append("Silpo Photos directory missing")

for required in (
    "silpo-oster",
    "Богдана Хмельницького, 54",
    "50.948833799986254",
    "30.87572244094098",
    "FOCGeoReference::Silpo()",
    "AOCInteractableDoor",
    "20",
    "R14.1",
    "R14.2",
    "R14.3",
):
    require(required in tz, f"Silpo TZ lost required contract: {required}")

require("silpo-oster" in scope and "main" in scope, "branch isolation statement incomplete")
require("Photos/" in refs and "20" in refs, "reference README does not document committed photo pack")

if failures:
    print("Silpo location validation failures:")
    for failure in failures:
        print(f" - {failure}")
    sys.exit(1)

print("Silpo Oster static contracts: PASS")
