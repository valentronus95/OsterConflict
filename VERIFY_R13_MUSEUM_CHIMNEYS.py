from pathlib import Path

ROOT = Path(__file__).resolve().parent
HEADER = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public" / "OCR13MuseumChimneyArtSubsystem.h"
SOURCE = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCR13MuseumChimneyArtSubsystem.cpp"
ASSET = ROOT / "OsterConflict" / "Content" / "Modular_Rural_Cabin" / "Meshes" / "Props" / "Stone_Chimney.uasset"

REQUIRED_SOURCE_TOKENS = [
    'FindISM(WorldSector, TEXT("LandmarkDetails"))',
    'AOCWorldSectorOster::MuseumAnchor()',
    'ExpectedMuseumChimneyCount = 2',
    'IsMuseumChimneyProxy',
    'Stone_Chimney.Stone_Chimney',
    'R13_MuseumStoneChimneys',
    'Root->SetMobility(EComponentMobility::Static)',
    'ChimneyArt->SetCollisionProfileName(TEXT("BlockAll"))',
    'ChimneyTransforms.Num() != ExpectedMuseumChimneyCount',
    'Proxy->UpdateInstanceTransform(ChimneyIndices[RestoreIndex]',
    'other LandmarkDetails untouched',
]

FORBIDDEN_SOURCE_TOKENS = [
    'Proxy->SetVisibility(false',
    'Proxy->SetCollisionEnabled(ECollisionEnabled::NoCollision)',
    'FindISM(WorldSector, TEXT("LandmarkBlocks"))',
    'FindISM(WorldSector, TEXT("LandmarkWindows"))',
    'FindISM(WorldSector, TEXT("LandmarkRoofs"))',
]


def fail(message: str) -> None:
    raise SystemExit(f"R13 MUSEUM CHIMNEYS VERIFY FAIL: {message}")


if not HEADER.is_file():
    fail(f"missing header: {HEADER.relative_to(ROOT)}")
if not SOURCE.is_file():
    fail(f"missing source: {SOURCE.relative_to(ROOT)}")
if not ASSET.is_file():
    fail(f"missing committed chimney asset: {ASSET.relative_to(ROOT)}")

source_text = SOURCE.read_text(encoding="utf-8")
for token in REQUIRED_SOURCE_TOKENS:
    if token not in source_text:
        fail(f"missing source guard/token: {token}")
for token in FORBIDDEN_SOURCE_TOKENS:
    if token in source_text:
        fail(f"unsafe broad landmark mutation present: {token}")

print("R13 MUSEUM CHIMNEYS VERIFY: PASS")
print("Checks static art-root mobility, two-proxy museum-only replacement, collision preservation and rollback safety.")
