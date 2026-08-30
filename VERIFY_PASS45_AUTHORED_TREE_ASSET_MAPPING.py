#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
WORLD = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCWorldSectorOster.cpp"
GUARD = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCFoliageRuntimeGuardSubsystem.cpp"


def fail(message: str) -> None:
    raise SystemExit(f"PASS45 AUTHORED TREE ASSET MAPPING FAIL: {message}")


for path in (WORLD, GUARD):
    if not path.is_file():
        fail(f"missing {path.relative_to(ROOT)}")

world = WORLD.read_text(encoding="utf-8", errors="replace")
guard = GUARD.read_text(encoding="utf-8", errors="replace")

# Exact current production mapping. Runtime path and tracked disk payload are paired deliberately so a source-only
# rename cannot manufacture a green authored-tree contract when the corresponding .uasset is not committed.
expected = (
    (
        "AuthoredDeciduousTrees",
        "/Game/AdvancedVillagePack/Meshes/SM_Tree_Var01.SM_Tree_Var01",
        ROOT / "OsterConflict" / "Content" / "AdvancedVillagePack" / "Meshes" / "SM_Tree_Var01.uasset",
    ),
    (
        "AuthoredPine01Trees",
        "/Game/Modular_Rural_Cabin/Meshes/Foliage/SM_Pine_Tree_01.SM_Pine_Tree_01",
        ROOT / "OsterConflict" / "Content" / "Modular_Rural_Cabin" / "Meshes" / "Foliage" / "SM_Pine_Tree_01.uasset",
    ),
    (
        "AuthoredPine03Trees",
        "/Game/Modular_Rural_Cabin/Meshes/Foliage/SM_Pine_Tree_03.SM_Pine_Tree_03",
        ROOT / "OsterConflict" / "Content" / "Modular_Rural_Cabin" / "Meshes" / "Foliage" / "SM_Pine_Tree_03.uasset",
    ),
)

for component, asset_path, disk_path in expected:
    if component not in world:
        fail(f"source owner missing component {component}")
    if asset_path not in world:
        fail(f"{component} is no longer bound to exact authored asset {asset_path}")
    if not disk_path.is_file():
        fail(f"tracked authored tree asset missing: {disk_path.relative_to(ROOT)}")
    if component not in guard:
        fail(f"runtime foliage guard no longer observes component {component}")

# Source identity is exact, not merely 'anything except Cylinder/Sphere'. This guard deliberately does not claim
# that the deciduous asset is an exact oak species: the canonical Pass45 contract keeps oak as a content gap until
# reference-backed content exists.
for forbidden in (
    'MakeISM(TEXT("AuthoredDeciduousTrees"), TEXT("NoCollision"))',
    'MakeISM(TEXT("AuthoredPine01Trees"), TEXT("NoCollision"))',
    'MakeISM(TEXT("AuthoredPine03Trees"), TEXT("NoCollision"))',
):
    if forbidden in world:
        fail(f"authored tree component regressed to generic/default mesh construction: {forbidden}")

for primitive in (
    "/Engine/BasicShapes/Cylinder",
    "/Engine/BasicShapes/Sphere",
):
    # Primitive assets may still exist elsewhere in historical source topology, but they may never be the exact
    # mesh argument on any of the three production-authored tree component declarations guarded above.
    for component, _, _ in expected:
        start = world.find(f'TEXT("{component}")')
        if start < 0:
            continue
        snippet = world[start:start + 320]
        if primitive in snippet:
            fail(f"{component} regressed to primitive tree mesh {primitive}")

for needle in (
    'TEXT("AuthoredDeciduousTrees")',
    'TEXT("AuthoredPine01Trees")',
    'TEXT("AuthoredPine03Trees")',
    "IsRejectedPrimitiveTreeMesh",
    "PASS45_AUTHORED_VEGETATION_READY",
    "oak_asset_verified=0",
):
    if needle not in guard:
        fail(f"runtime foliage guard lost source-authored vegetation evidence {needle!r}")

print("PASS45 AUTHORED TREE ASSET MAPPING PASS")
print("- AuthoredDeciduousTrees -> tracked SM_Tree_Var01")
print("- AuthoredPine01Trees -> tracked Modular_Rural_Cabin / SM_Pine_Tree_01")
print("- AuthoredPine03Trees -> tracked Modular_Rural_Cabin / SM_Pine_Tree_03")
print("- runtime path and committed .uasset are both guarded for every production tree family")
print("- runtime guard still rejects Cylinder/Sphere tree proxies")
print("- oak remains explicit CONTENT GAP; no exact-species claim was manufactured")
print("STATUS: SOURCE/CONTENT CONTRACT ONLY; UE 5.8 visual acceptance remains required")
