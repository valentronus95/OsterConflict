from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict/Source/OsterConflict"

COORDINATOR = SRC / "Private/OCLandmarkStartupCoordinatorSubsystem.cpp"
SEPARATION = SRC / "Private/OCR146LandmarkSeparationSubsystem.cpp"
VALIDATOR_H = SRC / "Public/OCWorldGeometryStabilitySubsystem.h"
VALIDATOR_CPP = SRC / "Private/OCWorldGeometryStabilitySubsystem.cpp"
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


def has_semantic_material_write(source: str) -> bool:
    """Detect a SetMaterial write tied locally to a tracked world-sector family.

    A whole-file token co-occurrence is intentionally insufficient: Pass6 legitimately references
    Roads/Sidewalks while removing obsolete BASE instances and separately calls SetMaterial on weapons.
    The ownership gate therefore requires the semantic family token to be in the same local material-write
    context rather than elsewhere in an unrelated function.
    """
    semantic_tokens = ('TEXT("Ground")', 'TEXT("Roads")', 'TEXT("Sidewalks")')
    for match in re.finditer(r"\bSetMaterial\s*\(", source):
        start = max(0, match.start() - 1800)
        end = min(len(source), match.end() + 500)
        window = source[start:end]
        if "AOCWorldSectorOster" in window and any(token in window for token in semantic_tokens):
            return True
    return False


coordinator = read(COORDINATOR)
separation = read(SEPARATION)
header = read(VALIDATOR_H)
validator = read(VALIDATOR_CPP)
visual_environment = read(VISUAL_ENVIRONMENT)
world_sector = read(WORLD_SECTOR)
engine_config = read(ENGINE_CONFIG)
launcher = read(LAUNCHER)

# Pass45 runtime evidence rejected the old generic AdvancedVillagePack/recovered presentation owners.
# World-stability CI must protect their physical retirement rather than requiring their resurrection.
for path in RETIRED_GENERIC_OWNERS:
    if path.exists():
        raise SystemExit(
            f"PASS12 WORLD STABILITY VERIFY FAIL: rejected generic/recovered owner resurrected: {path.relative_to(ROOT)}"
        )

# Pass45 P0 black-world correction is one coherent exposure/lighting contract. UE 5.8 Directional Light
# intensity is lux, so physical daylight must not be paired with the previous disabled exposure adaptation.
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
forbid(engine_config, "r.DefaultFeature.AutoExposure=False", "Pass45 renderer exposure contract")

# The accepted source-only semantic world baseline remains owned by AOCWorldSectorOster. Do not allow
# a new late owner to target Ground/Roads/Sidewalks with SetMaterial after that baseline.
for needle in (
    "/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial",
    "Tint(Ground",
    "Tint(Roads",
    "Tint(Sidewalks",
):
    require(world_sector, needle, "accepted semantic world-material owner")

for cpp_path in (SRC / "Private").glob("*.cpp"):
    if cpp_path == WORLD_SECTOR or cpp_path.name == "OCR13StadiumSurfaceSubsystem.cpp":
        continue
    source = read(cpp_path)
    if has_semantic_material_write(source):
        raise SystemExit(
            "PASS12 WORLD STABILITY VERIFY FAIL: second semantic world-material owner detected: "
            f"{cpp_path.relative_to(ROOT)}"
        )

# The authoritative location startup must cancel historical delayed timers before immediate builds.
for needle in (
    "SetTimerForNextTick",
    "RunAuthoritativeStartup",
    "Timers.ClearAllTimersForObject(Stage);",
    "RunAuthoritativeBuildNow(World)",
    "RunAuthoritativeDetailNow(World)",
    "historical delayed reveal timers were cancelled",
):
    require(coordinator, needle, "landmark startup coordinator")

# Pass45 retires the old mutating reconciliation loop. Separation is now one bounded validation-only pass
# after current landmark startup. Pass12 only requires that it completes before the 12s geometry baseline.
for needle in (
    "constexpr float ValidationDelaySeconds",
    "ValidateSeparation",
    "PASS45_LANDMARK_SEPARATION_VALIDATION_SCHEDULED",
    "PASS45_LANDMARK_SEPARATION_VALIDATION_READY",
    "PASS45_LANDMARK_SEPARATION_VALIDATION_FAIL",
    "mutation=0",
    "periodic_scan=0",
    "primary_authoring_fix_required=1",
):
    require(separation, needle, "Pass45 validation-only landmark separation")
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
    raise SystemExit("PASS12 WORLD STABILITY VERIFY FAIL: landmark validation must be bounded and finish before 12 s baseline capture")

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
    'TEXT("Ground")',
    'TEXT("Roads")',
    'TEXT("Sidewalks")',
    "ValidateSemanticMaterials",
    "UMaterialInstanceDynamic",
    "GetAllVectorParameterInfo",
    'Parameter.Name == TEXT("Color")',
    "semantic_material_missing_",
    "semantic_mid_missing_",
    "semantic_color_parameter_missing_",
    "PASS45_WORLD_MATERIAL_BASELINE_READY",
    "PASS45_WORLD_MATERIAL_STABLE",
    "late_geometry_mutation_",
    "PASS12_WORLD_GEOMETRY_BASELINE_CAPTURED",
    "PASS12_WORLD_GEOMETRY_STABLE_SAMPLE",
    "PASS12_WORLD_GEOMETRY_STABLE",
    "PASS12_WORLD_GEOMETRY_STABILITY_FAIL",
):
    require(validator, needle, "Pass12/Pass45 runtime validator")

for needle in (
    "RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd",
    "PASS12_WORLD_GEOMETRY_STABILITY_FAIL",
    "PASS12_WORLD_GEOMETRY_STABLE",
    "at least 21 seconds",
    "R14_CURRENT_GAMEPLAY.log",
):
    require(launcher, needle, "Pass12 Windows launcher")

print("WORLD GEOMETRY STABILITY PASS12/PASS45 SOURCE CONTRACT PASS")
print("- historical landmark delayed timers are cancelled by the authoritative startup coordinator")
print("- landmark separation is one bounded validation-only pass with mutation=0")
print("- rejected generic world/decorator/recovered owners remain physically retired")
print("- Pass45 daylight is component-owned, replicated and paired: 120000 lux + AutoExposure=True")
print("- AOCWorldSectorOster remains the accepted Ground/Roads/Sidewalks semantic-material owner")
print("- no second source owner may target those semantic families with SetMaterial")
print("- validation completes before the 12 s Pass12 baseline")
print("- Pass12 snapshots source geometry and validates semantic MID/Color at 12s, 16s and 20s")
print("- missing/broken semantic materials emit a family-specific runtime FAIL marker")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 runtime evidence still required")
