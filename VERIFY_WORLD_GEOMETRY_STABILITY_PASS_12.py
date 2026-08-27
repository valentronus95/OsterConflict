from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict/Source/OsterConflict"

COORDINATOR = SRC / "Private/OCLandmarkStartupCoordinatorSubsystem.cpp"
SEPARATION = SRC / "Private/OCR146LandmarkSeparationSubsystem.cpp"
MUSEUM_MODEL = SRC / "Private/OCR137MuseumPhotoModelSubsystem.cpp"
CULTURE_MODEL = SRC / "Private/OCR146CultureHousePhotoModelSubsystem.cpp"
GEO_REFERENCE = SRC / "Private/OCGeoReference.cpp"
VALIDATOR_H = SRC / "Public/OCWorldGeometryStabilitySubsystem.h"
VALIDATOR_CPP = SRC / "Private/OCWorldGeometryStabilitySubsystem.cpp"
SURFACE_UPGRADE = SRC / "Private/OCAuthoredWorldSurfaceUpgradeSubsystem.cpp"
VISUAL_ENVIRONMENT = SRC / "Private/OCVisualEnvironment.cpp"
WORLD_SECTOR = SRC / "Private/OCWorldSectorOster.cpp"
ENGINE_CONFIG = ROOT / "OsterConflict/Config/DefaultEngine.ini"
LAUNCHER = ROOT / "RUN_R14_WORLD_STABILITY_RUNTIME_ACCEPTANCE.cmd"

RETIRED_GENERIC_OWNERS = (
    SRC / "Public/OCWorldAssetModelsSubsystem.h",
    SRC / "Private/OCWorldAssetModelsSubsystem.cpp",
    SRC / "Public/OCAssetModelDecorator.h",
    SRC / "Private/OCAssetModelDecorator.cpp",
    SRC / "Public/OCRecoveredEnvironmentSubsystem.h",
    SRC / "Private/OCRecoveredEnvironmentSubsystem.cpp",
    SRC / "Public/OCRecoveredBuildingDetailsSubsystem.h",
    SRC / "Private/OCRecoveredBuildingDetailsSubsystem.cpp",
    SRC / "Public/OCRecoveredRoadsidePropsSubsystem.h",
    SRC / "Private/OCRecoveredRoadsidePropsSubsystem.cpp",
)


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS12 WORLD STABILITY VERIFY FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8")


def require(text: str, needle: str, where: str) -> None:
    if needle not in text:
        raise SystemExit(f"PASS12 WORLD STABILITY VERIFY FAIL: {where}: missing {needle!r}")


def forbid(text: str, needle: str, where: str) -> None:
    if needle in text:
        raise SystemExit(f"PASS12 WORLD STABILITY VERIFY FAIL: {where}: forbidden {needle!r}")


def function_slice(source: str, signature: str, next_signature: str) -> str:
    start = source.find(signature)
    end = source.find(next_signature, start + len(signature))
    if start < 0 or end < 0:
        raise SystemExit(
            f"PASS12 WORLD STABILITY VERIFY FAIL: cannot isolate {signature!r} before {next_signature!r}"
        )
    return source[start:end]


def has_semantic_material_write(source: str) -> bool:
    """Detect a SetMaterial write tied locally to a tracked world-sector surface family."""
    semantic_tokens = (
        'TEXT("Ground")',
        'TEXT("Roads")',
        'TEXT("Sidewalks")',
        'TEXT("ParkPaths")',
        'TEXT("Fences")',
    )
    for match in re.finditer(r"\bSetMaterial\s*\(", source):
        start = max(0, match.start() - 1800)
        end = min(len(source), match.end() + 500)
        window = source[start:end]
        if "AOCWorldSectorOster" in window and any(token in window for token in semantic_tokens):
            return True
    return False


coordinator = read(COORDINATOR)
separation = read(SEPARATION)
museum_model = read(MUSEUM_MODEL)
culture_model = read(CULTURE_MODEL)
geo_reference = read(GEO_REFERENCE)
header = read(VALIDATOR_H)
validator = read(VALIDATOR_CPP)
surface_upgrade = read(SURFACE_UPGRADE)
visual_environment = read(VISUAL_ENVIRONMENT)
world_sector = read(WORLD_SECTOR)
engine_config = read(ENGINE_CONFIG)
launcher = read(LAUNCHER)

