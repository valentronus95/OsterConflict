from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"

FILES = {
    "whole": SRC / "Private" / "OCR13WholeOsterArtSubsystem.cpp",
    "environment": SRC / "Private" / "OCR13EnvironmentDressingSubsystem.cpp",
    "foliage": SRC / "Private" / "OCR13FoliageDiversitySubsystem.cpp",
    "museum_h": SRC / "Public" / "OCR13MuseumReferenceSubsystem.h",
    "museum": SRC / "Private" / "OCR13MuseumReferenceSubsystem.cpp",
    "ground_h": SRC / "Public" / "OCR13GroundSurfaceSubsystem.h",
    "ground": SRC / "Private" / "OCR13GroundSurfaceSubsystem.cpp",
    "yard_h": SRC / "Public" / "OCR13ResidentialYardSubsystem.h",
    "yard": SRC / "Private" / "OCR13ResidentialYardSubsystem.cpp",
    "poles_h": SRC / "Public" / "OCR13UtilityPoleSubsystem.h",
    "poles": SRC / "Private" / "OCR13UtilityPoleSubsystem.cpp",
    "enter_h": SRC / "Public" / "OCR13EnterableHouseArtSubsystem.h",
    "enter": SRC / "Private" / "OCR13EnterableHouseArtSubsystem.cpp",
    "krush": SRC / "Private" / "OCKrushelnytskaVisualSliceSubsystem.cpp",
}


def fail(message: str) -> None:
    raise SystemExit("R13.4 VISUAL BATCH CONSOLIDATION VERIFY FAIL: " + message)


for name, path in FILES.items():
    if not path.is_file():
        fail(f"missing {name}: {path.relative_to(ROOT)}")

texts = {name: path.read_text(encoding="utf-8", errors="replace") for name, path in FILES.items()}

# All new UHT headers must preserve generated.h as the final include.
for name in ("museum_h", "ground_h", "yard_h", "poles_h", "enter_h"):
    includes = [line.strip() for line in texts[name].splitlines() if line.strip().startswith("#include")]
    if not includes or "generated.h" not in includes[-1]:
        fail(f"generated.h is not the last include in {FILES[name].name}")

# Whole-city base art owns structures/primary trees only. EnvironmentDressing owns dense grass.
whole = texts["whole"]
for token in [
    "adaptive grass delegated to EnvironmentDressing",
    'HideProxy(WorldSector, TEXT("GrassMown"))',
    'HideProxy(WorldSector, TEXT("GrassRough"))',
    'HideProxy(WorldSector, TEXT("GrassWetland"))',
    "AddHeightMatchedTreeReplacements",
    "SM_Pine_Tree_01.SM_Pine_Tree_01",
    "SM_Pine_Tree_03.SM_Pine_Tree_03",
    "SM_Pine_Tree_05.SM_Pine_Tree_05",
    'AddHouseFamily(House01, TEXT("R13_House01"))',
    'AddHouseFamily(House02, TEXT("R13_House02"))',
]:
    if token not in whole:
        fail(f"WholeOster ownership/species marker missing: {token}")

for forbidden in [
    "AddGrassReplacements(",
    "SM_House_Var01_Extra03.SM_House_Var01_Extra03",
    "SM_House_Var01_Extra05.SM_House_Var01_Extra05",
    "SM_House_Var01_Extra07.SM_House_Var01_Extra07",
]:
    if forbidden in whole:
        fail(f"WholeOster still duplicates delegated dressing responsibility: {forbidden}")

# Existing richer environment owner must still provide adaptive PN grass, exclusions and companion house dressing.
environment = texts["environment"]
for token in [
    "MownSpacingCm = 450.0f",
    "RoughSpacingCm = 600.0f",
    "WetlandSpacingCm = 700.0f",
    "MaxCellsPerZone = 2200",
    "IsExcluded(Location, Exclusions",
    "/Game/PN_FoliageCollection/Meshes/grassMesh/grass_01_01_mesh.grass_01_01_mesh",
    "SM_House_Var01_Extra%02d.SM_House_Var01_Extra%02d",
    "R13 environment dressing: grass=%d plants=%d house extras=%d yard props=%d companion trees=%d stumps=%d.",
]:
    if token not in environment:
        fail(f"EnvironmentDressing ownership marker missing: {token}")

# FoliageDiversity remains the only companion shrub/reed owner. The temporary duplicate subsystem must be gone.
foliage = texts["foliage"]
for token in [
    "Shrubs_1.Shrubs_1",
    "Shrubs_1_Single.Shrubs_1_Single",
    "Bush_1.Bush_1",
    "Cat_Tail.Cat_Tail",
    "Cat_Tail_2.Cat_Tail_2",
    "AddCompanionPines",
    "AddShrubCompanions",
    "AddWetlandReeds",
]:
    if token not in foliage:
        fail(f"FoliageDiversity ownership marker missing: {token}")

for duplicate in [
    SRC / "Public" / "OCR13FoliageDetailSubsystem.h",
    SRC / "Private" / "OCR13FoliageDetailSubsystem.cpp",
]:
    if duplicate.exists():
        fail(f"duplicate foliage owner still exists: {duplicate.relative_to(ROOT)}")

