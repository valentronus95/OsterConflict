from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
HEADER = SRC / "Public" / "OCR13LandmarkSiteDressingSubsystem.h"
CPP = SRC / "Private" / "OCR13LandmarkSiteDressingSubsystem.cpp"
WORLD = SRC / "Private" / "OCWorldSectorOster.cpp"
CIVIC = SRC / "Private" / "OCR13CivicLandscapingSubsystem.cpp"
ROADSIDE = SRC / "Private" / "OCR13RoadsideInfrastructureSubsystem.cpp"
CONTENT = ROOT / "OsterConflict" / "Content"


def fail(message: str) -> None:
    raise SystemExit("R13.5 LANDMARK SITE DRESSING VERIFY FAIL: " + message)


for path in (HEADER, CPP, WORLD, CIVIC, ROADSIDE):
    if not path.is_file():
        fail(f"missing source file: {path.relative_to(ROOT)}")

header = HEADER.read_text(encoding="utf-8", errors="replace")
cpp = CPP.read_text(encoding="utf-8", errors="replace")
world = WORLD.read_text(encoding="utf-8", errors="replace")
civic = CIVIC.read_text(encoding="utf-8", errors="replace")
roadside = ROADSIDE.read_text(encoding="utf-8", errors="replace")

for token in [
    "public UWorldSubsystem",
    "virtual void OnWorldBeginPlay(UWorld& InWorld) override;",
    "void ApplyLandmarkSiteDressing(UWorld& World);",
    "bool bApplied = false;",
    "Planting and roadside poles are owned by the dedicated R13.5 civic/roadside subsystems",
]:
    if token not in header:
        fail(f"subsystem declaration/ownership marker missing: {token}")

includes = [line.strip() for line in header.splitlines() if line.strip().startswith("#include")]
if not includes or "generated.h" not in includes[-1]:
    fail("generated.h must remain the last include in the UHT header")

for token in [
    "DressingDelaySeconds = 2.35f",
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
    "/Game/Modular_Rural_Cabin/Meshes/Props/Plastic_Trash_Bin_Bin.Plastic_Trash_Bin_Bin":
        CONTENT / "Modular_Rural_Cabin" / "Meshes" / "Props" / "Plastic_Trash_Bin_Bin.uasset",
    "/Game/Modular_Rural_Cabin/Meshes/Props/Utility_Box_1a.Utility_Box_1a":
        CONTENT / "Modular_Rural_Cabin" / "Meshes" / "Props" / "Utility_Box_1a.uasset",
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
    "R13_LandmarkTrashBins",
    "R13_LandmarkUtilityBoxes",
    "R13_CollegeServicePallets",
    "R13_CollegeServiceCrates",
    "FVector(720.0f, 1680.0f, 12.0f), 0.0f",
    "Pitch, track and goal area remain completely clear",
    "Main entrance and broad stairs around X=900 stay unobstructed",
    "civic planting and roadside poles remain separately owned",
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

# Landscaping and generic roadside poles already have dedicated owners. This pass must not duplicate them.
for forbidden in [
    "Shrubs_1.Shrubs_1",
    "Shrubs_1_Single.Shrubs_1_Single",
    "Bush_1.Bush_1",
    "Power_Pole_1.Power_Pole_1",
    "Cube.Cube",
    "HideProxy(",
    "SetVisibility(false",
    "DestroyComponent(",
    "SetCollisionProfileName(TEXT(\"BlockAll\"))",
]:
    if forbidden in cpp:
        fail(f"site pass duplicates another owner or mutates authored geometry: {forbidden}")

for token in [
    "AddMuseumGarden",
    "AddCollegeCampusPlanting",
    "AddStadiumPerimeterPlanting",
]:
    if token not in civic:
        fail(f"dedicated civic planting owner missing: {token}")
if "AddRoadsidePoles" not in roadside:
    fail("dedicated roadside pole owner missing")

for token in [
    "BuildMuseumAndStadium();",
    "BuildCentralPark();",
    "BuildCollegeSector();",
    "AddBox(StadiumGeometry",
    "AddBox(LandmarkBlocks, MainCenter",
]:
    if token not in world:
        fail(f"base landmark geometry contract missing: {token}")

print("R13.5 LANDMARK SITE DRESSING VERIFY: PASS")
print("Checks visual-only museum approach and restrained stadium/college furniture, committed assets, separate planting/pole ownership, no proxy suppression and no navigation/collision mutation.")
