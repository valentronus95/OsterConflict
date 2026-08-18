from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
HEADER = SRC / "Public" / "OCR13LandmarkSiteDressingSubsystem.h"
CPP = SRC / "Private" / "OCR13LandmarkSiteDressingSubsystem.cpp"
WORLD = SRC / "Private" / "OCWorldSectorOster.cpp"
CONTENT = ROOT / "OsterConflict" / "Content"


def fail(message: str) -> None:
    raise SystemExit("R13.4 LANDMARK SITE DRESSING VERIFY FAIL: " + message)


for path in (HEADER, CPP, WORLD):
    if not path.is_file():
        fail(f"missing source file: {path.relative_to(ROOT)}")

header = HEADER.read_text(encoding="utf-8", errors="replace")
cpp = CPP.read_text(encoding="utf-8", errors="replace")
world = WORLD.read_text(encoding="utf-8", errors="replace")

for token in [
    "public UWorldSubsystem",
    "virtual void OnWorldBeginPlay(UWorld& InWorld) override;",
    "void ApplyLandmarkSiteDressing(UWorld& World);",
    "bool bApplied = false;",
]:
    if token not in header:
        fail(f"subsystem declaration missing: {token}")

includes = [line.strip() for line in header.splitlines() if line.strip().startswith("#include")]
if not includes or "generated.h" not in includes[-1]:
    fail("generated.h must remain the last include in the UHT header")

for token in [
    "DressingDelaySeconds = 2.05f",
    "OsterConflict_Runtime",
    "CreateWeakLambda",
    "GameMode->IsFrontendOnlySession()",
    "AOCWorldSectorOster::MuseumAnchor()",
    "AOCWorldSectorOster::StadiumAnchor()",
    "AOCWorldSectorOster::CollegeAnchor()",
]:
    if token not in cpp:
        fail(f"runtime/landmark ownership marker missing: {token}")

asset_markers = {
    "/Game/TileableForestRoad/Meshes/SM_Forest_Path.SM_Forest_Path":
        CONTENT / "TileableForestRoad" / "Meshes" / "SM_Forest_Path.uasset",
    "/Game/Modular_Rural_Cabin/Meshes/Foliage/Shrubs_1.Shrubs_1":
        CONTENT / "Modular_Rural_Cabin" / "Meshes" / "Foliage" / "Shrubs_1.uasset",
    "/Game/Modular_Rural_Cabin/Meshes/Foliage/Shrubs_1_Single.Shrubs_1_Single":
        CONTENT / "Modular_Rural_Cabin" / "Meshes" / "Foliage" / "Shrubs_1_Single.uasset",
    "/Game/Modular_Rural_Cabin/Meshes/Foliage/Bush_1.Bush_1":
        CONTENT / "Modular_Rural_Cabin" / "Meshes" / "Foliage" / "Bush_1.uasset",
    "/Game/Modular_Rural_Cabin/Meshes/Props/Plastic_Trash_Bin_Bin.Plastic_Trash_Bin_Bin":
        CONTENT / "Modular_Rural_Cabin" / "Meshes" / "Props" / "Plastic_Trash_Bin_Bin.uasset",
    "/Game/Modular_Rural_Cabin/Meshes/Props/Utility_Box_1a.Utility_Box_1a":
        CONTENT / "Modular_Rural_Cabin" / "Meshes" / "Props" / "Utility_Box_1a.uasset",
    "/Game/Modular_Rural_Cabin/Meshes/Props/Power_Pole_1.Power_Pole_1":
        CONTENT / "Modular_Rural_Cabin" / "Meshes" / "Props" / "Power_Pole_1.uasset",
    "/Game/Modular_Rural_Cabin/Meshes/Props/Pallet.Pallet":
        CONTENT / "Modular_Rural_Cabin" / "Meshes" / "Props" / "Pallet.uasset",
    "/Game/Modular_Rural_Cabin/Meshes/Props/Wooden_Crate_1.Wooden_Crate_1":
        CONTENT / "Modular_Rural_Cabin" / "Meshes" / "Props" / "Wooden_Crate_1.uasset",
}
for marker, path in asset_markers.items():
    if marker not in cpp:
        fail(f"bundled asset bridge missing: {marker}")
    if not path.is_file():
        fail(f"referenced bundled asset is not committed: {path.relative_to(ROOT)}")

for token in [
    "R13_MuseumApproachPath",
    "R13_LandmarkShrub01",
    "R13_LandmarkTrashBins",
    "R13_LandmarkUtilityBoxes",
    "R13_CollegePowerPoles",
    "R13_CollegeServicePallets",
    "R13_CollegeServiceCrates",
    "FVector(720.0f, 1680.0f, 12.0f), 0.0f",
    "Keep the pitch and running apron completely clear",
    "Front entrance and broad steps stay open",
]:
    if token not in cpp:
        fail(f"site-layout restraint marker missing: {token}")

for token in [
    "SetCollisionEnabled(ECollisionEnabled::NoCollision)",
    "SetGenerateOverlapEvents(false)",
    "SetCanEverAffectNavigation(false)",
    "ArtRoot->SetActorEnableCollision(false)",
    "SetCullDistances",
]:
    if token not in cpp:
        fail(f"visual-only safety marker missing: {token}")

for forbidden in [
    "Cube.Cube",
    "HideProxy(",
    "SetVisibility(false",
    "DestroyComponent(",
    "SetCollisionProfileName(TEXT(\"BlockAll\"))",
]:
    if forbidden in cpp:
        fail(f"landmark site pass must not replace/suppress gameplay geometry: {forbidden}")

for token in [
    "BuildMuseumAndStadium();",
    "BuildCentralPark();",
    "BuildCollegeSector();",
    "AddBox(StadiumGeometry",
    "AddBox(LandmarkBlocks, MainCenter",
]:
    if token not in world:
        fail(f"base landmark geometry contract missing: {token}")

print("R13.4 LANDMARK SITE DRESSING VERIFY: PASS")
print("Checks visual-only museum approach/planting, stadium spectator-edge dressing, college service/campus dressing, committed assets, no proxy suppression and no nav/collision mutation.")