# Museum must preserve photo-driven identity beyond the generic landmark bridges.
museum = texts["museum"]
for token in [
    "R13_MuseumDarkPlinth",
    "R13_MuseumBrickAccents",
    "R13_MuseumBlueGreyUpper",
    "R13_MuseumPaleTrim",
    "R13_MuseumGreyDoors",
    "R13_MuseumApproachPath",
    "R13_MuseumSideGlazing",
    "Glass_Window.Glass_Window",
    "Pale gable outline",
    "Three shallow entrance steps",
    "Side porch/glazed volume",
    "SM_Pine_Tree_01.SM_Pine_Tree_01",
    "SM_Pine_Tree_03.SM_Pine_Tree_03",
    "SM_Pine_Tree_05.SM_Pine_Tree_05",
]:
    if token not in museum:
        fail(f"museum photo-reference marker missing: {token}")

# Ground is material-only so the spawn/vehicle collision floor remains untouched.
ground = texts["ground"]
for token in [
    "Diorama_Ground.Diorama_Ground",
    "Ground->SetMaterial(0, GroundMaterial)",
    "Do not touch Ground collision, scale, location or visibility",
]:
    if token not in ground:
        fail(f"ground-surface safety marker missing: {token}")
for forbidden in ["SetCollisionEnabled", "SetRelativeLocation", "SetWorldLocation", "SetRelativeScale3D", "SetWorldScale3D"]:
    if forbidden in ground:
        fail(f"ground material pass must not mutate gameplay floor geometry: {forbidden}")

# Unique residential detail is intentionally non-blocking and must not re-add clutter already owned by EnvironmentDressing.
yard = texts["yard"]
for token in [
    "Side_Shed.Side_Shed",
    "Outhouse_House.Outhouse_House",
    "Wheel_Barrow.Wheel_Barrow",
    "Pallet.Pallet",
    "Tire.Tire",
    "Utility_Box_1a.Utility_Box_1a",
    "SetCanEverAffectNavigation(false)",
    "base yard clutter remains owned by EnvironmentDressing",
]:
    if token not in yard:
        fail(f"unique-yard marker missing: {token}")
for forbidden in ["Log_Pile_1.Log_Pile_1", "Metal_Barrel.Metal_Barrel"]:
    if forbidden in yard:
        fail(f"yard pass duplicates EnvironmentDressing clutter: {forbidden}")

# Utility poles derive from road topology and stay visual-only.
poles = texts["poles"]
for token in [
    "Power_Pole_1.Power_Pole_1",
    'FindISM(WorldSector, TEXT("Roads"))',
    "IsInsideCompactBounds",
    "SetCollisionEnabled(ECollisionEnabled::NoCollision)",
    "Museum + FVector(-5200.0f, 2300.0f, 0.0f)",
]:
    if token not in poles:
        fail(f"utility-pole marker missing: {token}")

# Enterable house art must style surfaces/roof without replacing its authored shell openings or interiors.
enter = texts["enter"]
for token in [
    "Roof_Both_Ends_4m.Roof_Both_Ends_4m",
    "Metal_Roof.Metal_Roof",
    "Wood_Planks_Painted_Blue.Wood_Planks_Painted_Blue",
    'FindISM(House, TEXT("Shell"))',
    'FindISM(House, TEXT("YardFences"))',
    'FindISM(House, TEXT("YardPaths"))',
    "authored doors/windows/interiors preserved",
]:
    if token not in enter:
        fail(f"enterable-house art marker missing: {token}")

# Dedicated Krushelnytska slice uses the same PN family and real conifers as the rest of the map.
krush = texts["krush"]
for token in [
    "/Game/PN_FoliageCollection/Meshes/grassMesh/grass_01_01_mesh.grass_01_01_mesh",
    "/Game/PN_FoliageCollection/Meshes/grassMesh/grass_01_02_mesh.grass_01_02_mesh",
    "/Game/PN_FoliageCollection/Meshes/grassMesh/grass_01_03_mesh.grass_01_03_mesh",
    "SM_Pine_Tree_01.SM_Pine_Tree_01",
    "SM_Pine_Tree_03.SM_Pine_Tree_03",
    "SM_Pine_Tree_05.SM_Pine_Tree_05",
]:
    if token not in krush:
        fail(f"Krushelnytska unified-foliage marker missing: {token}")

# Cheap source-level delimiter sanity for every touched C++ source.
for name in ("whole", "museum", "ground", "yard", "poles", "enter", "krush"):
    text = texts[name]
    for left, right in (("(", ")"), ("{", "}"), ("[", "]")):
        if text.count(left) != text.count(right):
            fail(f"delimiter mismatch {left}{right} in {FILES[name].name}")

print("R13.4 VISUAL BATCH CONSOLIDATION VERIFY: PASS")
print("Checks single-owner grass/foliage responsibilities, real conifers, museum photo-reference details, terrain material safety, unique rural-yard props, road-derived poles, enterable-house exterior art and Krushelnytska foliage consistency.")
