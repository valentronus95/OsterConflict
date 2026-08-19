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

# Source topology and its intentionally mixed center/rotation conventions.
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
]:
    require(world, token, "BuildCollegeSector/source topology")

# Visual facade must mirror those two source placement modes instead of forcing one transform convention on all parts.
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

# Prove why the access repair is currently necessary: the 45 cm-deep front fence overlaps the lowest 220 cm-deep step.
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
    require(access_h, token, "college access header")

for token in [
    "constexpr float CollegeAccessRepairDelaySeconds = 2.40f;",
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
    "SourceFences->RemoveInstance(LegacyIndex)",
    "SplitFence->DestroyComponent();",
    'TEXT("R13_CollegeAccessRepairApplied")',
    "34m opening centered on X+900cm",
    "27.5m maximum stair width clears the opening",
    "side/rear fences untouched",
    "GameMode->IsFrontendOnlySession()",
]:
    require(access, token, "college access repair")

if 'TEXT("NoCollision")' in access or "SetCanEverAffectNavigation(false)" in access:
    fail("college split front fence must preserve blocking/navigation behavior")

# 3400 cm = 34 m. Verify the two replacement segments exactly preserve the 104 m outer fence span and opening.
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

if abs(left_outer + 5200.0) > 0.01 or abs(right_outer - 5200.0) > 0.01:
    fail(f"split front fence no longer preserves original 104m outer span: {left_outer=} {right_outer=}")
if abs(actual_gap_width - gap_width) > 0.01 or abs(actual_gap_center - gap_center) > 0.01:
    fail(f"split gap constants disagree with segment geometry: width={actual_gap_width}, center={actual_gap_center}")
if abs((left_length + gap_width + right_length) - 10400.0) > 0.01:
    fail("split segments + opening must exactly cover the original 104m front-fence span")
if gap_width <= 2750.0:
    fail("college entrance opening must remain wider than the maximum 27.5m authored stair width")

# The campus connection remains pedestrian-scale and the two trees formerly on its centerline remain displaced.
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

# Stadium invariants retained by the shared verifier.
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
    "Checks mixed college source coordinates, aligned facade/windows/stairs, exact 34m blocking-fence opening, "
    "2.8m campus path/tree clearance, plus stadium visuals; visual overlays remain collision-neutral while the split fence preserves gameplay blocking."
)
