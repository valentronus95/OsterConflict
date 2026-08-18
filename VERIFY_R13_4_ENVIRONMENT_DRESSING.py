from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
HEADER = SRC / "Public" / "OCR13EnvironmentDressingSubsystem.h"
CPP = SRC / "Private" / "OCR13EnvironmentDressingSubsystem.cpp"
WORLD = SRC / "Private" / "OCWorldSectorOster.cpp"


def fail(message: str) -> None:
    raise SystemExit("R13.4 ENVIRONMENT DRESSING VERIFY FAIL: " + message)


for path in (HEADER, CPP, WORLD):
    if not path.is_file():
        fail(f"missing source file: {path.relative_to(ROOT)}")

header = HEADER.read_text(encoding="utf-8", errors="replace")
cpp = CPP.read_text(encoding="utf-8", errors="replace")
world = WORLD.read_text(encoding="utf-8", errors="replace")

for token in [
    "public UWorldSubsystem",
    "virtual void OnWorldBeginPlay(UWorld& InWorld) override;",
    "void ApplyEnvironmentDressing(UWorld& World);",
    "bool bApplied = false;",
]:
    if token not in header:
        fail(f"subsystem declaration missing: {token}")

includes = [line.strip() for line in header.splitlines() if line.strip().startswith("#include")]
if not includes or "generated.h" not in includes[-1]:
    fail("generated.h must remain the last include in the UHT header")

# The pass must run after the compact/R12/R13 art bridges, not race them at BeginPlay.
for token in [
    "DressingDelaySeconds = 1.60f",
    "CreateWeakLambda",
    "OsterConflict_Runtime",
    "GameMode->IsFrontendOnlySession()",
]:
    if token not in cpp:
        fail(f"delayed runtime ownership marker missing: {token}")

# Grass density must adapt to the authored area size. A fixed 5x5 carpet is the regression we are replacing.
for token in [
    "MownSpacingCm = 450.0f",
    "RoughSpacingCm = 600.0f",
    "WetlandSpacingCm = 700.0f",
    "MaxCellsPerZone = 2200",
    "Scale.X * 100.0f",
    "Scale.Y * 100.0f",
    "RequestedCells > MaxCellsPerZone",
    "FMath::Sqrt(static_cast<float>(RequestedCells)",
    "JitterX",
    "JitterY",
    "IsExcluded(Location, Exclusions",
]:
    if token not in cpp:
        fail(f"adaptive grass marker missing: {token}")

if "constexpr float Fractions[] = { -0.42f, -0.21f, 0.0f, 0.21f, 0.42f }" in cpp:
    fail("fixed Whole-Oster 5x5 grass pattern leaked into the new density pass")

for token in [
    "/Game/PN_FoliageCollection/Meshes/grassMesh/grass_01_01_mesh.grass_01_01_mesh",
    "/Game/PN_FoliageCollection/Meshes/grassMesh/grass_01_02_mesh.grass_01_02_mesh",
    "/Game/PN_FoliageCollection/Meshes/grassMesh/grass_01_03_mesh.grass_01_03_mesh",
    "/Game/PN_FoliageCollection/Meshes/groundPlantMesh/ground_01_01.ground_01_01",
    "/Game/AdvancedVillagePack/Meshes/SM_Plant.SM_Plant",
]:
    if token not in cpp:
        fail(f"committed grass/plant asset bridge missing: {token}")

# Existing complete houses stay the structural base; their matching companion meshes add visible variation.
for token in [
    "SM_House_Var01_Extra%02d.SM_House_Var01_Extra%02d",
    "Index <= 8",
    "SM_House_Var02_Extra.SM_House_Var02_Extra",
    'FindISMsInWorld(World, TEXT("R13_House01"))',
    'FindISMsInWorld(World, TEXT("R13_House02"))',
    'FindISMsInWorld(World, TEXT("R12_House01"))',
    'FindISMsInWorld(World, TEXT("R12_House02"))',
    "Extra->AddInstance(House, true)",
]:
    if token not in cpp:
        fail(f"house variation marker missing: {token}")

for token in [
    "SM_Logs_Var01.SM_Logs_Var01",
    "SM_Crate_Closed.SM_Crate_Closed",
    "SM_Barrel.SM_Barrel",
    "SM_Cart_Var02.SM_Cart_Var02",
    "SM_Well.SM_Well",
    "SM_Stonepath_Var02.SM_Stonepath_Var02",
    "SM_Tree_Var03.SM_Tree_Var03",
    "SM_Tree_Var04.SM_Tree_Var04",
    "SM_Tree_Var05.SM_Tree_Var05",
    "SM_Treestump_Var01.SM_Treestump_Var01",
    "SM_Treestump_Var02.SM_Treestump_Var02",
]:
    if token not in cpp:
        fail(f"yard/vegetation model marker missing: {token}")

# Dressing is visual-only: do not let a density pass silently rewrite nav/collision gameplay.
for token in [
    "ECollisionEnabled::NoCollision",
    "SetCanEverAffectNavigation(false)",
    "DressingRoot->SetActorEnableCollision(false)",
    "R13 environment dressing: grass=%d plants=%d house extras=%d yard props=%d companion trees=%d stumps=%d.",
]:
    if token not in cpp:
        fail(f"visual-only safety/logging marker missing: {token}")

# Protected landmark/world contracts must survive the density pass.
for token in [
    "BuildMuseumAndStadium();",
    "BuildCentralPark();",
    "BuildCollegeSector();",
    "BuildVegetation();",
]:
    if token not in world:
        fail(f"Oster source-world landmark/vegetation marker missing: {token}")

print("R13.4 ENVIRONMENT DRESSING VERIFY: PASS")
print("Checks adaptive grass density, bundled ground plants, house companion variation, restrained yard props/secondary trees, no gameplay collision and Oster landmark preservation.")
