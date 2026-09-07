#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
WORLD = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCWorldSectorOster.cpp"
GUARD = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCFoliageRuntimeGuardSubsystem.cpp"
RETIRED_UPGRADE = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCTreeContentUpgradeSubsystem.cpp"
RETIRED_UPGRADE_H = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public" / "OCTreeContentUpgradeSubsystem.h"


def fail(message: str) -> None:
    raise SystemExit(f"PASS45 AUTHORED TREE ASSET MAPPING FAIL: {message}")


for path in (WORLD, GUARD):
    if not path.is_file():
        fail(f"missing {path.relative_to(ROOT)}")

world = WORLD.read_text(encoding="utf-8", errors="replace")
guard = GUARD.read_text(encoding="utf-8", errors="replace")
if RETIRED_UPGRADE.exists() or RETIRED_UPGRADE_H.exists():
    fail("late tree content-upgrade owner survived primary-authoring migration")

# Exact player-facing tree identity is selected by AOCWorldSectorOster during primary authoring. Keeping a second
# late remap owner would violate PASS45 one-owner rules and can cause startup visual replacement/flicker.
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
    if component not in world:
        fail(f"source owner missing component {component}")
    if asset_path not in world:
        fail(f"{component} primary mesh changed from exact runtime asset {asset_path}")
    if not disk_path.is_file():
        fail(f"tracked authoring tree asset missing: {disk_path.relative_to(ROOT)}")
    if component not in guard:
        fail(f"runtime foliage guard no longer observes component {component}")

for needle in (
    "PASS45_REGIONAL_TREE_INTAKE_WIRED",
    "primary_authoring=1",
    "late_mutation=0",
    "runtime_acceptance=0",
):
    if needle not in world:
        fail(f"primary tree-authoring runtime evidence missing {needle!r}")

# The gameplay guard must prove the final mapping, not merely accept any non-primitive asset. Otherwise a failed
# one-shot upgrade can leave the older authoring mesh installed while PASS10 still emits READY.
for needle in (
    "FRuntimeTreeFamilyExpectation",
    "RuntimeTreeFamilies",
    "Mesh->GetPathName()",
    "ActualPath.Equals(Family.MeshPath, ESearchCase::CaseSensitive)",
    "RuntimeIdentityMismatches == 0",
    "PASS45_RUNTIME_TREE_IDENTITY_FAIL",
    "PASS45_RUNTIME_TREE_IDENTITY_READY",
    "exact_runtime_identity=1",
    "final_runtime_tree_identity_not_ready",
):
    if needle not in guard:
        fail(f"runtime foliage guard lost exact final-tree identity contract {needle!r}")

# Exact tree identity must not be mislabeled as accepted runtime vegetation. Source markers stay structural only.
if "PASS45_RUNTIME_TREE_IDENTITY_READY" in guard and "runtime_acceptance=0" not in guard:
    fail("runtime tree identity source marker lost explicit non-acceptance truth")

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
    for component, _, _ in runtime_expected:
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
print("- primary gameplay authoring: HillTree_02 + ScotsPine_01 + ScotsPineTall_01 are tracked and explicit")
print("- gameplay guard now requires those exact three final-runtime meshes before foliage READY can be emitted")
print("- late tree remap owner is physically retired; no startup transform rewrite remains")
print("- runtime path and committed .uasset are guarded for every production tree family")
print("- runtime guard still rejects Cylinder/Sphere tree proxies")
print("- oak remains explicit CONTENT GAP; no exact-species claim was manufactured")
print("STATUS: SOURCE/CONTENT CONTRACT ONLY; UE 5.8 visual acceptance remains required")
