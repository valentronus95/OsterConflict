#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
FRESH = ROOT / "OsterConflict" / "Scripts" / "verify_production_vehicle_fresh_load.py"
DECORATOR_H = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public" / "OCAssetModelDecorator.h"
DECORATOR_CPP = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCAssetModelDecorator.cpp"
ENTERABLE_H = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public" / "OCEnterableHouse.h"
ENTERABLE_CPP = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCEnterableHouse.cpp"

errors = []


def read(path: Path) -> str:
    if not path.is_file():
        errors.append(f"missing file: {path.relative_to(ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


fresh = read(FRESH)
decorator = read(DECORATOR_H) + "\n" + read(DECORATOR_CPP)
enterable = read(ENTERABLE_H) + "\n" + read(ENTERABLE_CPP)

# BTR material readiness must include an actual imported texture dependency, not merely a non-default Material asset.
for needle in (
    'BTR_DEST = "/Game/Production/Vehicles/BTR4"',
    "def verify_btr_texture_dependencies(materials):",
    "EditorAssetLibrary.list_assets",
    "find_package_referencers_for_asset",
    "load_assets_to_confirm=True",
    "isinstance(asset, unreal.MaterialInterface)",
    "isinstance(asset, unreal.Texture)",
    "owned_material_packages",
    "referenced_textures",
    "BTR4_TEXTURE_DEPENDENCIES_READY",
    "white_default_guard=1",
    'if label == "BTR4":',
    "verify_btr_texture_dependencies(materials)",
):
    req(needle in fresh, f"BTR authored texture-dependency gate missing: {needle}")

for weak_ready in (
    'if label == "BTR4":\n            pass',
    'BTR4_TEXTURE_DEPENDENCIES_READY materials=0',
):
    req(weak_ready not in fresh, f"BTR texture gate weakened: {weak_ready}")

# User-rejected generic village residential replacements must stay retired until reference-faithful Oster assets exist.
for needle in (
    "PASS45_GENERIC_RESIDENTIAL_REPLACEMENT_RETIRED",
    "semantic_baseline=1",
    "advanced_village_houses=0",
    "village_fences=0",
    "side_sheds=0",
    "runtime_house_replacement=0",
):
    req(needle in decorator, f"generic residential retirement marker missing: {needle}")

for forbidden in (
    "BuildResidentialModels",
    "AddResidentialHouse",
    "SelectResidentialFence",
    "SM_House_Var01",
    "SM_House_Var02",
    "SM_Fence_Var01",
    "SM_Fence_Var02",
    "SM_Fence_Var03",
    "SM_Fence_Var04",
    "Side_Shed.Side_Shed",
    "RealSideShed",
):
    req(forbidden not in decorator, f"rejected generic residential runtime replacement returned: {forbidden}")

# The separate enterable-house yard owner must not reintroduce the same unreferenced fence/shed family.
for needle in (
    "PASS45_ENTERABLE_HOUSE_YARD_REFERENCE_GUARD_READY",
    "semantic_fence_baseline=1",
    "real_yard_fence=0",
    "side_shed=0",
    "unreferenced_shed=0",
):
    req(needle in enterable, f"enterable-house yard reference guard missing: {needle}")
for forbidden in (
    "RealYardFence",
    "RealSideShed",
    "Fence_Old_1_2m",
    "Side_Shed.Side_Shed",
    "AddFittedFenceLine",
):
    req(forbidden not in enterable, f"unreferenced enterable-house yard replacement returned: {forbidden}")

if errors:
    print("PASS45 CONTENT DEPENDENCY VERIFY FAIL")
    for error in errors:
        print(f" - {error}")
    raise SystemExit(1)

print("PASS45 CONTENT DEPENDENCY SOURCE CONTRACT PASS")
print("- BTR4 fresh-load readiness requires a real Material -> imported Texture dependency")
print("- non-placeholder material names alone cannot certify the white/default BTR artifact as fixed")
print("- rejected AdvancedVillage residential houses/fences and Side_Shed stay physically retired")
print("- enterable-house yard keeps its semantic fence baseline and no unreferenced shed mesh")
print("STATUS: SOURCE VERIFIED / LOCAL UE IMPORT + RUNTIME VISUAL PROOF PENDING")
