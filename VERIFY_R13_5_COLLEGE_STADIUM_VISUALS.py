from pathlib import Path
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


def const_float(text: str, name: str) -> float:
    match = re.search(rf"constexpr\s+float\s+{re.escape(name)}\s*=\s*(-?[0-9.]+)f\s*;", text)
    if not match:
        fail(f"cannot parse constexpr float {name}")
    return float(match.group(1))


for path in (
    COLLEGE_H, COLLEGE_CPP, ACCESS_H, ACCESS_CPP,
    STADIUM_H, STADIUM_CPP, WORLD_CPP, ROAD_CPP, CIVIC_CPP,
):
    if not path.is_file():
        fail(f"missing {path.relative_to(ROOT)}")

college_h = COLLEGE_H.read_text(encoding="utf-8", errors="replace")
college = COLLEGE_CPP.read_text(encoding="utf-8", errors="replace")
access_h = ACCESS_H.read_text(encoding="utf-8", errors="replace")
access = ACCESS_CPP.read_text(encoding="utf-8", errors="replace")
stadium_h = STADIUM_H.read_text(encoding="utf-8", errors="replace")
stadium = STADIUM_CPP.read_text(encoding="utf-8", errors="replace")
world = WORLD_CPP.read_text(encoding="utf-8", errors="replace")
road = ROAD_CPP.read_text(encoding="utf-8", errors="replace")
civic = CIVIC_CPP.read_text(encoding="utf-8", errors="replace")

for name, text in (("college", college_h), ("access", access_h), ("stadium", stadium_h)):
    includes = [line.strip() for line in text.splitlines() if line.strip().startswith("#include")]
    if not includes or "generated.h" not in includes[-1]:
        fail(f"generated.h must remain final include in {name} header")

# BuildCollegeSector itself uses two placement contracts. AddBox rotates the mesh but keeps the supplied center,
# while AddFacadeWindow explicitly rotates its local offset around CollegeAnchor before calling AddBox.
for token in [
    "void AOCWorldSectorOster::BuildCollegeSector()",
    "const float Yaw = 1.0f;",
    "AddBox(LandmarkBlocks, MainCenter, FVector(6500, 1900, 1440), Yaw);",
    "AddBox(LandmarkRoofs, College + FVector(0, 0, 1460), FVector(6650, 2020, 70), Yaw);",
    "AddBox(LandmarkDetails, College + FVector(900, -1230, 230), FVector(2450, 600, 460), Yaw);",
    "AddBox(LandmarkDetails, College + FVector(900, -1590, 505), FVector(2650, 920, 70), Yaw);",
    "College + FVector(900, -1940 - Step * 115.0f, 22 + Step * 22.0f)",
    "FVector(2750 - Step * 100.0f, 220, 40)",
    "constexpr int32 Columns = 9;",
    "constexpr int32 Rows = 4;",
    "const float X = -2800.0f + Col * 700.0f;",
    "const float Z = 255.0f + Row * 340.0f;",
    "if (Row == 0 && (Col == 5 || Col == 6)) continue;",
    "AddFacadeWindow(LandmarkWindows, College, FVector(X, -965, Z), FVector(430, 24, 220), Yaw, true);",
    "AddBoxRotated(Component, Center, SizeCm, FRotator(0.0f, YawDegrees, 0.0f));",
    "Transform.SetLocation(Center);",
    "const FVector WorldOffset = Rotate2D(LocalOffset, BuildingYawDegrees);",
    "BuildingCenter + WorldOffset",
    "AddBox(Fences, College + FVector(0, -2450, 110), FVector(10400, 45, 220), Yaw);",
]:
    if token not in world:
        fail(f"BuildCollegeSector/source placement marker missing: {token}")

