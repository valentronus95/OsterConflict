from pathlib import Path
import math
import re

ROOT = Path(__file__).resolve().parent
CPP = ROOT / "OsterConflict/Source/OsterConflict/Private/OCR13SilpoPhotoModelSubsystem.cpp"
HEADER = ROOT / "OsterConflict/Source/OsterConflict/Public/OCR13SilpoPhotoModelSubsystem.h"
DETAIL_CPP = ROOT / "OsterConflict/Source/OsterConflict/Private/OCR13SilpoFacadeDetailSubsystem.cpp"
DETAIL_HEADER = ROOT / "OsterConflict/Source/OsterConflict/Public/OCR13SilpoFacadeDetailSubsystem.h"
SITE_CPP = ROOT / "OsterConflict/Source/OsterConflict/Private/OCR13SilpoSiteDetailSubsystem.cpp"
SITE_HEADER = ROOT / "OsterConflict/Source/OsterConflict/Public/OCR13SilpoSiteDetailSubsystem.h"
FOLIAGE_CPP = ROOT / "OsterConflict/Source/OsterConflict/Private/OCR13SilpoFoliageUpgradeSubsystem.cpp"
FOLIAGE_HEADER = ROOT / "OsterConflict/Source/OsterConflict/Public/OCR13SilpoFoliageUpgradeSubsystem.h"
PARKING_CPP = ROOT / "OsterConflict/Source/OsterConflict/Private/OCR13SilpoParkingDetailSubsystem.cpp"
PARKING_HEADER = ROOT / "OsterConflict/Source/OsterConflict/Public/OCR13SilpoParkingDetailSubsystem.h"
GLYPH_CPP = ROOT / "OsterConflict/Source/OsterConflict/Private/OCR13SilpoLogoFallbackSubsystem.cpp"
GLYPH_HEADER = ROOT / "OsterConflict/Source/OsterConflict/Public/OCR13SilpoLogoFallbackSubsystem.h"


def fail(message: str) -> None:
    raise SystemExit(f"R13 SILPO PHOTO MODEL VERIFY FAIL: {message}")


def read(path: Path) -> str:
    if not path.exists():
        fail(f"missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8")


def require(text: str, needle: str, where: str) -> None:
    if needle not in text:
        fail(f"{where}: missing {needle!r}")


def delay(text: str, name: str, where: str) -> float:
    match = re.search(rf"constexpr\s+float\s+{re.escape(name)}\s*=\s*([0-9.]+)f", text)
    if not match:
        fail(f"{where}: cannot find {name}")
    return float(match.group(1))


cpp = read(CPP)
header = read(HEADER)
detail_cpp = read(DETAIL_CPP)
detail_header = read(DETAIL_HEADER)
site_cpp = read(SITE_CPP)
site_header = read(SITE_HEADER)
foliage_cpp = read(FOLIAGE_CPP)
foliage_header = read(FOLIAGE_HEADER)
parking_cpp = read(PARKING_CPP)
parking_header = read(PARKING_HEADER)
glyph_cpp = read(GLYPH_CPP)
glyph_header = read(GLYPH_HEADER)

for needle in [
    "class OSTERCONFLICT_API UOCR13SilpoPhotoModelSubsystem",
    "void ReplaceSilpo(UWorld& World);",
    "void SuppressSourceBuilding(UWorld& World);",
    "void BuildSilpo(UWorld& World);",
]:
    require(header, needle, "base header")

for needle in [
    "constexpr double SilpoLatitude = 50.94907;",
    "constexpr double SilpoLongitude = 30.87621;",
    'TEXT("R13_SilpoPhotoModel")',
    'TEXT("R13Silpo_MainShell")',
    'TEXT("R13Silpo_SteppedParapet")',
    'TEXT("R13Silpo_EntranceFrame")',
    'TEXT("R13Silpo_EntranceGlass")',
    'TEXT("R13Silpo_AdvertisingFrames")',
    'TEXT("R13Silpo_LogoBlueBorder")',
    'TEXT("R13Silpo_LogoOrangeFace")',
    'TEXT("R13Silpo_LogoText")',
    'TEXT("СІЛЬПО")',
    'TEXT("R13Silpo_ParkingApron")',
    'TEXT("R13Silpo_ParkingLines")',
    "Model->SetActorEnableCollision(true)",
    "SuppressSourceBuilding(World);",
    "BuildSilpo(World);",
]:
    require(cpp, needle, "base Silpo model")

