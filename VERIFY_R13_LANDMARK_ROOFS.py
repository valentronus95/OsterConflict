from pathlib import Path

ROOT = Path(__file__).resolve().parent
HEADER = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public" / "OCR13LandmarkRoofArtSubsystem.h"
SOURCE = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCR13LandmarkRoofArtSubsystem.cpp"
ROOF_MATERIAL = ROOT / "OsterConflict" / "Content" / "Modular_Rural_Cabin" / "Materials" / "Instances" / "Metal_Roof.uasset"

REQUIRED_SOURCE_TOKENS = [
    'FindISM(WorldSector, TEXT("LandmarkRoofs"))',
    'AOCWorldSectorOster::MuseumAnchor()',
    'ExpectedMuseumRoofPanelCount = 8',
    'IsMuseumPitchedRoofPanel',
    'Slope >= 18.0f && Slope <= 36.0f',
    'Metal_Roof.Metal_Roof',
    'R13_MuseumMetalRoof',
    'RoofArt->SetCollisionProfileName(TEXT("BlockAll"))',
    'MuseumRoofTransforms.Num() != ExpectedMuseumRoofPanelCount',
    'Proxy->UpdateInstanceTransform(MuseumRoofIndices[RestoreIndex]',
    'flat college LandmarkRoofs untouched',
]

FORBIDDEN_SOURCE_TOKENS = [
    'Proxy->SetVisibility(false',
    'Proxy->SetCollisionEnabled(ECollisionEnabled::NoCollision)',
    'FindISM(WorldSector, TEXT("LandmarkBlocks"))',
    'FindISM(WorldSector, TEXT("LandmarkDetails"))',
    'FindISM(WorldSector, TEXT("LandmarkWindows"))',
]


def fail(message: str) -> None:
    raise SystemExit(f"R13 LANDMARK ROOFS VERIFY FAIL: {message}")


if not HEADER.is_file():
    fail(f"missing header: {HEADER.relative_to(ROOT)}")
if not SOURCE.is_file():
    fail(f"missing source: {SOURCE.relative_to(ROOT)}")
if not ROOF_MATERIAL.is_file():
    fail(f"missing committed roof material: {ROOF_MATERIAL.relative_to(ROOT)}")

source_text = SOURCE.read_text(encoding="utf-8")
for token in REQUIRED_SOURCE_TOKENS:
    if token not in source_text:
        fail(f"missing source guard/token: {token}")

for token in FORBIDDEN_SOURCE_TOKENS:
    if token in source_text:
        fail(f"unsafe broad landmark mutation present: {token}")

print("R13 LANDMARK ROOFS VERIFY: PASS")
print("Checks the eight-panel museum-only roof bridge, collision preservation and rollback safety.")
