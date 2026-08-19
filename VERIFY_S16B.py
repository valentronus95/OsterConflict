from pathlib import Path

ROOT = Path(__file__).parent / "OsterConflict"
H = ROOT / "Source/OsterConflict/Public/OCWorldSectorOster.h"
C = ROOT / "Source/OsterConflict/Private/OCWorldSectorOster.cpp"
required = [
    H, C,
    ROOT / "Docs/SESSION_16B_README_UA.md",
    ROOT / "Docs/VEGETATION_FENCE_PALETTE_S16B.md",
    ROOT / "Docs/S16B_TEST_MATRIX.md",
]
for p in required:
    assert p.exists(), f"missing {p}"
text = H.read_text() + C.read_text()
markers = [
    "WoodFences", "MetalFences", "LightSheetFences",
    "SovietPoplarTrunks", "SovietPoplarCrowns",
    "BirchTrunks", "BirchCrowns", "PineTrunks", "PineCrowns",
    "GrassMown", "GrassRough", "GrassWetland",
    "ETreeProxy::Poplar", "ETreeProxy::Birch", "ETreeProxy::Pine",
    "const int32 FenceRoll = HouseCounter % 20",
    "if (FenceRoll >= 12 && FenceRoll < 17) FenceFamily = MetalFences",
    "else if (FenceRoll >= 17) FenceFamily = LightSheetFences",
    "AddGrassPatch(GrassWetland, FVector(-93000, 35000, 0)",
    "AddGrassPatch(GrassWetland, FVector( 43000,-93000, 0)",
]
for m in markers:
    assert m in text, f"missing marker: {m}"
print(f"S16B structural verification: PASS\nChecked {len(required)} required files and {len(markers)} S16B markers.")
