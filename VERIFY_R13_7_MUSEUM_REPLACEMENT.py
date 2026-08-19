from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent
PHOTO = ROOT / "OsterConflict/Source/OsterConflict/Private/OCR137MuseumPhotoModelSubsystem.cpp"
SITE = ROOT / "OsterConflict/Source/OsterConflict/Private/OCR137MuseumSiteReplacementSubsystem.cpp"
VALIDATOR = ROOT / "OsterConflict/Source/OsterConflict/Private/OCR137MuseumRuntimeValidationSubsystem.cpp"
VALIDATOR_H = ROOT / "OsterConflict/Source/OsterConflict/Public/OCR137MuseumRuntimeValidationSubsystem.h"


def fail(message: str) -> None:
    raise SystemExit(f"R13.7 MUSEUM REPLACEMENT VERIFY FAIL: {message}")


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


photo = read(PHOTO)
site = read(SITE)
validator = read(VALIDATOR)
read(VALIDATOR_H)

site_delay = delay(site, "MuseumSiteCleanupDelaySeconds", "site cleanup")
photo_delay = delay(photo, "MuseumPhotoModelDelaySeconds", "photo model")
validation_delay = delay(validator, "MuseumValidationDelaySeconds", "runtime validator")
if not site_delay < photo_delay < validation_delay:
    fail(
        f"invalid pass ordering: cleanup={site_delay}, photo={photo_delay}, validation={validation_delay}"
    )

for needle in [
    "AOCWorldSectorOster::MuseumAnchor()",
    'TEXT("R137_MuseumPhotoModel")',
    'TEXT("R137Museum_Plinth")',
    'TEXT("R137Museum_BrickBody")',
    'TEXT("R137Museum_BlueGreyTimber")',
    'TEXT("R137Museum_SheetMetalRoof")',
    'TEXT("R137Museum_WindowGlass")',
    'TEXT("R137Museum_WindowGrilles")',
    'TEXT("R137Museum_StepsAndSlabs")',
    'TEXT("R137Museum_YellowGasPipe")',
    'TEXT("R137Museum_RearAnnex")',
    "Model->SetActorEnableCollision(true)",
    "SM_Pine_Tree_01",
    "SM_Pine_Tree_03",
]:
    require(photo, needle, "photo model")

if "Birch" in photo or "birch" in photo:
    fail("photo model reintroduced birch-specific vegetation")

for needle in [
    'StartsWith(TEXT("R13_Museum"))',
    'Name == TEXT("LandmarkBlocks")',
    'Name == TEXT("LandmarkRoofs")',
    'Name == TEXT("LandmarkWindows")',
    'Name == TEXT("LandmarkDetails")',
    'Name == TEXT("Fences")',
    "Component->SetCollisionEnabled(ECollisionEnabled::NoCollision)",
    "AOCWorldSectorOster::MuseumAnchor()",
]:
    require(site, needle, "site cleanup")

if "StadiumAnchor" in site or "BuildStadium" in site:
    fail("museum-only site cleanup must not alter the adjacent stadium")

for needle in [
    'FinalMuseumTag(TEXT("R137_MuseumPhotoModel"))',
    "FinalModelActors == 1",
    "VisibleLegacyComponents == 0",
    "SourceLandmarkInstancesNearMuseum == 0",
    "SourceFenceInstancesNearMuseum == 0",
    'TEXT("R13.7 museum validation PASS:',
    'TEXT("R13.7 museum validation FAILED:',
]:
    require(validator, needle, "runtime validator")

print(
    "R13.7 MUSEUM REPLACEMENT VERIFY: PASS "
    f"(cleanup {site_delay:.2f}s -> final {photo_delay:.2f}s -> validation {validation_delay:.2f}s)"
)