for family in [
    'Name == TEXT("Buildings")',
    'Name == TEXT("ResidentialRoofs")',
    'Name == TEXT("ResidentialDetails")',
    'Name == TEXT("LandmarkBlocks")',
    'Name == TEXT("LandmarkRoofs")',
    'Name == TEXT("LandmarkWindows")',
    'Name == TEXT("LandmarkDetails")',
]:
    require(cpp, family, "placeholder suppression")

if 'Name == TEXT("Roads")' in cpp or 'Name == TEXT("Sidewalks")' in cpp:
    fail("site cleanup must not delete source road/sidewalk families")

for needle in [
    "FVector(3200.0f, 1800.0f, 480.0f)",
    "Stepped front silhouette visible in the frontal and oblique references",
    "Repeated raised side parapet piers visible along the long wall",
    "Left-side entrance vestibule and tiled approach",
    "Wall-mounted shallow lamps",
]:
    require(cpp, needle, "photo silhouette")

for needle in [
    "class OSTERCONFLICT_API UOCR13SilpoFacadeDetailSubsystem",
    "void ApplyFacadeDetails(UWorld& World);",
]:
    require(detail_header, needle, "detail header")

for needle in [
    'ActorHasTag(TEXT("R13_SilpoPhotoModel"))',
    'TEXT("R13_SilpoFacadeDetailApplied")',
    'TEXT("R13SilpoDetail_LogoBlueCloud")',
    'TEXT("R13SilpoDetail_LogoOrangeCloud")',
    'TEXT("R13SilpoDetail_LogoWhiteOutline")',
    'TEXT("R13SilpoDetail_LogoText")',
    'TEXT("Сільпо")',
    'TEXT("РИБНИЙ ЧЕТВЕР")',
    'TEXT("-20%")',
    'TEXT("ЦІНА ТИЖНЯ")',
    'TEXT("СУПЕР ЦІНА")',
    'TEXT("СМАЧНА СЕРЕДА")',
    'TEXT("-15%")',
    'TEXT("ПІЦА")',
    'TEXT("СМАЧНО ЩОДНЯ")',
    'TEXT("silpo.ua")',
    'TEXT("8:00-22:00")',
    'TEXT("100 м")',
    'TEXT("R13SilpoDetail_PizzaPosterFace")',
    'Name == TEXT("R13Silpo_LogoBlueBorder")',
    'Name == TEXT("R13Silpo_LogoOrangeFace")',
    'Component->SetHiddenInGame(true, true)',
]:
    require(detail_cpp, needle, "facade detail pass")

for needle in [
    "class OSTERCONFLICT_API UOCR13SilpoSiteDetailSubsystem",
    "void ApplySiteDetails(UWorld& World);",
]:
    require(site_header, needle, "site detail header")

for needle in [
    'ActorHasTag(TEXT("R13_SilpoPhotoModel"))',
    'TEXT("R13_SilpoSiteDetailApplied")',
    'TEXT("R13SilpoSite_PosterMountingRails")',
    'TEXT("R13SilpoSite_FacadeSeams")',
    'TEXT("R13SilpoSite_EntranceHardware")',
    'TEXT("R13SilpoSite_BlueEntranceBin")',
    'TEXT("R13SilpoSite_FlowerBedSoil")',
    'TEXT("R13SilpoSite_FlowerStems")',
    'TEXT("R13SilpoSite_Shrubs")',
    'TEXT("ТЕЛЕФОНИ")',
    'TEXT("СМАРТ-ПРИСТРОЇ")',
    'TEXT("АКСЕСУАРИ")',
    'TEXT("СЕРВІС")',
    'TEXT("ЩОЧЕТВЕРГА ДІЮТЬ ЗНИЖКИ")',
    'TEXT("НА ПРОДУКЦІЮ РИБНОГО ВІДДІЛУ")',
    'TEXT("У СУПЕРМАРКЕТАХ СІЛЬПО")',
    'TEXT("ЩОСЕРЕДИ ДІЮТЬ ЗНИЖКИ")',
    'TEXT("< 100 м >")',
]:
    require(site_cpp, needle, "site detail pass")

