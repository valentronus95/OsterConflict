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
    "S16B Oster fence fidelity", "street-facing yards",
    "Pass 44 removes the old Desna/Oster wetland proxies outside the compact map",
]
for m in markers:
    assert m in text, f"missing marker: {m}"

# S16B originally described a peripheral wet-meadow/reed-edge proxy. Pass 44's user-authoritative
# compact battlefield excludes that old shoreline area, so source verification must not resurrect it.
assert "wet meadow/reed-edge" not in text, "retired peripheral wetland proxy returned to active source"

print(
    f"S16B structural verification: PASS (Pass 44 compact bounds retire peripheral wetland proxy)\n"
    f"Checked {len(required)} required files and {len(markers)} S16B/Pass44 markers."
)
