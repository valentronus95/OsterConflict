from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
HEADER = SRC / "Public" / "OCR13CentralParkDressingSubsystem.h"
CPP = SRC / "Private" / "OCR13CentralParkDressingSubsystem.cpp"
CONTENT = ROOT / "OsterConflict" / "Content"

REQUIRED_ASSETS = [
    CONTENT / "Modular_Rural_Cabin" / "Meshes" / "Foliage" / "Shrubs_1.uasset",
    CONTENT / "Modular_Rural_Cabin" / "Meshes" / "Foliage" / "Shrubs_1_Single.uasset",
    CONTENT / "Modular_Rural_Cabin" / "Meshes" / "Foliage" / "Bush_1.uasset",
    CONTENT / "Modular_Rural_Cabin" / "Meshes" / "Foliage" / "Flower_Patch_1.uasset",
    CONTENT / "Modular_Rural_Cabin" / "Meshes" / "Foliage" / "Grass_Patch_Long.uasset",
]

REQUIRED_TOKENS = [
    'AOCWorldSectorOster::ParkAnchor()',
    'R13_CentralParkDressingRoot',
    'R13_CentralParkShrub',
    'R13_CentralParkFlowers',
    'R13_CentralParkLongGrass',
    'Component->SetCollisionEnabled(ECollisionEnabled::NoCollision)',
    'Component->SetCanEverAffectNavigation(false)',
    'ArtRoot->SetActorEnableCollision(false)',
    'if (GameMode->IsFrontendOnlySession()) return;',
    'alleys/memorial/nav unchanged',
]

FORBIDDEN_TOKENS = [
    'MuseumAnchor()',
    'StadiumAnchor()',
    'CollegeAnchor()',
    'LandmarkBlocks',
    'StadiumGeometry',
    'ParkGeometry',
    'ParkDetails',
    'SetCollisionProfileName(TEXT("BlockAll"))',
    'SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics)',
]


def fail(message: str) -> None:
    raise SystemExit("R13.5 CENTRAL PARK DRESSING VERIFY FAIL: " + message)


for path in (HEADER, CPP):
    if not path.is_file():
        fail(f"missing source file: {path.relative_to(ROOT)}")

text = CPP.read_text(encoding="utf-8", errors="replace")
for token in REQUIRED_TOKENS:
    if token not in text:
        fail(f"missing park dressing token: {token}")
for token in FORBIDDEN_TOKENS:
    if token in text:
        fail(f"central park pass must not own other landmarks or mutate authored geometry: {token}")

for asset in REQUIRED_ASSETS:
    if not asset.is_file():
        fail(f"missing committed park asset: {asset.relative_to(ROOT)}")

print("R13.5 CENTRAL PARK DRESSING VERIFY: PASS")
print("Checks park-only planting ownership, committed foliage assets and zero collision/navigation mutation.")
