from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict/Source/OsterConflict"

COORDINATOR = SRC / "Private/OCLandmarkStartupCoordinatorSubsystem.cpp"
SEPARATION = SRC / "Private/OCR146LandmarkSeparationSubsystem.cpp"
VALIDATOR_H = SRC / "Public/OCWorldGeometryStabilitySubsystem.h"
VALIDATOR_CPP = SRC / "Private/OCWorldGeometryStabilitySubsystem.cpp"
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


coordinator = read(COORDINATOR)
separation = read(SEPARATION)
header = read(VALIDATOR_H)
validator = read(VALIDATOR_CPP)
launcher = read(LAUNCHER)

# Pass45 runtime evidence rejected the old generic AdvancedVillagePack/recovered presentation owners.
# World-stability CI must protect their physical retirement rather than requiring their resurrection.
for path in RETIRED_GENERIC_OWNERS:
    if path.exists():
        raise SystemExit(
            f"PASS12 WORLD STABILITY VERIFY FAIL: rejected generic/recovered owner resurrected: {path.relative_to(ROOT)}"
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
    'TEXT("Roads")',
    'TEXT("Sidewalks")',
    "late_geometry_mutation_",
    "PASS12_WORLD_GEOMETRY_BASELINE_CAPTURED",
    "PASS12_WORLD_GEOMETRY_STABLE_SAMPLE",
    "PASS12_WORLD_GEOMETRY_STABLE",
    "PASS12_WORLD_GEOMETRY_STABILITY_FAIL",
):
    require(validator, needle, "Pass12 runtime validator")

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
print("- validation completes before the 12 s Pass12 baseline")
print("- Pass12 snapshots source geometry families at 12s and compares again at 16s/20s")
print("- any late instance-count mutation emits a family-specific runtime FAIL marker")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 runtime evidence still required")
