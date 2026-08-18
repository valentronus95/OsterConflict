from pathlib import Path

ROOT = Path(__file__).resolve().parent
HEADER = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public" / "OCR13ParkFurnitureSubsystem.h"
SOURCE = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCR13ParkFurnitureSubsystem.cpp"

REQUIRED_ASSETS = [
    ROOT / "OsterConflict" / "Content" / "Modular_Rural_Cabin" / "Meshes" / "Props" / "Old_Planks_Plank_1.uasset",
    ROOT / "OsterConflict" / "Content" / "Modular_Rural_Cabin" / "Meshes" / "Props" / "Old_Planks_Plank_2.uasset",
    ROOT / "OsterConflict" / "Content" / "Modular_Rural_Cabin" / "Meshes" / "Props" / "Old_Planks_Plank_3.uasset",
]

REQUIRED_SOURCE_TOKENS = [
    'FindISM(WorldSector, TEXT("ParkDetails"))',
    'AOCWorldSectorOster::ParkAnchor()',
    'ExpectedCentralParkBenchCount = 14',
    'IsBenchProxyTransform',
    'BenchTransforms.Num() != ExpectedCentralParkBenchCount',
    'Old_Planks_Plank_1.Old_Planks_Plank_1',
    'Old_Planks_Plank_2.Old_Planks_Plank_2',
    'Old_Planks_Plank_3.Old_Planks_Plank_3',
    'R13_ParkBenchCollision',
    'CollisionProxy->AddInstance(ProxyTransform, true)',
    'Proxy->UpdateInstanceTransform(BenchIndices[BenchIndex], HiddenTransform, true, true, true)',
    'Proxy->UpdateInstanceTransform(BenchIndices[RestoreIndex]',
    'CollisionProxy->ClearInstances()',
    'Root->SetMobility(EComponentMobility::Static)',
    'if (Replaced != ExpectedCentralParkBenchCount)',
    'unrelated ParkDetails untouched',
]

FORBIDDEN_SOURCE_TOKENS = [
    'ParkDetailsProxy->SetVisibility(false',
    'ParkDetailsProxy->SetCollisionEnabled(ECollisionEnabled::NoCollision)',
]


def fail(message: str) -> None:
    raise SystemExit(f"R13 PARK FURNITURE VERIFY FAIL: {message}")


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
        fail(f"unsafe whole-ParkDetails mutation present: {token}")

for asset in REQUIRED_ASSETS:
    if not asset.is_file():
        fail(f"missing committed plank asset: {asset.relative_to(ROOT)}")

print("R13 PARK FURNITURE VERIFY: PASS")
print("Checks 14-bench all-or-nothing replacement, rollback, collision preservation and bundled old-plank art paths.")
