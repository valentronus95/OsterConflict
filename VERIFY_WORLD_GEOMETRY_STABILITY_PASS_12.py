from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict/Source/OsterConflict"

COORDINATOR = SRC / "Private/OCLandmarkStartupCoordinatorSubsystem.cpp"
SEPARATION = SRC / "Private/OCR146LandmarkSeparationSubsystem.cpp"
WORLD_MODELS = SRC / "Private/OCWorldAssetModelsSubsystem.cpp"
VALIDATOR_H = SRC / "Public/OCWorldGeometryStabilitySubsystem.h"
VALIDATOR_CPP = SRC / "Private/OCWorldGeometryStabilitySubsystem.cpp"
LAUNCHER = ROOT / "RUN_R14_WORLD_STABILITY_RUNTIME_ACCEPTANCE.cmd"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS12 WORLD STABILITY VERIFY FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8")


def require(text: str, needle: str, where: str) -> None:
    if needle not in text:
        raise SystemExit(f"PASS12 WORLD STABILITY VERIFY FAIL: {where}: missing {needle!r}")


coordinator = read(COORDINATOR)
separation = read(SEPARATION)
world_models = read(WORLD_MODELS)
header = read(VALIDATOR_H)
validator = read(VALIDATOR_CPP)
launcher = read(LAUNCHER)

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

# The known cleanup window is 40 * 0.20 s = 8 s. Pass 12 deliberately snapshots later at 12 s.
for needle in (
    "SeparationStartupGuardIntervalSeconds = 0.20f",
    "SeparationStartupGuardPassCount = 40",
    "RunStartupGuardPass",
    "bFinalValidation",
):
    require(separation, needle, "landmark separation startup window")

# Imported world model decoration must remain one-shot once its decorator actor exists.
for needle in (
    "if (!World || DecoratorActor.IsValid()) return;",
    "Decorator->PopulateForSector(OsterSector);",
    "HideLegacyVisualProxies(*OsterSector);",
    "DecoratorActor = Decorator;",
):
    require(world_models, needle, "one-shot world asset decorator")

for needle in (
    "UOCWorldGeometryStabilitySubsystem",
    "UTickableWorldSubsystem",
    "ReadTrackedCounts",
    "CompareWithBaseline",
):
    require(header, needle, "Pass 12 validator header")

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
    'TEXT("Roads")',
    'TEXT("Sidewalks")',
    "late_geometry_mutation_",
    "PASS12_WORLD_GEOMETRY_BASELINE_CAPTURED",
    "PASS12_WORLD_GEOMETRY_STABLE_SAMPLE",
    "PASS12_WORLD_GEOMETRY_STABLE",
    "PASS12_WORLD_GEOMETRY_STABILITY_FAIL",
):
    require(validator, needle, "Pass 12 runtime validator")

for needle in (
    "RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd",
    "PASS12_WORLD_GEOMETRY_STABILITY_FAIL",
    "PASS12_WORLD_GEOMETRY_STABLE",
    "at least 21 seconds",
    "R14_CURRENT_GAMEPLAY.log",
):
    require(launcher, needle, "Pass 12 Windows launcher")

print("WORLD GEOMETRY STABILITY PASS 12 SOURCE CONTRACT PASS")
print("- historical landmark delayed timers are cancelled by the authoritative startup coordinator")
print("- known landmark cleanup window remains bounded to 8 seconds")
print("- imported world decorator is one-shot after ownership is acquired")
print("- Pass 12 snapshots 10 source geometry families at 12s and compares again at 16s/20s")
print("- any late instance-count mutation emits a family-specific runtime FAIL marker")
print("- a green runtime marker narrows remaining VIS-FLICKER-001 to rendering/z-fighting/LOD instead of late rebuild")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 runtime evidence still required")
