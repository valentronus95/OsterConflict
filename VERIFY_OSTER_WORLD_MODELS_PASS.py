#!/usr/bin/env python3
"""Source contracts for the Oster world/model-only pass.

This validates repository ownership and integration structure. It deliberately does not claim
UE runtime acceptance of pivots, scale, materials, LFS hydration or final visual placement.
"""

from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parent
CPP = ROOT / "OsterConflict/Source/OsterConflict/Private/OCAssetModelDecorator.cpp"
HEADER = ROOT / "OsterConflict/Source/OsterConflict/Public/OCAssetModelDecorator.h"
WORLD_OWNER = ROOT / "OsterConflict/Source/OsterConflict/Private/OCWorldAssetModelsSubsystem.cpp"
ENTERABLE_CPP = ROOT / "OsterConflict/Source/OsterConflict/Private/OCEnterableHouse.cpp"
ENTERABLE_H = ROOT / "OsterConflict/Source/OsterConflict/Public/OCEnterableHouse.h"
CONTENT = ROOT / "OsterConflict/Content/AdvancedVillagePack/Meshes"
CABIN_PROPS = ROOT / "OsterConflict/Content/Modular_Rural_Cabin/Meshes/Props"


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def require_text(text: str, token: str, label: str) -> None:
    require(token in text, f"{label}: missing {token!r}")


