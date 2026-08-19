from pathlib import Path
import math
import re

ROOT = Path(__file__).resolve().parent
CPP = ROOT / "OsterConflict/Source/OsterConflict/Private/OCR13SilpoPhotoModelSubsystem.cpp"
HEADER = ROOT / "OsterConflict/Source/OsterConflict/Public/OCR13SilpoPhotoModelSubsystem.h"


def fail(message: str) -> None:
    raise SystemExit(f"R13 SILPO PHOTO MODEL VERIFY FAIL: {message}")


def read(path: Path) -> str:
    if not path.exists():
        fail(f"missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8")


def require(text: str, needle: str, where: str) -> None:
    if needle not in text:
        fail(f"{where}: missing {needle!r}")


cpp = read(CPP)
header = read(HEADER)

for needle in [
    "class OSTERCONFLICT_API UOCR13SilpoPhotoModelSubsystem",
    "void ReplaceSilpo(UWorld& World);",
    "void SuppressSourceBuilding(UWorld& World);",
    "void BuildSilpo(UWorld& World);",
]:
    require(header, needle, "header")

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
    require(cpp, needle, "Silpo model")

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

origin_lat = 50.948239
origin_lon = 30.883865
lat = 50.94907
lon = 30.87621
meters_per_lon = 111320.0 * math.cos(math.radians(origin_lat))
x_cm = (lon - origin_lon) * meters_per_lon * 100.0
y_cm = (lat - origin_lat) * 111320.0 * 100.0
if not (-70000.0 <= x_cm <= 25000.0 and -25000.0 <= y_cm <= 50000.0):
    fail(f"Silpo anchor escaped compact Oster bounds: ({x_cm:.1f}, {y_cm:.1f})")

for forbidden in [".jpeg", ".jpg", ".png", "764B665D", "2CDEA871", "DBF2A257", "91665653", "5B464C76", "67E3F35C"]:
    if forbidden.lower() in cpp.lower():
        fail(f"raw reference image leaked into runtime source: {forbidden}")

delay = re.search(r"constexpr\s+float\s+SilpoPhotoModelDelaySeconds\s*=\s*([0-9.]+)f", cpp)
if not delay:
    fail("cannot find SilpoPhotoModelDelaySeconds")
if float(delay.group(1)) < 5.2:
    fail("Silpo pass must run after the existing late R13 landmark passes")

print(
    "R13 SILPO PHOTO MODEL VERIFY: PASS "
    f"(anchor {x_cm:.1f},{y_cm:.1f} cm; photo shell/parapet/entrance/logo/posters/parking; "
    "source building footprint replacement without road deletion)"
)