for needle in [
    "class OSTERCONFLICT_API UOCR13SilpoFoliageUpgradeSubsystem",
    "void UpgradeFoliage(UWorld& World);",
]:
    require(foliage_header, needle, "foliage upgrade header")

for needle in [
    'TEXT("R13_SilpoFoliageUpgradeApplied")',
    '/Game/PN_FoliageCollection/Meshes/flowerMesh/flower_01_01.flower_01_01',
    '/Game/PN_FoliageCollection/Meshes/flowerMesh/flower_02_03.flower_02_03',
    '/Game/PN_FoliageCollection/Meshes/flowerMesh/flower_03_02.flower_03_02',
    '/Game/PN_FoliageCollection/Meshes/groundPlantMesh/ground_01_03.ground_01_03',
    '/Game/PN_FoliageCollection/Meshes/groundPlantMesh/ground_02_02.ground_02_02',
    'TEXT("R13SilpoFoliage_FlowerA")',
    'TEXT("R13SilpoFoliage_GroundA")',
    'Name == TEXT("R13SilpoSite_FlowerStems")',
    'Component->SetHiddenInGame(true, true)',
    "const bool bHasFlowerMesh = FlowerA || FlowerB || FlowerC;",
    "const bool bHasGroundMesh = GroundA || GroundB;",
    "if (!bHasFlowerMesh || !bHasGroundMesh)",
    "procedural fallback kept",
]:
    require(foliage_cpp, needle, "PN foliage upgrade")

for needle in [
    "class OSTERCONFLICT_API UOCR13SilpoParkingDetailSubsystem",
    "void ApplyParkingDetails(UWorld& World);",
]:
    require(parking_header, needle, "parking detail header")

for needle in [
    'TEXT("R13_SilpoParkingDetailApplied")',
    '/Game/VehicleVarietyPack/Meshes/SM_Hatchback.SM_Hatchback',
    '/Game/VehicleVarietyPack/Meshes/SM_SportsCar.SM_SportsCar',
    '/Game/VehicleVarietyPack/Meshes/SM_SUV.SM_SUV',
    '/Game/VehicleVarietyPack/Meshes/SM_Pickup.SM_Pickup',
    'TEXT("R13SilpoParking_Hatchbacks")',
    'TEXT("R13SilpoParking_Sedans")',
    'TEXT("R13SilpoParking_SUVs")',
    'TEXT("R13SilpoParking_Pickups")',
    'Component->SetCollisionEnabled(ECollisionEnabled::NoCollision)',
    'Component->SetCanEverAffectNavigation(false)',
    'const FBoxSphereBounds Bounds = Mesh->GetBounds();',
    'const float GroundedZ = ParkingSurfaceZ - MeshBottom;',
    'AddParkedCar(HatchbackCars, Hatchback, -880.0f, -1975.0f, 90.0f, 0.94f);',
    'AddParkedCar(PickupCars, Pickup, 1490.0f, -2360.0f, 88.0f, 0.90f);',
    'parking remains unobstructed',
]:
    require(parking_cpp, needle, "parking detail pass")

for needle in [
    "class OSTERCONFLICT_API UOCR13SilpoLogoFallbackSubsystem",
    "void ValidateLogo(UWorld& World);",
]:
    require(glyph_header, needle, "glyph fallback header")