def main() -> int:
    failures = []

    try:
        require(CPP.is_file(), "OCAssetModelDecorator.cpp missing")
        require(HEADER.is_file(), "OCAssetModelDecorator.h missing")
        require(WORLD_OWNER.is_file(), "OCWorldAssetModelsSubsystem.cpp missing")
        require(ENTERABLE_CPP.is_file(), "OCEnterableHouse.cpp missing")
        require(ENTERABLE_H.is_file(), "OCEnterableHouse.h missing")

        cpp = CPP.read_text(encoding="utf-8")
        header = HEADER.read_text(encoding="utf-8")
        owner = WORLD_OWNER.read_text(encoding="utf-8")
        enterable_cpp = ENTERABLE_CPP.read_text(encoding="utf-8")
        enterable_h = ENTERABLE_H.read_text(encoding="utf-8")

        required_assets = [
            "SM_House_Var01.uasset",
            "SM_House_Var02.uasset",
            *[f"SM_House_Var01_Extra{i:02d}.uasset" for i in range(1, 9)],
            "SM_House_Var02_Extra.uasset",
            "SM_Tree_Var04.uasset",
            "SM_Tree_Var05.uasset",
            "SM_Fence_Var01.uasset",
            "SM_Fence_Var02.uasset",
            "SM_Fence_Var03.uasset",
            "SM_Fence_Var04.uasset",
            *[f"SM_Bridge_Var{i:02d}.uasset" for i in range(1, 5)],
            "SM_Well.uasset",
            *[f"SM_Well_Extra{i:02d}.uasset" for i in range(1, 5)],
        ]
        for asset in required_assets:
            require((CONTENT / asset).is_file(), f"required authored world asset missing: {asset}")

        required_house_props = [
            "Old_Sofa.uasset",
            "Wooden_Table_Small.uasset",
            "Plastic_Chair.uasset",
            "Office_Chair.uasset",
            "Refrigerator_Old.uasset",
            "Wooden_Crate.uasset",
            "Metal_Barrel.uasset",
            "Wheel_Barrow.uasset",
            "Fence_Old_1_2m.uasset",
            "Side_Shed.uasset",
        ]
        for asset in required_house_props:
            require((CABIN_PROPS / asset).is_file(), f"required authored enterable-house prop missing: {asset}")

        require_text(owner, "AOCAssetModelDecorator", "single residential presentation owner")
        require_text(owner, "HideLegacyVisualProxies", "legacy proxy visibility owner")

        require_text(header, "AddResidentialHouse", "residential variant helper")
        require_text(header, "SelectResidentialFence", "residential fence selector")
        require_text(header, "SelectBridge", "bridge family selector")
        require_text(header, "AddAuthoredWell", "authored well helper")
        for index in range(1, 9):
            require_text(header, f"HouseAExtra{index:02d}", "house authored extras")
        require_text(header, "HouseBExtra", "house B authored extra")
        require_text(header, "TreeD", "tree family expansion")
        require_text(header, "TreeE", "tree family expansion")
        for suffix in "ABCD":
            require_text(header, f"VillageFence{suffix}", "fence family expansion")
            require_text(header, f"Bridge{suffix}", "bridge family expansion")
        for index in range(1, 5):
            require_text(header, f"WellExtra{index:02d}", "well authored extras")

        for index in range(1, 9):
            require_text(cpp, f"SM_House_Var01_Extra{index:02d}", "house extra runtime path")
        require_text(cpp, "SM_House_Var02_Extra", "house B extra runtime path")
        require_text(cpp, "SM_Tree_Var04", "tree 04 runtime path")
        require_text(cpp, "SM_Tree_Var05", "tree 05 runtime path")
        for index in range(1, 5):
            require_text(cpp, f"SM_Fence_Var{index:02d}", "fence runtime path")
            require_text(cpp, f"SM_Bridge_Var{index:02d}", "bridge runtime path")
            require_text(cpp, f"SM_Well_Extra{index:02d}", "well extra runtime path")

        # Authored family detail meshes share their base model transform rather than becoming new map sites.
        require_text(cpp, "AddMeshInstance(Extras[Seed % UE_ARRAY_COUNT(Extras)], Location, YawDegrees, Scale);",
                     "authored extra alignment")
        require_text(cpp, "AddMeshInstance(HouseBExtra, Location, YawDegrees, Scale);",
                     "House B extra alignment")

        # Existing bridge geography is immutable in this pass. Only the presentation family may change.
        infrastructure_match = re.search(
            r"void AOCAssetModelDecorator::BuildInfrastructureModels\(\).*?void AOCAssetModelDecorator::BuildAmbientProps",
            cpp,
            flags=re.DOTALL,
        )
        require(infrastructure_match is not None, "infrastructure model function not found")
        infrastructure = infrastructure_match.group(0)
        require_text(infrastructure, "SelectBridge(0)", "first existing bridge authored selector")
        require_text(infrastructure, "FVector(-17000.0f, -100000.0f, 0.0f)", "first existing bridge site")
        require_text(infrastructure, "SelectBridge(1)", "second existing bridge authored selector")
        require_text(infrastructure, "FVector(76000.0f, -65000.0f, 0.0f)", "second existing bridge site")
        require(infrastructure.count("SelectBridge(") == 2,
                "bridge pass must not invent additional bridge sites")

        # The enterable-house gap on Krushelnytskoi must remain reserved.
        require_text(cpp, "if (Index != 2)", "Krushelnytskoi enterable-house gap")

        # Residential visual placement must stay aligned with collision cores. Do not revive the old
        # arbitrary per-house XY jitter while collision is owned elsewhere.
        residential_match = re.search(
            r"void AOCAssetModelDecorator::BuildResidentialModels\(\).*?void AOCAssetModelDecorator::BuildVegetationModels",
            cpp,
            flags=re.DOTALL,
        )
        require(residential_match is not None, "residential model function not found")
        residential = residential_match.group(0)
        require("OffsetJitter" not in residential, "meter-scale/legacy residential jitter returned")
        require_text(residential, "const FVector Center = Block.Origin + FVector(", "collision-aligned residential centers")

        # Variety may not regress to the previous simple A/B parity placement.
        require("(HouseCounter % 2 == 0) ? HouseA : HouseB" not in residential,
                "residential visual family regressed to two-house parity")
        require_text(cpp, "switch (FMath::Abs(VariantSeed) % 5)", "five-family residential fence selector")

        # The existing residential well stays at its old site; the authored Extra is layered at the same transform.
        require_text(cpp, "AddAuthoredWell(YardB + FVector(1100, 1150, 0)", "existing well site preserved")
        require_text(cpp, "WellExtra01, WellExtra02, WellExtra03, WellExtra04", "well authored extra family")

        # All decorator ISMs remain presentation-only; collision stays with the authoritative world owner.
        require_text(cpp, 'Component->SetCollisionProfileName(TEXT("NoCollision"));', "decorator no-collision contract")
        require_text(cpp, "Component->SetGenerateOverlapEvents(false);", "decorator overlap contract")

        # Enterable-house owner now uses the already checked-in rural-cabin props instead of drawing
        # its sofa/table/chairs/fridge/fence/shed entirely from Engine Cube geometry.
        for prop_name in (
            "Old_Sofa",
            "Wooden_Table_Small",
            "Plastic_Chair",
            "Office_Chair",
            "Refrigerator_Old",
            "Wooden_Crate",
            "Metal_Barrel",
            "Wheel_Barrow",
            "Fence_Old_1_2m",
            "Side_Shed",
        ):
            require_text(enterable_cpp, prop_name, "enterable-house authored prop path")

        for member in (
            "RealSofa",
            "RealTable",
            "RealPlasticChair",
            "RealOfficeChair",
            "RealFridge",
            "RealCrate",
            "RealMetalBarrel",
            "RealWheelBarrow",
            "RealYardFence",
            "RealSideShed",
        ):
            require_text(enterable_h, member, "enterable-house model owner")

        require_text(enterable_h, "AddFittedGroundProp", "prop bounds-fitting helper")
        require_text(enterable_h, "AddFittedFenceLine", "real fence line helper")
        require_text(enterable_cpp, "const FBoxSphereBounds Bounds = Component->GetStaticMesh()->GetBounds();",
                     "prop runtime bounds fitting")
        require_text(enterable_cpp, "TargetLongestDimensionCm / NativeLongestDimension",
                     "prop uniform scale fitting")
        require_text(enterable_cpp, "DebugLabel->SetHiddenInGame(true);", "debug label hidden in production view")
        require_text(enterable_cpp, 'HouseholdFurniture->SetCollisionProfileName(TEXT("NoCollision"));',
                     "cosmetic furniture no invisible blocking collision")
        require_text(enterable_cpp, 'HouseholdElectronics->SetCollisionProfileName(TEXT("NoCollision"));',
                     "cosmetic electronics no invisible blocking collision")
        require_text(enterable_cpp, "ClearRealInteriorProps();", "interior variant real-prop rebuild")
        require_text(enterable_cpp, "SpawnInteractiveOpeningsServer();", "doors/windows interaction owner preserved")
        require_text(enterable_cpp, "AOCInteractableGate", "yard gate interaction owner preserved")

    except AssertionError as exc:
        failures.append(str(exc))

    if failures:
        print("OSTER WORLD MODELS SOURCE CONTRACT FAILED:", file=sys.stderr)
        for failure in failures:
            print(f" - {failure}", file=sys.stderr)
        return 1

    print("OSTER WORLD MODELS SOURCE CONTRACT PASS")
    print("- residential presentation remains owned by AOCAssetModelDecorator")
    print("- 9 authored house-detail assets are integrated over the two base house families")
    print("- broadleaf families expand from 3 to 5 and yard fences from 1 to 5 selectable families")
    print("- four authored bridge meshes are wired while the two existing bridge sites remain unchanged")
    print("- the existing residential well now uses its authored base + matching Extra detail family")
    print("- Krushelnytska enterable-house gap and collision-aligned residential centers remain intact")
    print("- enterable house now uses real sofa/table/chair/fridge/crate/barrel/fence/shed props when hydrated")
    print("- cosmetic household props do not leave invisible blocking collision")
    print("- interactive door/window/light/gate ownership remains intact")
    print("STATUS: CODED_UNTESTED; UE runtime acceptance is still required")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