for token in [
    "AOCWorldSectorOster::CollegeAnchor()",
    "R13_CollegeFacadeRoot",
    "R13_CollegeFacadeAligned",
    "R13_CollegeDarkPlinth",
    "R13_CollegeFacadeBands",
    "R13_CollegeEntranceCanopy",
    "R13_CollegeEntranceGlass",
    "R13_CollegeEntranceFrame",
    "R13_CollegeWindowGlass",
    "R13_CollegeWindowFrames",
    "R13_CollegeStairTreads",
    "R13_CollegeStairNosings",
    "R13_CollegeStepMat",
    "Glass_Window.Glass_Window",
    "FloorBandZ[]",
    "constexpr float CollegeYawDegrees = 1.0f;",
    "constexpr float MainWidthCm = 6500.0f;",
    "constexpr float MainDepthCm = 1900.0f;",
    "constexpr float MainHeightCm = 1440.0f;",
    "constexpr float MainFrontY = -950.0f;",
    "constexpr float MainSkinPlinthY = MainFrontY - 9.0f;",
    "constexpr float MainSkinBandY = MainFrontY - 10.0f;",
    "constexpr float EntranceCenterX = 900.0f;",
    "constexpr float EntranceBlockCenterY = -1230.0f;",
    "constexpr float EntranceFrontY = -1530.0f;",
    "constexpr float EntranceCanopyCenterY = -1590.0f;",
    "constexpr float WindowFrontY = -981.0f;",
    "constexpr float EntranceFrontLocalY = EntranceFrontY - EntranceBlockCenterY;",
    "FVector RotateCollegeVector(const FVector& LocalOffset)",
    "FVector CollegeRotatedLocalToWorld(const FVector& College, const FVector& LocalOffset)",
    "FVector CollegeAuthoredCenterToWorld(const FVector& College, const FVector& SourceCenterOffset)",
    "FVector CollegeFaceOffsetFromAuthoredCenter(const FVector& College, const FVector& SourceCenterOffset",
    "return College + SourceCenterOffset;",
    "return CollegeAuthoredCenterToWorld(College, SourceCenterOffset) + RotateCollegeVector(LocalFaceOffset);",
    "Mixed source-coordinate contract",
    "main facade/window offsets follow AddFacadeWindow",
    "entrance/canopy/stair centers are direct College + FVector(...) source centers",
    "MainWidthCm - 100.0f",
    "constexpr int32 WindowColumns = 9;",
    "constexpr int32 WindowRows = 4;",
    "if (Row == 0 && (Col == 5 || Col == 6)) continue;",
    "FVector(408.0f, 8.0f, 198.0f)",
    "FVector(446.0f, 12.0f, 18.0f)",
    "FVector(18.0f, 12.0f, 240.0f)",
    "const FVector EntranceSourcePlanCenter(EntranceCenterX, EntranceBlockCenterY, 0.0f);",
    "CollegeFaceOffsetFromAuthoredCenter(College, EntranceSourcePlanCenter",
    "EntranceFrontLocalY - 7.0f",
    "EntranceFrontLocalY - 9.0f",
    "for (int32 Step = 0; Step < 5; ++Step)",
    "const float StepCenterY = -1940.0f - Step * 115.0f;",
    "const float StepCenterZ = 22.0f + Step * 22.0f;",
    "const float StepWidth = 2750.0f - Step * 100.0f;",
    "const FVector StepSourceCenter(EntranceCenterX, StepCenterY, StepCenterZ);",
    "CollegeAuthoredCenterToWorld(College",
    "CollegeFaceOffsetFromAuthoredCenter(College, StepSourceCenter",
    "FVector(0.0f, -114.0f, 16.0f)",
    "const FVector CanopySourceCenter(EntranceCenterX, EntranceCanopyCenterY, 505.0f);",
    "const FVector CanopyCenter = CollegeAuthoredCenterToWorld(College, CanopySourceCenter);",
    "FVector(0.0f, -469.0f, 0.0f)",
    "FVector(-1334.0f, 0.0f, 0.0f)",
    "FVector(1334.0f, 0.0f, 0.0f)",
    "source-coordinate contract aligned",
    "rotated main/window offsets plus direct authored entrance/canopy/stair centers",
    "SetCollisionEnabled(ECollisionEnabled::NoCollision)",
    "SetCanEverAffectNavigation(false)",
    "source collision/footprint preserved",
]:
    if token not in college:
        fail(f"college facade marker missing: {token}")