# Pass45 runtime evidence rejected the old generic AdvancedVillagePack/recovered presentation owners.
for path in RETIRED_GENERIC_OWNERS:
    if path.exists():
        raise SystemExit(
            f"PASS12 WORLD STABILITY VERIFY FAIL: rejected generic/recovered owner resurrected: {path.relative_to(ROOT)}"
        )

# Pass45 P0 black-world correction is one coherent exposure/lighting contract.
for needle in (
    "bReplicates = true;",
    "bAlwaysRelevant = true;",
    "CreateDefaultSubobject<UDirectionalLightComponent>",
    "CreateDefaultSubobject<USkyLightComponent>",
    "CreateDefaultSubobject<USkyAtmosphereComponent>",
    "SunLight->SetIntensity(120000.0f);",
    "PASS45_DAYLIGHT_EXPOSURE_CONTRACT_READY",
    "expected_auto_exposure=1",
):
    require(visual_environment, needle, "Pass45 component-owned daylight contract")
for stale in (
    "SunLight->SetIntensity(4.0f);",
    "SpawnActor<ADirectionalLight>",
    "SpawnActor<ASkyLight>",
    "SpawnActor<AExponentialHeightFog>",
):
    forbid(visual_environment, stale, "retired black-world lighting contract")
require(engine_config, "r.DefaultFeature.AutoExposure=True", "Pass45 renderer exposure contract")
require(engine_config, "r.DefaultFeature.AutoExposure.ExtendDefaultLuminanceRange=True", "Pass45 EV100 exposure contract")
forbid(engine_config, "r.DefaultFeature.AutoExposure=False", "Pass45 renderer exposure contract")
forbid(engine_config, "r.DefaultFeature.AutoExposure.ExtendDefaultLuminanceRange=False", "Pass45 EV100 exposure contract")

# AOCWorldSectorOster still owns deterministic initial Cube transforms. Item 31 upgrades verified surface families
# before the 12-second baseline. Ground now has an explicit committed landscape material; the ISM road/path/fence
# families must continue to use the authored meshes' packaged materials rather than receiving runtime recolors.
for needle in (
    "/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial",
    "Tint(Ground",
    "Tint(Roads",
    "Tint(Sidewalks",
):
    require(world_sector, needle, "initial semantic world-material owner")

for needle in (
    "/Game/AdvancedVillagePack/Meshes/SM_Plane_1x1.SM_Plane_1x1",
    "/Game/AdvancedVillagePack/Materials/M_Inst_Landscape.M_Inst_Landscape",
    "/Game/Scene_RoadsideConstruction/Assets/Custom/Urb_Roa_Asphalt_01/SM_Urb_Roa_Asphalt_01.SM_Urb_Roa_Asphalt_01",
    "/Game/Scene_RoadsideConstruction/Assets/Custom/Urb_Roa_Sidewalk_01/SM_Urb_Roa_Sidewalk_01.SM_Urb_Roa_Sidewalk_01",
    "/Game/AdvancedVillagePack/Meshes/SM_Stonepath_Var01.SM_Stonepath_Var01",
    "/Game/AdvancedVillagePack/Meshes/SM_Fence_Var01.SM_Fence_Var01",
    "UpgradeGroundSurface",
    "UpgradeCubeFamily",
    "Component->SetStaticMesh(AuthoredMesh);",
    "Component->EmptyOverrideMaterials();",
    "Component->SetMaterial(0, AuthoredMaterial);",
    "ground_top_z_preserved=1",
    "playable_footprint_preserved=1",
    "ElapsedSeconds < 0.75f",
    "PASS45_AUTHORED_GROUND_SURFACE_READY",
    "PASS45_AUTHORED_ROAD_SURFACE_READY",
    "PASS45_AUTHORED_PARK_PATH_SURFACE_READY",
    "PASS45_AUTHORED_WORLD_FENCE_READY",
    "basicshape_meshes=0",
    "basicshape_material_overrides=0",
    "topology_preserved=1",
    "pass12_baseline_deadline_s=12",
):
    require(surface_upgrade, needle, "item31 authored surface upgrade")

