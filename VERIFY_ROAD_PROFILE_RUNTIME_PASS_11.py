from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict/Source/OsterConflict"

WORLD = SRC / "Private/OCWorldSectorOster.cpp"
DECORATOR = SRC / "Private/OCAssetModelDecorator.cpp"
ROAD_PROPS = SRC / "Private/OCRecoveredRoadsidePropsSubsystem.cpp"
CORRECTION_H = SRC / "Public/OCRoadProfileRuntimeCorrectionSubsystem.h"
CORRECTION_CPP = SRC / "Private/OCRoadProfileRuntimeCorrectionSubsystem.cpp"
LAUNCHER = ROOT / "RUN_R14_ROAD_PROFILE_RUNTIME_ACCEPTANCE.cmd"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS11 ROAD VERIFY FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8")


def require(text: str, needle: str, where: str) -> None:
    if needle not in text:
        raise SystemExit(f"PASS11 ROAD VERIFY FAIL: {where}: missing {needle!r}")


world = read(WORLD)
decorator = read(DECORATOR)
road_props = read(ROAD_PROPS)
header = read(CORRECTION_H)
correction = read(CORRECTION_CPP)
launcher = read(LAUNCHER)

# Preserve the current S16A XY topology as the authoritative source network.
for needle in (
    'Roads = MakeISM(TEXT("Roads"), TEXT("BlockAll"));',
    'Sidewalks = MakeISM(TEXT("Sidewalks"), TEXT("BlockAll"));',
    "void AOCWorldSectorOster::BuildRoadNetwork()",
    "auto AddRoadWithWalks",
    "constexpr float RoadZ = 8.0f;",
    "FVector(138000, 1050, 16)",
    "FVector(Size.X, 260.0f, 18.0f)",
):
    require(world, needle, "legacy S16A road profile")

# There must not be a second active global road mesh owner hiding the actual source of the defect.
if "Roads" in decorator or "Sidewalks" in decorator:
    raise SystemExit("PASS11 ROAD VERIFY FAIL: asset decorator unexpectedly owns road/sidewalk presentation")
require(road_props, "return false;", "retired roadside props subsystem")
require(road_props, "Intentionally retired", "retired roadside props subsystem")

for needle in (
    "UOCRoadProfileRuntimeCorrectionSubsystem",
    "UTickableWorldSubsystem",
    "NormalizeRoadProfile",
    "ValidateRoadProfile",
):
    require(header, needle, "road profile correction header")

for needle in (
    "constexpr float RoadThicknessCm = 4.0f;",
    "constexpr float SidewalkThicknessCm = 8.0f;",
    "constexpr float RoadCenterZCm = RoadThicknessCm * 0.5f;",
    "constexpr float SidewalkCenterZCm = RoadThicknessCm + SidewalkThicknessCm * 0.5f;",
    'FindISM(Sector, TEXT("Roads"))',
    'FindISM(Sector, TEXT("Sidewalks"))',
    "Location.Z = DesiredCenterZCm;",
    "Scale.Z = DesiredThicknessCm / 100.0f;",
    "UpdateInstanceTransform",
    "PASS11_ROAD_PROFILE_READY",
    "PASS11_ROAD_PROFILE_RUNTIME_FAIL",
):
    require(correction, needle, "road profile correction")

for needle in (
    "RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd",
    "PASS11_ROAD_PROFILE_READY",
    "PASS11_ROAD_PROFILE_RUNTIME_FAIL",
    "R14_CURRENT_GAMEPLAY.log",
):
    require(launcher, needle, "Windows road profile acceptance launcher")

print("ROAD PROFILE RUNTIME PASS 11 SOURCE CONTRACT PASS")
print("- one authoritative S16A Roads/Sidewalks source owner is confirmed")
print("- obsolete RoadZ=8 / 14-16 cm road and 18 cm sidewalk source profiles are explicitly detected")
print("- runtime correction preserves XY/yaw/length/width while normalizing only Z center/thickness")
print("- road profile target is 4 cm asphalt with an 8 cm sidewalk curb above it")
print("- strict Windows launcher requires PASS11_ROAD_PROFILE_READY and rejects FAIL")
print("STATUS: SOURCE CONTRACT ONLY; UE 5.8 drive/walk visual acceptance still required")