source_to_overlay_pairs = [
    ("FVector(6500, 1900, 1440)", "MainWidthCm = 6500.0f", "main mass width"),
    ("FVector(6500, 1900, 1440)", "MainDepthCm = 1900.0f", "main mass depth"),
    ("FVector(6500, 1900, 1440)", "MainHeightCm = 1440.0f", "main mass height"),
    ("FVector(900, -1230, 230)", "EntranceCenterX = 900.0f", "entrance X"),
    ("FVector(900, -1230, 230)", "EntranceBlockCenterY = -1230.0f", "entrance block Y"),
    ("FVector(900, -1590, 505)", "EntranceCanopyCenterY = -1590.0f", "canopy Y"),
    ("FVector(X, -965, Z)", "WindowFrontY = -981.0f", "front-window visual offset"),
    ("-1940 - Step * 115.0f", "StepCenterY = -1940.0f - Step * 115.0f", "stair center progression"),
    ("22 + Step * 22.0f", "StepCenterZ = 22.0f + Step * 22.0f", "stair height progression"),
    ("2750 - Step * 100.0f", "StepWidth = 2750.0f - Step * 100.0f", "stair width progression"),
]
for source_marker, overlay_marker, label in source_to_overlay_pairs:
    if source_marker not in world or overlay_marker not in college:
        fail(f"college source/overlay mismatch guard failed for {label}")