for needle in [
    '#include "Engine/Font.h"',
    'TEXT("R13_SilpoLogoGlyphChecked")',
    'TEXT("R13_SilpoLogoGeometryFallback")',
    'FindText(Model, TEXT("R13SilpoDetail_LogoText"))',
    'FindText(Model, TEXT("R13SilpoDetail_LogoWhiteOutline"))',
    'const FString RequiredWord = TEXT("Сільпо");',
    'Font->RemapChar(Character) == TCHAR(0)',
    'TEXT("R13SilpoGlyph_WhiteOutline")',
    'TEXT("R13SilpoGlyph_BlueFace")',
    'BuildWord(WhiteWord, -978.0f, 1.04f, 6.0f);',
    'BuildWord(BlueWord, -988.0f, 1.0f, 0.0f);',
    'AddGlyphC(Component',
    'AddGlyphI(Component',
    'AddGlyphL(Component',
    'AddGlyphSoft(Component',
    'AddGlyphP(Component',
    'AddGlyphO(Component',
]:
    require(glyph_cpp, needle, "Cyrillic logo guard")

base_delay = delay(cpp, "SilpoPhotoModelDelaySeconds", "base model")
detail_delay = delay(detail_cpp, "SilpoFacadeDetailDelaySeconds", "detail pass")
site_delay = delay(site_cpp, "SilpoSiteDetailDelaySeconds", "site detail pass")
foliage_delay = delay(foliage_cpp, "SilpoFoliageUpgradeDelaySeconds", "foliage upgrade")
parking_delay = delay(parking_cpp, "SilpoParkingDetailDelaySeconds", "parking detail")
glyph_delay = delay(glyph_cpp, "SilpoLogoFallbackDelaySeconds", "glyph fallback")
if not base_delay < detail_delay < site_delay < foliage_delay < parking_delay < glyph_delay:
    fail(
        "Silpo passes must stay ordered base -> facade -> site -> foliage -> parking -> glyph guard: "
        f"base={base_delay}, facade={detail_delay}, site={site_delay}, foliage={foliage_delay}, "
        f"parking={parking_delay}, glyph={glyph_delay}"
    )

origin_lat = 50.948239
origin_lon = 30.883865
lat = 50.94907
lon = 30.87621
meters_per_lon = 111320.0 * math.cos(math.radians(origin_lat))
x_cm = (lon - origin_lon) * meters_per_lon * 100.0
y_cm = (lat - origin_lat) * 111320.0 * 100.0
if not (-70000.0 <= x_cm <= 25000.0 and -25000.0 <= y_cm <= 50000.0):
    fail(f"Silpo anchor escaped compact Oster bounds: ({x_cm:.1f}, {y_cm:.1f})")

combined = (
    cpp + "\n" + detail_cpp + "\n" + site_cpp + "\n" + foliage_cpp + "\n" +
    parking_cpp + "\n" + glyph_cpp
)
for forbidden in [".jpeg", ".jpg", ".png", "764B665D", "2CDEA871", "DBF2A257", "91665653", "5B464C76", "67E3F35C"]:
    if forbidden.lower() in combined.lower():
        fail(f"raw reference image leaked into runtime source: {forbidden}")

if base_delay < 5.2:
    fail("Silpo base pass must run after the existing late R13 landmark passes")

print(
    "R13 SILPO PHOTO MODEL VERIFY: PASS "
    f"(anchor {x_cm:.1f},{y_cm:.1f} cm; base {base_delay:.2f}s -> facade {detail_delay:.2f}s -> "
    f"site {site_delay:.2f}s -> foliage {foliage_delay:.2f}s -> parking {parking_delay:.2f}s -> glyph {glyph_delay:.2f}s; "
    "photo shell + cloud Сільпо sign + Ukrainian promo posters + entrance/parking signage + poster rails + entrance bin + "
    "planted strip + PN foliage upgrade/fallback + grounded visual-only VehicleVarietyPack parking row + Cyrillic logo guard; "
    "source building footprint replacement without road deletion)"
)
