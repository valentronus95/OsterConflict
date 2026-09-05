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

# Pass45 item26 supersedes S16B Cylinder/Sphere species proxies. Preserve S16B fence/ground-cover
# semantics, but require the real authored deciduous + verified pine families now used by the world.
markers = [
    "WoodFences", "MetalFences", "LightSheetFences",
    "AuthoredDeciduousTrees", "AuthoredPine01Trees", "AuthoredPine03Trees",
    "HillTree_02", "ScotsPine_01", "ScotsPineTall_01",
    "GrassMown", "GrassRough", "GrassWetland",
    "ETreeFamily::Deciduous", "ETreeFamily::Pine", "AddGroundedTree",
    "Pass 44 removes the old Desna/Oster wetland proxies outside the compact map",
    "PASS45_WORLD_GENERIC_RESIDENTIAL_RETIRED",
    "AddBox(Fences, Museum +",
    "AddBox(Fences, Stadium +",
    "AddBox(Fences, College +",
]
for m in markers:
    assert m in text, f"missing marker: {m}"

for stale_tree in [
    "SovietPoplarTrunks", "SovietPoplarCrowns",
    "BirchTrunks", "BirchCrowns", "PineTrunks", "PineCrowns",
    "TreeTrunks", "TreeCrowns", "ETreeProxy::Poplar", "ETreeProxy::Birch", "ETreeProxy::Pine",
    "/Engine/BasicShapes/Cylinder.Cylinder", "/Engine/BasicShapes/Sphere.Sphere",
]:
    assert stale_tree not in text, f"retired primitive vegetation contract returned: {stale_tree}"

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

assert "wet meadow/reed-edge" not in text, "retired peripheral wetland proxy returned to active source"
assert "SM_Oak" not in text, "Pass45 item26 must not invent an unverified oak asset"

print(
    f"S16B structural verification: PASS (Pass44 compact bounds + Pass45 authored vegetation/reference-driven fence ownership)\n"
    f"Checked {len(required)} required files and {len(markers)} S16B/Pass44/Pass45 markers."
)