cube_family = function_slice(
    surface_upgrade,
    "bool UpgradeCubeFamily(",
    "}\n\nbool UOCAuthoredWorldSurfaceUpgradeSubsystem::ShouldCreateSubsystem",
)
for stale in (
    "SetMaterial(",
    "BasicShapeMaterial.BasicShapeMaterial",
):
    forbid(cube_family, stale, "authored ISM surface upgrade must preserve packaged materials")

for cpp_path in (SRC / "Private").glob("*.cpp"):
    if cpp_path in (WORLD_SECTOR, SURFACE_UPGRADE) or cpp_path.name == "OCR13StadiumSurfaceSubsystem.cpp":
        continue
    source = read(cpp_path)
    if has_semantic_material_write(source):
        raise SystemExit(
            "PASS12 WORLD STABILITY VERIFY FAIL: second semantic world-material owner detected: "
            f"{cpp_path.relative_to(ROOT)}"
        )

# Gate D identity contract remains unchanged.
for needle in (
    'Model->Tags.Add(TEXT("R137_MuseumPhotoModel"));',
    'TEXT("R137Museum_BrickBody")',
    'TEXT("R137Museum_BlueGreyTimber")',
    'TEXT("R137Museum_SheetMetalRoof")',
    'FVector(1700.0f, 840.0f, 320.0f)',
    "PASS45_MUSEUM_R137_PRIMARY_EXTERIOR_READY",
):
    require(museum_model, needle, "R13.7 Museum identity source")
for forbidden in (
    "R137Museum_Columns",
    "ColumnXs[]",
    "six-column facade",
):
    forbid(museum_model, forbidden, "Museum must not encode Culture House column identity")

for needle in (
    'Model->Tags.Add(TEXT("R146_CultureHouseAuthoritative"));',
    'Model->Tags.Add(TEXT("CultureHouseOster_Hranovskoho3"));',
    'TEXT("R146Culture_Columns")',
    "const float ColumnXs[] = { -1130.0f, -680.0f, -230.0f, 230.0f, 680.0f, 1130.0f };",
    "six-column facade",
):
    require(culture_model, needle, "R14.6 Culture House identity source")
require(geo_reference, 'TEXT("MuseumSolonyna"), 50.948239, 30.883865', "Museum geo identity")
require(geo_reference, 'TEXT("OsterCultureHouse"), 50.948694, 30.881435', "Culture House geo identity")

for needle in (
    "SetTimerForNextTick",
    "RunAuthoritativeStartup",
    "Timers.ClearAllTimersForObject(Stage);",
    "RunAuthoritativeBuildNow(World)",
    "RunAuthoritativeDetailNow(World)",
    "historical delayed reveal timers were cancelled",
):
    require(coordinator, needle, "landmark startup coordinator")

for needle in (
    "constexpr float ValidationDelaySeconds",
    "ValidateSeparation",
    "PASS45_LANDMARK_SEPARATION_VALIDATION_SCHEDULED",
    "PASS45_LANDMARK_SEPARATION_VALIDATION_READY",
    "PASS45_LANDMARK_SEPARATION_VALIDATION_FAIL",
    "PASS45_LANDMARK_IDENTITY_VALIDATION_READY",
    "PASS45_LANDMARK_IDENTITY_VALIDATION_FAIL",
    'MuseumOwnerTag(TEXT("R137_MuseumPhotoModel"))',
    'CultureOwnerTag(TEXT("R146_CultureHouseAuthoritative"))',
    "MinimumMuseumCultureSeparationCm = 10000.0f",
    "CultureIdentityInstancesAtMuseum == 0",
    "MuseumIdentityInstancesAtCulture == 0",
    "CultureColumnShafts == 6",
    "CultureOwnerAnchorErrorCm <= 100.0f",
    "mutation=0",
    "periodic_scan=0",
    "primary_authoring_fix_required=1",
):
    require(separation, needle, "Pass45 validation-only landmark separation/identity")
for stale in (
    "RunStartupGuardPass",
    "PASS45_LANDMARK_RECONCILIATION_BUDGET_READY",
    "PASS45_LANDMARK_RECONCILIATION_COMPLETE",
    "SeparationStartupGuardIntervalSeconds",
    "SeparationStartupGuardPassCount",
    "RemoveInstance(",
    "->Destroy(",
):
    forbid(separation, stale, "retired landmark mutation/reconciliation path")

