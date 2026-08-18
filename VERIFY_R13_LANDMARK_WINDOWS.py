from pathlib import Path

ROOT = Path(__file__).resolve().parent
HEADER = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public" / "OCR13LandmarkWindowArtSubsystem.h"
SOURCE = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCR13LandmarkWindowArtSubsystem.cpp"

REQUIRED_ASSETS = [
    ROOT / "OsterConflict" / "Content" / "Modular_Rural_Cabin" / "Materials" / "Instances" / "Glass_Window.uasset",
    ROOT / "OsterConflict" / "Content" / "Modular_Rural_Cabin" / "Materials" / "Instances" / "Window_Frame.uasset",
]

REQUIRED_SOURCE_TOKENS = [
    'FindISM(WorldSector, TEXT("LandmarkWindows"))',
    'Glass_Window.Glass_Window',
    'Window_Frame.Window_Frame',
    'DecodeWindowProxy',
    'BuildFramedWindows',
    'const int32 SourceCount = Proxy->GetInstanceCount()',
    'if (Replaced != SourceCount)',
    'ArtRoot->Destroy()',
    'Proxy->SetVisibility(false, true)',
    'landmark massing untouched',
]

FORBIDDEN_SOURCE_TOKENS = [
    'FindISM(WorldSector, TEXT("LandmarkBlocks"))',
    'FindISM(WorldSector, TEXT("LandmarkDetails"))',
    'FindISM(WorldSector, TEXT("LandmarkRoofs"))',
]


def fail(message: str) -> None:
    raise SystemExit(f"R13 LANDMARK WINDOWS VERIFY FAIL: {message}")


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
        fail(f"window bridge must not mutate mixed landmark family: {token}")

for asset in REQUIRED_ASSETS:
    if not asset.is_file():
        fail(f"missing committed material asset: {asset.relative_to(ROOT)}")

print("R13 LANDMARK WINDOWS VERIFY: PASS")
print("Checks all-or-nothing framed-glass replacement while preserving landmark blocks/details/roofs.")