for forbidden in [
    "FVector(4740.0f, 18.0f, 140.0f)",
    "FVector(4720.0f, 20.0f, 26.0f)",
    "College + FVector(0.0f, -1040.0f, 0.0f)",
    "const float FrontY = College.Y - 860.0f;",
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

# The authored continuous front fence intersects the lowest stair at Y=-2450. Keep the source marker explicit so
# this runtime repair becomes deliberately obsolete if BuildCollegeSector is later corrected at the source.
legacy_fence_min_y = -2450.0 - 45.0 / 2.0
legacy_fence_max_y = -2450.0 + 45.0 / 2.0
lowest_step_y = -1940.0 - 4.0 * 115.0
lowest_step_min_y = lowest_step_y - 220.0 / 2.0
lowest_step_max_y = lowest_step_y + 220.0 / 2.0
if max(legacy_fence_min_y, lowest_step_min_y) > min(legacy_fence_max_y, lowest_step_max_y):
    fail("legacy front fence no longer intersects the lowest stair; remove/update the runtime access repair")

for token in [
    "class OSTERCONFLICT_API UOCR13CollegeAccessRepairSubsystem",
    "void RepairCollegeEntrance(UWorld& World);",
]:
    if token not in access_h:
        fail(f"college access header marker missing: {token}")

for token in [
    "constexpr float CollegeAccessRepairDelaySeconds = 2.40f;",
    "constexpr float CollegeYawDegrees = 1.0f;",
    "constexpr float FrontFenceY = -2450.0f;",
    "constexpr float FrontFenceZ = 110.0f;",
    "constexpr float FrontFenceGapCenterX = 900.0f;",
    "constexpr float FrontFenceGapWidthCm = 3400.0f;",
    "constexpr float LeftFenceCenterX = -3000.0f;",
    "constexpr float LeftFenceLengthCm = 4400.0f;",
    "constexpr float RightFenceCenterX = 3900.0f;",
    "constexpr float RightFenceLengthCm = 2600.0f;",
    "const FVector LegacyFrontFenceScale(104.0f, 0.45f, 2.20f);",
    "Sector->GetComponents<UInstancedStaticMeshComponent>(Components, false);",
    'const FName FencesName(TEXT("Fences"));',
    "Fences->GetInstanceTransform(Index, Transform, false)",
    "Transform.GetLocation().Equals(ExpectedCenter, 4.0f)",
    "Transform.GetScale3D().Equals(LegacyFrontFenceScale, 0.02f)",
    'TEXT("R13_CollegeFrontFenceSplit")',
    "Split->SetStaticMesh(SourceFences->GetStaticMesh());",
    "SourceFences->GetMaterial(0)",
    "Split->SetCollisionProfileName(SourceFences->GetCollisionProfileName());",
    "Split->SetCollisionEnabled(SourceFences->GetCollisionEnabled());",
    "Split->SetCanEverAffectNavigation(true);",
    "Local-space instances match the source Fences component convention exactly",
    "LeftFenceLengthCm / 100.0f",
    "RightFenceLengthCm / 100.0f",
    "SourceFences->RemoveInstance(LegacyIndex)",
    "SplitFence->DestroyComponent();",
    'TEXT("R13_CollegeAccessRepairApplied")',
    "3.4m opening centered on X+900",
    "side/rear fences untouched",
    "GameMode->IsFrontendOnlySession()",
]:
    if token not in access:
        fail(f"college access repair marker missing: {token}")

if 'TEXT("NoCollision")' in access or "SetCanEverAffectNavigation(false)" in access:
    fail("college split front fence must preserve blocking/navigation behavior")

gap_center = const_float(access, "FrontFenceGapCenterX")
gap_width = const_float(access, "FrontFenceGapWidthCm")
left_center = const_float(access, "LeftFenceCenterX")
left_length = const_float(access, "LeftFenceLengthCm")
right_center = const_float(access, "RightFenceCenterX")
right_length = const_float(access, "RightFenceLengthCm")
left_outer = left_center - left_length / 2.0
left_inner = left_center + left_length / 2.0
right_inner = right_center - right_length / 2.0
right_outer = right_center + right_length / 2.0
actual_gap_width = right_inner - left_inner
actual_gap_center = (left_inner + right_inner) / 2.0

if abs(left_outer - (-5200.0)) > 0.01 or abs(right_outer - 5200.0) > 0.01:
    fail(f"split front fence no longer preserves original 104m outer span: {left_outer=} {right_outer=}")
if abs(actual_gap_width - gap_width) > 0.01 or abs(actual_gap_center - gap_center) > 0.01:
    fail(f"split front fence gap constants disagree with segment geometry: width={actual_gap_width}, center={actual_gap_center}")
if abs((left_length + gap_width + right_length) - 10400.0) > 0.01:
    fail("split segments + opening must exactly cover the original 104m front-fence span")
if gap_width <= 2750.0:
    fail("college entrance opening must remain wider than the maximum 2.75m authored stair width")

for token in [
    'TEXT("S01_PATH_COLLEGE_CAMPUS")',
    "FVector(900, 5200, 12)",
    "FVector(8000, 280, 18)",
    "1.0f",
    "Pedestrian-only college campus path; narrowed from the legacy plaza-sized sidewalk proxy",
]:
    if token not in road:
        fail(f"college pedestrian-path marker missing: {token}")
if "FVector(8000, 5900, 18)" in road:
    fail("legacy 80x59 m college sidewalk slab returned; campus path must remain pedestrian-scale")

for token in [
    "void AddCollegeCampusPlanting(const FVector& College",
    "The S01 college path is centered near Y=5200 and only 2.8 m wide",
    "FVector(-1800, 5700, 0)",
    "FVector(2100, 4700, 0)",
    "SetCollisionEnabled(ECollisionEnabled::NoCollision)",
    "SetCanEverAffectNavigation(false)",
    "pedestrian college path/navigation remain clear",
]:
    if token not in civic:
        fail(f"college campus planting/path-clearance marker missing: {token}")
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
    "GameMode->IsFrontendOnlySession()",
    "SetCollisionEnabled(ECollisionEnabled::NoCollision)",
    "SetCanEverAffectNavigation(false)",
    "source collision preserved",
]:
    if token not in stadium:
        fail(f"stadium visual marker missing: {token}")

for label, text in (("college", college), ("access", access), ("stadium", stadium), ("civic", civic)):
    for forbidden in ["FMath::Rand", "FRand"]:
        if forbidden in text:
            fail(f"{label} contains nondeterministic marker: {forbidden}")
    for left, right in (("(", ")"), ("{", "}"), ("[", "]")):
        if text.count(left) != text.count(right):
            fail(f"delimiter mismatch {left}{right} in {label}")

print("R13.5 COLLEGE/STADIUM VISUAL VERIFY: PASS")
print(
    "Checks mixed BuildCollegeSector coordinates, aligned college facade/windows/stairs, exact 3.4m blocking-fence entrance repair, "
    "2.8m campus path/tree clearance, plus stadium visuals; visual overlays remain collision-neutral while the split fence preserves gameplay blocking."
)
