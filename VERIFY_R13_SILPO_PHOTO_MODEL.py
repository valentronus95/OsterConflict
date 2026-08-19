from pathlib import Path
import math
import re

ROOT = Path(__file__).resolve().parent
CPP = ROOT / "OsterConflict/Source/OsterConflict/Private/OCR13SilpoPhotoModelSubsystem.cpp"
HEADER = ROOT / "OsterConflict/Source/OsterConflict/Public/OCR13SilpoPhotoModelSubsystem.h"
DETAIL_CPP = ROOT / "OsterConflict/Source/OsterConflict/Private/OCR13SilpoFacadeDetailSubsystem.cpp"
DETAIL_HEADER = ROOT / "OsterConflict/Source/OsterConflict/Public/OCR13SilpoFacadeDetailSubsystem.h"


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

base_delay = delay(cpp, "SilpoPhotoModelDelaySeconds", "base model")
detail_delay = delay(detail_cpp, "SilpoFacadeDetailDelaySeconds", "detail pass")
if not base_delay < detail_delay:
    fail(f"facade detail pass must run after base shell: base={base_delay}, detail={detail_delay}")

origin_lat = 50.948239
origin_lon = 30.883865
lat = 50.94907
lon = 30.87621
meters_per_lon = 111320.0 * math.cos(math.radians(origin_lat))
x_cm = (lon - origin_lon) * meters_per_lon * 100.0
y_cm = (lat - origin_lat) * 111320.0 * 100.0
if not (-70000.0 <= x_cm <= 25000.0 and -25000.0 <= y_cm <= 50000.0):
    fail(f"Silpo anchor escaped compact Oster bounds: ({x_cm:.1f}, {y_cm:.1f})")

combined = cpp + "\n" + detail_cpp
for forbidden in [".jpeg", ".jpg", ".png", "764B665D", "2CDEA871", "DBF2A257", "91665653", "5B464C76", "67E3F35C"]:
    if forbidden.lower() in combined.lower():
        fail(f"raw reference image leaked into runtime source: {forbidden}")

if base_delay < 5.2:
    fail("Silpo base pass must run after the existing late R13 landmark passes")

print(
    "R13 SILPO PHOTO MODEL VERIFY: PASS "
    f"(anchor {x_cm:.1f},{y_cm:.1f} cm; base {base_delay:.2f}s -> facade {detail_delay:.2f}s; "
    "photo shell + cloud Сільпо sign + Ukrainian promo posters + entrance/parking signage; "
    "source building footprint replacement without road deletion)"
)