delay_match = re.search(r"ValidationDelaySeconds\s*=\s*([0-9.]+)f", separation)
if not delay_match:
    raise SystemExit("PASS12 WORLD STABILITY VERIFY FAIL: landmark validation delay is not explicit")
delay_seconds = float(delay_match.group(1))
if delay_seconds <= 0.0 or delay_seconds >= 12.0:
    raise SystemExit("PASS12 WORLD STABILITY VERIFY FAIL: landmark validation must finish before 12 s baseline capture")

for needle in (
    "UOCWorldGeometryStabilitySubsystem",
    "UTickableWorldSubsystem",
    "ReadTrackedCounts",
    "CompareWithBaseline",
):
    require(header, needle, "Pass12 validator header")

for needle in (
    "BaselineCaptureSeconds = 12.0f",
    "ComparisonIntervalSeconds = 4.0f",
    "RequiredStableComparisons = 2",
    'TEXT("Buildings")',
    'TEXT("ResidentialRoofs")',
    'TEXT("ResidentialDetails")',
    'TEXT("LandmarkBlocks")',
    'TEXT("LandmarkRoofs")',
    'TEXT("LandmarkWindows")',
    'TEXT("LandmarkDetails")',
    'TEXT("ParkGeometry")',
    'TEXT("ParkPaths")',
    'TEXT("Fences")',
    'TEXT("Ground")',
    'TEXT("Roads")',
    'TEXT("Sidewalks")',
    "ValidateSemanticMaterials",
    "HasAuthoredGroundSurface",
    "HasAuthoredSurface",
    "SM_Plane_1x1",
    "M_Inst_Landscape",
    "SM_Urb_Roa_Asphalt_01",
    "SM_Urb_Roa_Sidewalk_01",
    "SM_Stonepath_Var01",
    "SM_Fence_Var01",
    "authored_ground_mesh_missing",
    "authored_ground_mesh_invalid_",
    "authored_ground_material_missing",
    "authored_ground_material_invalid_",
    "authored_surface_mesh_missing_",
    "authored_surface_mesh_invalid_",
    "authored_surface_material_missing_",
    "authored_surface_basicshape_material_",
    "PASS45_WORLD_MATERIAL_BASELINE_READY",
    "ground_authored=1",
    "authored_surface_families=5",
    "basicshape_surface_materials=0",
    "PASS45_WORLD_MATERIAL_STABLE",
    "late_geometry_mutation_",
    "PASS12_WORLD_GEOMETRY_BASELINE_CAPTURED",
    "PASS12_WORLD_GEOMETRY_STABLE_SAMPLE",
    "PASS12_WORLD_GEOMETRY_STABLE",
    "PASS12_WORLD_GEOMETRY_STABILITY_FAIL",
):
    require(validator, needle, "Pass12/Pass45 runtime validator")
for stale in (
    "HasColorMID",
    "UMaterialInstanceDynamic",
    "semantic_mid_missing_",
    "semantic_color_parameter_missing_",
    "ground_legacy_mid=1",
):
    forbid(validator, stale, "retired BasicShape MID stability acceptance")

for needle in (
    "RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd",
    "PASS12_WORLD_GEOMETRY_STABILITY_FAIL",
    "PASS12_WORLD_GEOMETRY_STABLE",
    "at least 21 seconds",
    "R14_CURRENT_GAMEPLAY.log",
):
    require(launcher, needle, "Pass12 Windows launcher")

print("WORLD GEOMETRY STABILITY PASS12/PASS45 ITEM31 SOURCE CONTRACT PASS")
print("- historical landmark delayed timers remain cancelled and identity validation stays mutation-free")
print("- Pass45 daylight remains component-owned: 120000 lux + AutoExposure=True + extended EV100 range")
print("- playable Ground upgrades before baseline to tracked SM_Plane_1x1 + M_Inst_Landscape with XY/top-Z preserved")
print("- Roads/Sidewalks/ParkPaths/Fences upgrade before baseline to tracked authored meshes with packaged materials")
print("- Pass12 validates Ground plus four authored ISM surface families at 12s, 16s and 20s")
print("- no unrelated late source owner may mutate Ground/Roads/Sidewalks/ParkPaths/Fences materials")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 runtime evidence still required")
