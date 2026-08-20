from pathlib import Path
import math
import re

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
COLLEGE_H = SRC / "Public" / "OCR13CollegeFacadeSubsystem.h"
COLLEGE_CPP = SRC / "Private" / "OCR13CollegeFacadeSubsystem.cpp"
ACCESS_H = SRC / "Public" / "OCR13CollegeAccessRepairSubsystem.h"
ACCESS_CPP = SRC / "Private" / "OCR13CollegeAccessRepairSubsystem.cpp"
STADIUM_H = SRC / "Public" / "OCR13StadiumSurfaceSubsystem.h"
STADIUM_CPP = SRC / "Private" / "OCR13StadiumSurfaceSubsystem.cpp"
WORLD_CPP = SRC / "Private" / "OCWorldSectorOster.cpp"
ROAD_CPP = SRC / "Private" / "OCLocationSectorS01RoadData.cpp"
CIVIC_CPP = SRC / "Private" / "OCR13CivicLandscapingSubsystem.cpp"


def fail(message: str) -> None:
    raise SystemExit("R13.5 COLLEGE/STADIUM VISUAL VERIFY FAIL: " + message)


def read(path: Path) -> str:
    if not path.is_file():
        fail(f"missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(text: str, needle: str, where: str) -> None:
    if needle not in text:
        fail(f"{where}: missing {needle!r}")


def const_float(text: str, name: str) -> float:
    match = re.search(rf"constexpr\s+float\s+{re.escape(name)}\s*=\s*(-?[0-9.]+)f\s*;", text)
    if not match:
        fail(f"cannot parse constexpr float {name}")
    return float(match.group(1))


college_h = read(COLLEGE_H)
college = read(COLLEGE_CPP)
access_h = read(ACCESS_H)
access = read(ACCESS_CPP)
stadium_h = read(STADIUM_H)
stadium = read(STADIUM_CPP)
world = read(WORLD_CPP)
road = read(ROAD_CPP)
civic = read(CIVIC_CPP)

for name, text in (("college", college_h), ("access", access_h), ("stadium", stadium_h)):
    includes = [line.strip() for line in text.splitlines() if line.strip().startswith("#include")]
    if not includes or "generated.h" not in includes[-1]:
        fail(f"generated.h must remain final include in {name} header")

for token in [
    "void AOCWorldSectorOster::BuildCollegeSector()",
    "const float Yaw = 1.0f;",
    "AddBox(LandmarkBlocks, MainCenter, FVector(6500, 1900, 1440), Yaw);",
    "AddBox(LandmarkDetails, College + FVector(900, -1230, 230), FVector(2450, 600, 460), Yaw);",
    "AddBox(LandmarkDetails, College + FVector(900, -1590, 505), FVector(2650, 920, 70), Yaw);",
    "College + FVector(900, -1940 - Step * 115.0f, 22 + Step * 22.0f)",
    "FVector(2750 - Step * 100.0f, 220, 40)",
    "constexpr int32 Columns = 9;",
    "constexpr int32 Rows = 4;",
    "if (Row == 0 && (Col == 5 || Col == 6)) continue;",
    "AddFacadeWindow(LandmarkWindows, College, FVector(X, -965, Z), FVector(430, 24, 220), Yaw, true);",
    "Transform.SetLocation(Center);",
    "const FVector WorldOffset = Rotate2D(LocalOffset, BuildingYawDegrees);",
    "BuildingCenter + WorldOffset",
    "AddBox(Fences, College + FVector(0, -2450, 110), FVector(10400, 45, 220), Yaw);",
    "AddBox(Fences, College + FVector(-5600, 3400, 110), FVector(45, 11700, 220), Yaw);",
    "AddBox(Fences, College + FVector(0, 9300, 110), FVector(11200, 45, 220), Yaw);",
    "const float HalfWidth = Size.Y * 0.5f;",
    "HalfWidth + 260.0f",
    "FVector(Size.X, 260.0f, 18.0f)",
]:
    require(world, token, "BuildCollegeSector/source topology")

for token in [
    "class OSTERCONFLICT_API UOCR13CollegeFacadeSubsystem",
    "R13_CollegeFacadeAligned",
    "constexpr float MainWidthCm = 6500.0f;",
    "constexpr float MainDepthCm = 1900.0f;",
    "constexpr float MainHeightCm = 1440.0f;",
    "constexpr float EntranceCenterX = 900.0f;",
    "FVector RotateCollegeVector(const FVector& LocalOffset)",
    "FVector CollegeRotatedLocalToWorld(const FVector& College, const FVector& LocalOffset)",
    "FVector CollegeAuthoredCenterToWorld(const FVector& College, const FVector& SourceCenterOffset)",
    "FVector CollegeFaceOffsetFromAuthoredCenter(const FVector& College, const FVector& SourceCenterOffset",
    "return College + SourceCenterOffset;",
    "Mixed source-coordinate contract",
    "R13_CollegeWindowGlass",
    "R13_CollegeWindowFrames",
    "constexpr int32 WindowColumns = 9;",
    "constexpr int32 WindowRows = 4;",
    "if (Row == 0 && (Col == 5 || Col == 6)) continue;",
    "const FVector EntranceSourcePlanCenter(EntranceCenterX, EntranceBlockCenterY, 0.0f);",
    "CollegeFaceOffsetFromAuthoredCenter(College, EntranceSourcePlanCenter",
    "R13_CollegeStairTreads",
    "R13_CollegeStairNosings",
    "const float StepCenterY = -1940.0f - Step * 115.0f;",
    "const float StepCenterZ = 22.0f + Step * 22.0f;",
    "const float StepWidth = 2750.0f - Step * 100.0f;",
    "const FVector StepSourceCenter(EntranceCenterX, StepCenterY, StepCenterZ);",
    "CollegeFaceOffsetFromAuthoredCenter(College, StepSourceCenter",
    "const FVector CanopySourceCenter(EntranceCenterX, EntranceCanopyCenterY, 505.0f);",
    "const FVector CanopyCenter = CollegeAuthoredCenterToWorld(College, CanopySourceCenter);",
    "source-coordinate contract aligned",
    "rotated main/window offsets plus direct authored entrance/canopy/stair centers",
    "SetCollisionEnabled(ECollisionEnabled::NoCollision)",
    "SetCanEverAffectNavigation(false)",
    "source collision/footprint preserved",
]:
    require(college, token, "college facade")

for forbidden in [
    "Main authored block is 48 x 17 x 15.5 m",
    "FVector CollegeLocalToWorld(const FVector& College, const FVector& LocalOffset)",
    "const FVector EntranceFront = CollegeRotatedLocalToWorld",
    "const FVector CanopyCenter = CollegeRotatedLocalToWorld",
    "AddBox(StairTreads, CollegeRotatedLocalToWorld",
    "MainFrontY - 16.0f",
    "MainFrontY - 18.0f",
]:
    if forbidden in college:
        fail(f"obsolete/unified-coordinate/floating college facade topology returned: {forbidden}")

# Legacy front fence physically intersects the lowest stair.
legacy_fence_min_y = -2450.0 - 45.0 / 2.0
legacy_fence_max_y = -2450.0 + 45.0 / 2.0
lowest_step_y = -1940.0 - 4.0 * 115.0
lowest_step_min_y = lowest_step_y - 220.0 / 2.0
lowest_step_max_y = lowest_step_y + 220.0 / 2.0
if max(legacy_fence_min_y, lowest_step_min_y) > min(legacy_fence_max_y, lowest_step_max_y):
    fail("legacy front fence no longer intersects the lowest stair; remove/update the runtime access repair")

for token in [
    "class OSTERCONFLICT_API UOCR13CollegeAccessRepairSubsystem",
    "void ScheduleRepair(UWorld& World, int32 AttemptIndex);",
    "bool RepairCollegeEntrance(UWorld& World);",
]:
    require(access_h, token, "college access header")

for token in [
    "constexpr float CollegeAccessInitialDelaySeconds = 0.10f;",
    "constexpr float CollegeAccessRetryDelaySeconds = 0.25f;",
    "constexpr int32 CollegeAccessMaxAttempts = 40;",
    "constexpr float FrontFenceGapCenterX = 900.0f;",
    "constexpr float FrontFenceGapWidthCm = 3400.0f;",
    "constexpr float FrontLeftSegmentCenterX = -3000.0f;",
    "constexpr float FrontLeftSegmentLengthCm = 4400.0f;",
    "constexpr float FrontRightSegmentCenterX = 3900.0f;",
    "constexpr float FrontRightSegmentLengthCm = 2600.0f;",
    "const FVector LegacyFrontFenceScale(104.0f, 0.45f, 2.20f);",
    "constexpr float LeftFenceX = -5600.0f;",
    "constexpr float LeftFenceCenterY = 3400.0f;",
    "constexpr float LeftFenceGapCenterY = 0.0f;",
    "constexpr float LeftFenceGapWidthCm = 1800.0f;",
    "constexpr float LeftLowerSegmentCenterY = -1675.0f;",
    "constexpr float LeftLowerSegmentLengthCm = 1550.0f;",
    "constexpr float LeftUpperSegmentCenterY = 5075.0f;",
    "constexpr float LeftUpperSegmentLengthCm = 8350.0f;",
    "const FVector LegacyLeftFenceScale(0.45f, 117.0f, 2.20f);",
    "FindLegacyFenceInstance(UInstancedStaticMeshComponent* Fences, const FVector& ExpectedCenter",
    'TEXT("R13_CollegeAccessFenceSplits")',
    "Split->SetCollisionProfileName(SourceFences->GetCollisionProfileName());",
    "Split->SetCollisionEnabled(SourceFences->GetCollisionEnabled());",
    "Split->SetCanEverAffectNavigation(true);",
    "ScheduleRepair(InWorld, 0);",
    "if (AttemptIndex >= CollegeAccessMaxAttempts)",
    "if (!Sector->HasActorBegunPlay()) return false;",
    "const int32 FrontIndex = FindLegacyFenceInstance(SourceFences, LegacyFrontCenter, LegacyFrontFenceScale);",
    "const int32 LeftIndex = FindLegacyFenceInstance(SourceFences, LegacyLeftCenter, LegacyLeftFenceScale);",
    "FrontIndex == INDEX_NONE || LeftIndex == INDEX_NONE || FrontIndex == LeftIndex",
    "const bool bFrontRemovedFirst = FrontIndex > LeftIndex;",
    "SourceFences->RemoveInstance(FirstIndex)",
    "SourceFences->RemoveInstance(SecondIndex)",
    "SourceFences->AddInstance(FirstTransform, false);",
    "first removal restored and split replacement rolled back",
    'TEXT("R13_CollegeAccessRepairApplied")',
    "34m stair opening at X+900cm",
    "18m vehicle+sidewalk gate at Y=0 with 1.8m side clearance",
    "blocking/navigation preserved",
    "north boundary untouched",
]:
    require(access, token, "college access repair")

for forbidden in [
    "constexpr float CollegeAccessRepairDelaySeconds = 2.40f;",
    'TEXT("NoCollision")',
    "SetCanEverAffectNavigation(false)",
    'TEXT("R13_CollegeFrontFenceSplit")',
]:
    if forbidden in access:
        fail(f"obsolete/nonblocking college access repair marker returned: {forbidden}")

# Front fence: original span [-5200,+5200], exact 34 m opening centered on X=+900.
front_gap_center = const_float(access, "FrontFenceGapCenterX")
front_gap_width = const_float(access, "FrontFenceGapWidthCm")
front_left_center = const_float(access, "FrontLeftSegmentCenterX")
front_left_length = const_float(access, "FrontLeftSegmentLengthCm")
front_right_center = const_float(access, "FrontRightSegmentCenterX")
front_right_length = const_float(access, "FrontRightSegmentLengthCm")
front_left_outer = front_left_center - front_left_length / 2.0
front_left_inner = front_left_center + front_left_length / 2.0
front_right_inner = front_right_center - front_right_length / 2.0
front_right_outer = front_right_center + front_right_length / 2.0
if abs(front_left_outer + 5200.0) > 0.01 or abs(front_right_outer - 5200.0) > 0.01:
    fail("front split fence no longer preserves original 104m outer span")
if abs((front_right_inner - front_left_inner) - front_gap_width) > 0.01:
    fail("front split fence segment geometry disagrees with gap width")
if abs(((front_left_inner + front_right_inner) / 2.0) - front_gap_center) > 0.01:
    fail("front split fence opening is no longer centered on authored entrance X=+900")
if abs((front_left_length + front_gap_width + front_right_length) - 10400.0) > 0.01:
    fail("front split segments + opening must exactly cover original 104m span")
if front_gap_width <= 2750.0:
    fail("front opening must remain wider than maximum 27.5m stair width")

# West fence: original span Y=-2450..9250, exact 18 m road/sidewalk gate centered at Y=0.
left_gap_center = const_float(access, "LeftFenceGapCenterY")
left_gap_width = const_float(access, "LeftFenceGapWidthCm")
left_lower_center = const_float(access, "LeftLowerSegmentCenterY")
left_lower_length = const_float(access, "LeftLowerSegmentLengthCm")
left_upper_center = const_float(access, "LeftUpperSegmentCenterY")
left_upper_length = const_float(access, "LeftUpperSegmentLengthCm")
left_lower_outer = left_lower_center - left_lower_length / 2.0
left_lower_inner = left_lower_center + left_lower_length / 2.0
left_upper_inner = left_upper_center - left_upper_length / 2.0
left_upper_outer = left_upper_center + left_upper_length / 2.0
if abs(left_lower_outer - (-2450.0)) > 0.01 or abs(left_upper_outer - 9250.0) > 0.01:
    fail("west split fence no longer preserves original 117m outer span")
if abs((left_upper_inner - left_lower_inner) - left_gap_width) > 0.01:
    fail("west split fence segment geometry disagrees with gate width")
if abs(((left_lower_inner + left_upper_inner) / 2.0) - left_gap_center) > 0.01:
    fail("west vehicle gate is no longer centered on approach Y=0")
if abs((left_lower_length + left_gap_width + left_upper_length) - 11700.0) > 0.01:
    fail("west split segments + gate must exactly cover original 117m span")
road_width_cm = 660.0
sidewalk_width_cm = 260.0
sidewalk_center_from_road_cm = road_width_cm / 2.0 + 260.0
road_with_walks_half_envelope_cm = sidewalk_center_from_road_cm + sidewalk_width_cm / 2.0
road_with_walks_envelope_cm = road_with_walks_half_envelope_cm * 2.0
if abs(road_with_walks_envelope_cm - 1440.0) > 0.01:
    fail("college road+two-sidewalk envelope assumption changed")
if left_gap_width < road_with_walks_envelope_cm + 360.0:
    fail("west gate must retain >=180cm clearance on each side of road+sidewalk envelope")

for token in [
    'TEXT("S01_ROAD_COLLEGE_APPROACH")',
    "FVector(-16050, 0, 8), FVector(24900, 660, 14), 0.0f, true",
    "retains its original X=-28500 outer start but now terminates at X=-3600",
    "before the rotated 65x19m main building envelope",
]:
    require(road, token, "college vehicle approach")
if "FVector(-13500, 0, 8), FVector(30000, 660, 14)" in road:
    fail("legacy college approach still extends through the main building")

approach_match = re.search(
    r'TEXT\("S01_ROAD_COLLEGE_APPROACH"\).*?FVector\((-?[0-9.]+),\s*0,\s*8\),\s*'
    r'FVector\(([0-9.]+),\s*660,\s*14\),\s*0\.0f', road, flags=re.S)
if not approach_match:
    fail("cannot parse college vehicle approach center/length")
approach_center_x = float(approach_match.group(1))
approach_length = float(approach_match.group(2))
approach_start_x = approach_center_x - approach_length / 2.0
approach_end_x = approach_center_x + approach_length / 2.0
if abs(approach_start_x + 28500.0) > 0.01:
    fail(f"college approach remote start moved unexpectedly: {approach_start_x}")
yaw_rad = math.radians(1.0)
rotated_main_half_extent_x = (6500.0 / 2.0) * math.cos(yaw_rad) + (1900.0 / 2.0) * math.sin(yaw_rad)
rotated_main_west_x = -rotated_main_half_extent_x
approach_clearance_cm = rotated_main_west_x - approach_end_x
if approach_clearance_cm < 300.0:
    fail(f"college vehicle approach must keep >=300cm from rotated main envelope; got {approach_clearance_cm:.2f}cm")

for token in [
    'TEXT("S01_PATH_COLLEGE_CAMPUS")',
    "FVector(900, 5200, 12)",
    "FVector(8000, 280, 18)",
    "Pedestrian-only college campus path; narrowed from the legacy plaza-sized sidewalk proxy",
]:
    require(road, token, "college pedestrian path")
if "FVector(8000, 5900, 18)" in road:
    fail("legacy 80x59m college sidewalk slab returned")

for token in [
    "The S01 college path is centered near Y=5200 and only 2.8 m wide",
    "FVector(-1800, 5700, 0)",
    "FVector(2100, 4700, 0)",
    "pedestrian college path/navigation remain clear",
]:
    require(civic, token, "college campus planting")
for forbidden in ["FVector(-1800, 5200, 0)", "FVector(2100, 5250, 0)"]:
    if forbidden in civic:
        fail(f"tree returned to college pedestrian path centerline: {forbidden}")

for token in [
    "AOCWorldSectorOster::StadiumAnchor()",
    "R13_StadiumSurfaceRoot",
    "R13_StadiumTurf",
    "R13_StadiumTrack",
    "R13_StadiumFieldLines",
    "R13_StadiumGoalAccents",
    "R13_StadiumSpectatorBenches",
    "Old_Planks_Plank_1.Old_Planks_Plank_1",
    "FVector(10440.0f, 6740.0f, 8.0f)",
    "R13.5 stadium surface: 105x68 m turf",
    "source collision preserved",
]:
    require(stadium, token, "stadium visuals")

for label, text in (("college", college), ("access", access), ("stadium", stadium), ("civic", civic)):
    for forbidden in ["FMath::Rand", "FRand"]:
        if forbidden in text:
            fail(f"{label} contains nondeterministic marker: {forbidden}")
    for left, right in (("(", ")"), ("{", "}"), ("[", "]")):
        if text.count(left) != text.count(right):
            fail(f"delimiter mismatch {left}{right} in {label}")

print("R13.5 COLLEGE/STADIUM VISUAL VERIFY: PASS")
print(
    "Checks mixed college coordinates, facade/windows/stairs, lifecycle-safe transactional fence repairs (34m stair opening + 18m road gate), "
    "vehicle-approach/building clearance, 2.8m campus path/tree clearance, plus stadium visuals."
)
