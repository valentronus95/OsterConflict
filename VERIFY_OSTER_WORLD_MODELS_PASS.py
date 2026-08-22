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
CONTENT = ROOT / "OsterConflict/Content/AdvancedVillagePack/Meshes"


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

        cpp = CPP.read_text(encoding="utf-8")
        header = HEADER.read_text(encoding="utf-8")
        owner = WORLD_OWNER.read_text(encoding="utf-8")

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
        ]
        for asset in required_assets:
            require((CONTENT / asset).is_file(), f"required authored world asset missing: {asset}")

        require_text(owner, "AOCAssetModelDecorator", "single residential presentation owner")
        require_text(owner, "HideLegacyVisualProxies", "legacy proxy visibility owner")

        require_text(header, "AddResidentialHouse", "residential variant helper")
        require_text(header, "SelectResidentialFence", "residential fence selector")
        for index in range(1, 9):
            require_text(header, f"HouseAExtra{index:02d}", "house authored extras")
        require_text(header, "HouseBExtra", "house B authored extra")
        require_text(header, "TreeD", "tree family expansion")
        require_text(header, "TreeE", "tree family expansion")
        require_text(header, "VillageFenceA", "fence family expansion")
        require_text(header, "VillageFenceB", "fence family expansion")
        require_text(header, "VillageFenceC", "fence family expansion")

        for index in range(1, 9):
            require_text(cpp, f"SM_House_Var01_Extra{index:02d}", "house extra runtime path")
        require_text(cpp, "SM_House_Var02_Extra", "house B extra runtime path")
        require_text(cpp, "SM_Tree_Var04", "tree 04 runtime path")
        require_text(cpp, "SM_Tree_Var05", "tree 05 runtime path")
        require_text(cpp, "SM_Fence_Var01", "fence 01 runtime path")
        require_text(cpp, "SM_Fence_Var02", "fence 02 runtime path")
        require_text(cpp, "SM_Fence_Var03", "fence 03 runtime path")

        # The extra mesh must share the exact location/yaw/scale of its base authored house.
        require_text(cpp, "AddMeshInstance(Extras[Seed % UE_ARRAY_COUNT(Extras)], Location, YawDegrees, Scale);",
                     "House A extra alignment")
        require_text(cpp, "AddMeshInstance(HouseBExtra, Location, YawDegrees, Scale);",
                     "House B extra alignment")

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

        # All decorator ISMs remain presentation-only; collision stays with the authoritative world owner.
        require_text(cpp, 'Component->SetCollisionProfileName(TEXT("NoCollision"));', "decorator no-collision contract")
        require_text(cpp, "Component->SetGenerateOverlapEvents(false);", "decorator overlap contract")

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
    print("- broadleaf families expand from 3 to 5 and yard fences from 1 to 4 selectable families")
    print("- Krushelnytskoi enterable-house gap and collision-aligned residential centers remain intact")
    print("- decorator models remain presentation-only/no-collision")
    print("STATUS: CODED_UNTESTED; UE runtime acceptance is still required")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
