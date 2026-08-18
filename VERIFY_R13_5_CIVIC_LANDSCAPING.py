from pathlib import Path

ROOT = Path(__file__).resolve().parent
HEADER = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public" / "OCR13CivicLandscapingSubsystem.h"
SOURCE = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCR13CivicLandscapingSubsystem.cpp"

REQUIRED_ASSETS = [
    ROOT / "OsterConflict" / "Content" / "Modular_Rural_Cabin" / "Meshes" / "Foliage" / "Shrubs_1.uasset",
    ROOT / "OsterConflict" / "Content" / "Modular_Rural_Cabin" / "Meshes" / "Foliage" / "Shrubs_1_Single.uasset",
    ROOT / "OsterConflict" / "Content" / "Modular_Rural_Cabin" / "Meshes" / "Foliage" / "Bush_1.uasset",
    ROOT / "OsterConflict" / "Content" / "Modular_Rural_Cabin" / "Meshes" / "Foliage" / "Flower_Patch_1.uasset",
    ROOT / "OsterConflict" / "Content" / "Modular_Rural_Cabin" / "Meshes" / "Foliage" / "Grass_Patch_Long.uasset",
]

REQUIRED_SOURCE_TOKENS = [
    'AOCWorldSectorOster::MuseumAnchor()',
    'AOCWorldSectorOster::ParkAnchor()',
    'AOCWorldSectorOster::CollegeAnchor()',
    'AOCWorldSectorOster::StadiumAnchor()',
    'AddMuseumGarden',
    'AddCentralParkBorders',
    'AddCollegeCampusPlanting',
    'AddStadiumPerimeterPlanting',
    'Shrubs_1.Shrubs_1',
    'Shrubs_1_Single.Shrubs_1_Single',
    'Bush_1.Bush_1',
    'Flower_Patch_1.Flower_Patch_1',
    'Grass_Patch_Long.Grass_Patch_Long',
    'Component->SetCollisionEnabled(ECollisionEnabled::NoCollision)',
    'Component->SetCanEverAffectNavigation(false)',
    'LandscapeRoot->SetActorEnableCollision(false)',
    'Root->SetMobility(EComponentMobility::Static)',
    'if (GameMode->IsFrontendOnlySession()) return;',
    'landmark collision/massing unchanged',
]

FORBIDDEN_SOURCE_TOKENS = [
    'FindISM(',
    'SetCollisionProfileName(TEXT("BlockAll"))',
    'SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics)',
    'LandmarkBlocks',
    'LandmarkRoofs',
    'LandmarkWindows',
    'LandmarkDetails',
    'StadiumGeometry',
    'ParkGeometry',
]


def fail(message: str) -> None:
    raise SystemExit(f"R13.5 CIVIC LANDSCAPING VERIFY FAIL: {message}")


if not HEADER.is_file():
    fail(f"missing header: {HEADER.relative_to(ROOT)}")
if not SOURCE.is_file():
    fail(f"missing source: {SOURCE.relative_to(ROOT)}")

source_text = SOURCE.read_text(encoding="utf-8")
for token in REQUIRED_SOURCE_TOKENS:
    if token not in source_text:
        fail(f"missing source guard/token: {token}")
for token in FORBIDDEN_SOURCE_TOKENS:
    if token in source_text:
        fail(f"civic dressing must remain visual-only and not mutate authored geometry: {token}")

for asset in REQUIRED_ASSETS:
    if not asset.is_file():
        fail(f"missing committed landscape asset: {asset.relative_to(ROOT)}")

print("R13.5 CIVIC LANDSCAPING VERIFY: PASS")
print("Checks anchor-driven museum/park/college/stadium planting, bundled meshes, frontend gating and zero collision/navigation mutation.")
