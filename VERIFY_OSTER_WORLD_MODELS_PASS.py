#!/usr/bin/env python3
"""Current Oster world/model source contracts.

This validates repository ownership and integration structure only. It never promotes source intent
into UE runtime acceptance. Pass 45 explicitly retires the user-rejected generic residential
house/fence/Side_Shed replacement while retaining unrelated verified model integrations.
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
MUSEUM_WINDOW_CPP = ROOT / "OsterConflict/Source/OsterConflict/Private/OCMuseumBreakableWindow.cpp"
SILPO_DETAIL_CPP = ROOT / "OsterConflict/Source/OsterConflict/Private/OCR141SilpoDetailSubsystem.cpp"
CULTURE_CPP = ROOT / "OsterConflict/Source/OsterConflict/Private/OCR146CultureHousePhotoModelSubsystem.cpp"
RECOVERED_ENV = ROOT / "OsterConflict/Source/OsterConflict/Private/OCRecoveredEnvironmentSubsystem.cpp"
CONTENT = ROOT / "OsterConflict/Content/AdvancedVillagePack/Meshes"
CABIN_PROPS = ROOT / "OsterConflict/Content/Modular_Rural_Cabin/Meshes/Props"
CABIN_MODULAR = ROOT / "OsterConflict/Content/Modular_Rural_Cabin/Meshes/Modular"


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def require_text(text: str, token: str, label: str) -> None:
    require(token in text, f"{label}: missing {token!r}")


def forbid_text(text: str, token: str, label: str) -> None:
    require(token not in text, f"{label}: forbidden {token!r}")


def main() -> int:
    failures = []
    try:
        for path, label in (
            (CPP, "OCAssetModelDecorator.cpp"),
            (HEADER, "OCAssetModelDecorator.h"),
            (WORLD_OWNER, "OCWorldAssetModelsSubsystem.cpp"),
            (ENTERABLE_CPP, "OCEnterableHouse.cpp"),
            (ENTERABLE_H, "OCEnterableHouse.h"),
            (MUSEUM_WINDOW_CPP, "OCMuseumBreakableWindow.cpp"),
            (SILPO_DETAIL_CPP, "OCR141SilpoDetailSubsystem.cpp"),
            (CULTURE_CPP, "OCR146CultureHousePhotoModelSubsystem.cpp"),
            (RECOVERED_ENV, "OCRecoveredEnvironmentSubsystem.cpp"),
        ):
            require(path.is_file(), f"{label} missing")

        cpp = CPP.read_text(encoding="utf-8")
        header = HEADER.read_text(encoding="utf-8")
        owner = WORLD_OWNER.read_text(encoding="utf-8")
        enterable_cpp = ENTERABLE_CPP.read_text(encoding="utf-8")
        enterable_h = ENTERABLE_H.read_text(encoding="utf-8")
        museum_window_cpp = MUSEUM_WINDOW_CPP.read_text(encoding="utf-8")
        silpo_detail_cpp = SILPO_DETAIL_CPP.read_text(encoding="utf-8")
        culture_cpp = CULTURE_CPP.read_text(encoding="utf-8")
        recovered_env = RECOVERED_ENV.read_text(encoding="utf-8")

        # Assets still used by the decorator after residential retirement must remain present.
        required_assets = [
            "SM_Tree_Var04.uasset",
            "SM_Tree_Var05.uasset",
            *[f"SM_Bridge_Var{i:02d}.uasset" for i in range(1, 5)],
            "SM_Well.uasset",
            *[f"SM_Well_Extra{i:02d}.uasset" for i in range(1, 5)],
        ]
        for asset in required_assets:
            require((CONTENT / asset).is_file(), f"required active authored world asset missing: {asset}")

        # Interior/utility props used by still-active owners remain checked independently of the retired
        # mass residential replacement family.
        required_props = [
            "Old_Sofa.uasset",
            "Wooden_Table_Small.uasset",
            "Plastic_Chair.uasset",
            "Office_Chair.uasset",
            "Refrigerator_Old.uasset",
            "Wooden_Crate.uasset",
            "Metal_Barrel.uasset",
            "Wheel_Barrow.uasset",
            "Power_Pole_1.uasset",
        ]
        for asset in required_props:
            require((CABIN_PROPS / asset).is_file(), f"required active authored prop missing: {asset}")
        require((CABIN_MODULAR / "Window_Frame_Part.uasset").is_file(),
                "required authored window frame missing: Window_Frame_Part.uasset")

        require_text(owner, "AOCAssetModelDecorator", "world model decorator owner")
        require_text(owner, "HideLegacyVisualProxies", "legacy proxy visibility owner")

        # Pass45: user-rejected generic village residential presentation is no longer a valid Oster owner.
        for token in (
            "PASS45_GENERIC_RESIDENTIAL_REPLACEMENT_RETIRED",
            "semantic_baseline=1",
            "advanced_village_houses=0",
            "village_fences=0",
            "side_sheds=0",
            "runtime_house_replacement=0",
        ):
            require_text(cpp + header, token, "Pass45 residential retirement")
        for token in (
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
            forbid_text(cpp + header, token, "rejected decorator residential family")
        forbid_text(cpp, 'Name == TEXT("ResidentialRoofs")', "semantic residential roof baseline must stay visible")
        forbid_text(cpp, 'Name == TEXT("ResidentialDetails")', "semantic residential detail baseline must stay visible")

        # Unrelated active world-model integrations stay intact.
        require_text(header, "SelectBridge", "bridge family selector")
        require_text(header, "AddAuthoredWell", "authored well helper")
        require_text(header, "TreeD", "tree family expansion")
        require_text(header, "TreeE", "tree family expansion")
        for suffix in "ABCD":
            require_text(header, f"Bridge{suffix}", "bridge family expansion")
        for index in range(1, 5):
            require_text(header, f"WellExtra{index:02d}", "well authored extras")
            require_text(cpp, f"SM_Bridge_Var{index:02d}", "bridge runtime path")
            require_text(cpp, f"SM_Well_Extra{index:02d}", "well extra runtime path")
        require_text(cpp, "SM_Tree_Var04", "tree 04 runtime path")
        require_text(cpp, "SM_Tree_Var05", "tree 05 runtime path")

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
        require(infrastructure.count("SelectBridge(") == 2, "bridge pass must not invent additional bridge sites")

        require_text(cpp, "AddAuthoredWell(YardB + FVector(1100, 1150, 0)", "existing well site preserved")
        require_text(cpp, "WellExtra01, WellExtra02, WellExtra03, WellExtra04", "well authored extra family")
        require_text(cpp, 'Component->SetCollisionProfileName(TEXT("NoCollision"));', "decorator no-collision contract")
        require_text(cpp, "Component->SetGenerateOverlapEvents(false);", "decorator overlap contract")

        # Unverified recovered geography remains disabled.
        should_create = re.search(
            r"bool UOCRecoveredEnvironmentSubsystem::ShouldCreateSubsystem\(UObject\* Outer\) const\s*\{(.*?)\n\}",
            recovered_env,
            flags=re.DOTALL,
        )
        require(should_create is not None, "recovered environment creation guard missing")
        require("return false;" in should_create.group(1), "unverified recovered environment layer became active again")
        require_text(recovered_env, "SM_Forest_Path", "historical forest path implementation retained")
        require_text(recovered_env, "No photo, satellite, drone or georeference", "retirement rationale")

        # Enterable-house interior model integration remains active. Exterior generic yard replacements are
        # not certified here; Pass45 closes them separately against user references.
        for prop_name in (
            "Old_Sofa", "Wooden_Table_Small", "Plastic_Chair", "Office_Chair", "Refrigerator_Old",
            "Wooden_Crate", "Metal_Barrel", "Wheel_Barrow",
        ):
            require_text(enterable_cpp, prop_name, "enterable-house authored interior/utility prop path")
        for member in (
            "RealSofa", "RealTable", "RealPlasticChair", "RealOfficeChair", "RealFridge",
            "RealCrate", "RealMetalBarrel", "RealWheelBarrow",
        ):
            require_text(enterable_h, member, "enterable-house model owner")
        require_text(enterable_h, "AddFittedGroundProp", "prop bounds-fitting helper")
        require_text(enterable_cpp, "const FBoxSphereBounds Bounds = Component->GetStaticMesh()->GetBounds();",
                     "prop runtime bounds fitting")
        require_text(enterable_cpp, "TargetLongestDimensionCm / NativeLongestDimension", "prop uniform scale fitting")
        require_text(enterable_cpp, "DebugLabel->SetHiddenInGame(true);", "debug label hidden in production view")
        require_text(enterable_cpp, 'HouseholdFurniture->SetCollisionProfileName(TEXT("NoCollision"));',
                     "cosmetic furniture no invisible blocking collision")
        require_text(enterable_cpp, 'HouseholdElectronics->SetCollisionProfileName(TEXT("NoCollision"));',
                     "cosmetic electronics no invisible blocking collision")
        require_text(enterable_cpp, "ClearRealInteriorProps();", "interior variant real-prop rebuild")
        require_text(enterable_cpp, "SpawnInteractiveOpeningsServer();", "doors/windows interaction owner preserved")
        require_text(enterable_cpp, "AOCInteractableGate", "yard gate interaction owner preserved")

        # Museum breakable window current owner contract.
        require_text(museum_window_cpp, "PASS30_MUSEUM_WINDOW_FRAME_CLEAN_READY", "museum clean frame marker")
        require("Window_Frame_Part.Window_Frame_Part" not in museum_window_cpp,
                "museum must not reuse the distorted rural-cabin frame")
        require("FitAuthoredFramePart" not in museum_window_cpp, "museum must not restore axis-stretched frame fitting")
        require_text(museum_window_cpp, "Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);",
                     "museum frame visual-only collision")
        require_text(museum_window_cpp, "Component->SetCastShadow(false);", "museum frame lightweight shadow contract")
        require_text(museum_window_cpp, "GlassPane->SetMaterial(0, Glass);", "museum breakable glass material preserved")

        # Photo-supported Silpo utility pole integration remains active.
        require_text(silpo_detail_cpp, "Power_Pole_1.Power_Pole_1", "Silpo authored utility pole path")
        require_text(silpo_detail_cpp, "AddFittedVerticalMesh", "Silpo utility-pole bounds fitting")
        require_text(silpo_detail_cpp, "FQuat::FindBetweenNormals", "Silpo utility-pole axis normalization")
        require_text(silpo_detail_cpp, "R141Silpo_AuthoredUtilityPole", "Silpo authored utility-pole owner")
        require_text(silpo_detail_cpp, "PowerPoleMesh, nullptr", "Silpo authored materials preserved")
        require_text(silpo_detail_cpp, "R141Silpo_UtilityPoleCollisionProxy", "Silpo pole collision proxy")
        require_text(silpo_detail_cpp, "UtilityPoleCollision->SetHiddenInGame(true, true);", "Silpo pole hidden collision proxy")

        # Culture House frame integration remains distinct from Museum.
        require_text(culture_cpp, "Window_Frame_Part.Window_Frame_Part", "Culture House authored frame path")
        require_text(culture_cpp, "R146Culture_AuthoredWindowFrames", "Culture House authored frame owner")
        require_text(culture_cpp, "AddFittedFrameSegment", "Culture House frame bounds fitting")
        require_text(culture_cpp, "AddAuthoredRectFrame", "Culture House four-segment frame assembly")
        require_text(culture_cpp, "FQuat::FindBetweenNormals", "Culture House frame axis normalization")
        require_text(culture_cpp, "AuthoredWindowFrame, nullptr", "Culture House authored materials preserved")
        require_text(culture_cpp, "AddSideWindow(DoorFrames, AuthoredWindowFrames, Glass", "Culture House side authored frames")
        require_text(culture_cpp, "AddBox(FallbackFrames", "Culture House frame Cube fallback")
        require_text(culture_cpp, "AddBox(Glass", "Culture House glass geometry preserved")

    except AssertionError as exc:
        failures.append(str(exc))

    if failures:
        print("OSTER WORLD MODELS SOURCE CONTRACT FAILED:", file=sys.stderr)
        for failure in failures:
            print(f" - {failure}", file=sys.stderr)
        return 1

    print("OSTER WORLD MODELS SOURCE CONTRACT PASS")
    print("- Pass45 retires user-rejected generic village house/fence/Side_Shed mass replacement")
    print("- semantic residential roofs/details remain visible until reference-faithful Oster assets exist")
    print("- four authored bridge meshes retain the two established bridge sites")
    print("- current tree/well/ambient integrations remain wired without blocking collision")
    print("- unreferenced recovered forest paths remain retired")
    print("- enterable-house interior props and interaction ownership remain intact")
    print("- Museum, Silpo and Culture House specific model contracts remain intact")
    print("STATUS: CODED_UNTESTED; UE runtime acceptance is still required")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
