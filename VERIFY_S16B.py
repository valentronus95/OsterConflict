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
header = H.read_text()
world = C.read_text()
text = header + world
markers = [
    "WoodFences", "MetalFences", "LightSheetFences",
    "SovietPoplarTrunks", "SovietPoplarCrowns",
    "BirchTrunks", "BirchCrowns", "PineTrunks", "PineCrowns",
    "GrassMown", "GrassRough", "GrassWetland",
    "ETreeProxy::Poplar", "ETreeProxy::Birch", "ETreeProxy::Pine",
    "Pass 44 removes the old Desna/Oster wetland proxies outside the compact map",
    "PASS45_WORLD_GENERIC_RESIDENTIAL_RETIRED",
    "AddBox(Fences, Museum +",
    "AddBox(Fences, Stadium +",
    "AddBox(Fences, College +",
]
for m in markers:
    assert m in text, f"missing marker: {m}"

# S16B originally described generic street-facing private-yard fence families. Pass45 supersedes
# that approximation: reusable palette components may remain, but procedural private-yard placement
# must not return. Reference-driven Museum/Stadium/College fences remain protected above.
for stale in [
    "BuildResidentialBlocks();",
    "void AOCWorldSectorOster::BuildResidentialBlocks()",
    "BuildSolomiiKrushelnytskoiStreet();",
    "void AOCWorldSectorOster::BuildSolomiiKrushelnytskoiStreet()",
    "S16B Oster fence fidelity",
    "street-facing yards",
]:
    assert stale not in world, f"retired generic S16B fence/residential owner returned: {stale}"

# S16B originally described a peripheral wet-meadow/reed-edge proxy. Pass 44's user-authoritative
# compact battlefield excludes that old shoreline area, so source verification must not resurrect it.
assert "wet meadow/reed-edge" not in text, "retired peripheral wetland proxy returned to active source"

print(
    f"S16B structural verification: PASS (Pass44 compact bounds + Pass45 reference-driven fence ownership)\n"
    f"Checked {len(required)} required files and {len(markers)} S16B/Pass44/Pass45 markers."
)
