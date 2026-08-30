#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
WORLD = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCWorldSectorOster.cpp"
GUARD = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCFoliageRuntimeGuardSubsystem.cpp"
UPGRADE = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCTreeContentUpgradeSubsystem.cpp"
UPGRADE_H = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public" / "OCTreeContentUpgradeSubsystem.h"


def fail(message: str) -> None:
    raise SystemExit(f"PASS45 AUTHORED TREE ASSET MAPPING FAIL: {message}")


for path in (WORLD, GUARD, UPGRADE, UPGRADE_H):
    if not path.is_file():
        fail(f"missing {path.relative_to(ROOT)}")

world = WORLD.read_text(encoding="utf-8", errors="replace")
guard = GUARD.read_text(encoding="utf-8", errors="replace")
upgrade = UPGRADE.read_text(encoding="utf-8", errors="replace")
upgrade_h = UPGRADE_H.read_text(encoding="utf-8", errors="replace")

# Authoring-stage tree identity. These are the exact tracked meshes owned by AOCWorldSectorOster before the
# one-shot Pass45 content-intake subsystem runs. Guard them separately from the final runtime intake below so CI
# cannot prove one tree family while gameplay silently replaces it with another unverified family.
authoring_expected = (
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

for component, asset_path, disk_path in authoring_expected:
    if component not in world:
        fail(f"source owner missing component {component}")
    if asset_path not in world:
        fail(f"{component} authoring mesh changed from exact authored asset {asset_path}")
    if not disk_path.is_file():
        fail(f"tracked authoring tree asset missing: {disk_path.relative_to(ROOT)}")
    if component not in guard:
        fail(f"runtime foliage guard no longer observes component {component}")

# Final gameplay mapping. UOCTreeContentUpgradeSubsystem deliberately replaces the authoring meshes at
# OnWorldBeginPlay with the imported KiteDemo family while preserving the three existing ISM owners/transforms.
# This is the mapping that a player actually sees, so it must be guarded together with the authoring-stage mapping.
runtime_expected = (
    (
        "AuthoredDeciduousTrees",
        "/Game/KiteDemo/Environments/Trees/HillTree_02/HillTree_02.HillTree_02",
        ROOT / "OsterConflict" / "Content" / "KiteDemo" / "Environments" / "Trees" / "HillTree_02" / "HillTree_02.uasset",
    ),
    (
        "AuthoredPine01Trees",
        "/Game/KiteDemo/Environments/Trees/ScotsPine_01/ScotsPine_01.ScotsPine_01",
        ROOT / "OsterConflict" / "Content" / "KiteDemo" / "Environments" / "Trees" / "ScotsPine_01" / "ScotsPine_01.uasset",
    ),
    (
        "AuthoredPine03Trees",
        "/Game/KiteDemo/Environments/Trees/ScotsPineTall_01/ScotsPineTall_01.ScotsPineTall_01",
        ROOT / "OsterConflict" / "Content" / "KiteDemo" / "Environments" / "Trees" / "ScotsPineTall_01" / "ScotsPineTall_01.uasset",
    ),
)

for component, asset_path, disk_path in runtime_expected:
    if component not in upgrade:
        fail(f"runtime tree intake no longer targets component {component}")
    if asset_path not in upgrade:
        fail(f"final runtime mapping missing exact imported tree asset {asset_path}")
    if not disk_path.is_file():
        fail(f"tracked runtime tree asset missing: {disk_path.relative_to(ROOT)}")

for needle in (
    "One-shot Pass45 content-intake upgrade",
    "preserves their placement",
):
    if needle not in upgrade_h:
        fail(f"runtime tree intake ownership contract missing {needle!r}")

for needle in (
    "UpgradeTreeFamily",
    "GetInstanceTransform",
    "UpdateInstanceTransform",
    "OldBottomZ",
    "DesiredHeight",
    "DesiredWidth",
    "Component->SetStaticMesh(NewMesh);",
    "Component->EmptyOverrideMaterials();",
    "PASS45_REGIONAL_TREE_INTAKE_FAIL",
    "PASS45_REGIONAL_TREE_INTAKE_WIRED",
    "placement_preserved=1",
    "ground_base_preserved=1",
    "height_preserved=1",
    "runtime_acceptance=0",
):
    if needle not in upgrade:
        fail(f"final runtime tree intake lost fail-honest/preservation contract {needle!r}")

# A failed imported load or transform remap must never be mislabeled as accepted runtime vegetation.
if "runtime_acceptance=1" in upgrade:
    fail("source tree intake falsely claims UE runtime acceptance")

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
    for component, _, _ in authoring_expected:
        start = world.find(f'TEXT("{component}")')
        if start < 0:
            continue
        snippet = world[start:start + 320]
        if primitive in snippet:
            fail(f"{component} regressed to primitive tree mesh {primitive}")
    if primitive in upgrade:
        fail(f"runtime tree intake regressed to primitive mesh {primitive}")

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
print("- authoring stage: SM_Tree_Var01 + Modular_Rural_Cabin Pine 01/03 are tracked and explicit")
print("- final gameplay intake: HillTree_02 + ScotsPine_01 + ScotsPineTall_01 are tracked and explicit")
print("- one-shot runtime replacement preserves owner, placement, ground base and intended height/width")
print("- runtime path and committed .uasset are guarded for both stages of every production tree family")
print("- runtime guard still rejects Cylinder/Sphere tree proxies")
print("- oak remains explicit CONTENT GAP; no exact-species claim was manufactured")
print("STATUS: SOURCE/CONTENT CONTRACT ONLY; UE 5.8 visual acceptance remains required")
