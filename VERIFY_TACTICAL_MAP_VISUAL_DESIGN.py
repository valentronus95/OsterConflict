from pathlib import Path

ROOT = Path(__file__).resolve().parent
VISUAL = ROOT / "OsterConflict/Source/OsterConflict/Private/OCTacticalMapVisual.cpp"
MAP_H = ROOT / "OsterConflict/Source/OsterConflict/Public/OCTacticalMapSubsystem.h"
WORLD_H = ROOT / "OsterConflict/Source/OsterConflict/Public/OCWorldSectorOster.h"
NORMAL_RUN = ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd"
SANDBOX_RUN = ROOT / "RUN_R14_MAIN_SANDBOX_TEST.cmd"


def read(path: Path) -> str:
    if not path.exists():
        raise AssertionError(f"missing: {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8")


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


visual = read(VISUAL)
map_h = read(MAP_H)
world_h = read(WORLD_H)
normal_run = read(NORMAL_RUN)
sandbox_run = read(SANDBOX_RUN)

# The production M-map must not use the raw green scene capture as its primary visual.
require("NativeConstruct() override" in map_h, "production visual lifecycle is not wired into the map widget")
require("BuildProductionVisualLayer" in map_h and "BuildProductionVisualLayer" in visual,
        "production vector layer is missing")
require("TacticalMapWorldCapture" in visual and "ESlateVisibility::Collapsed" in visual,
        "raw scene capture is still allowed to dominate the production M-map")
require("TacticalField" in visual and "TacticalBackdrop" in visual,
        "dark tactical base palette is missing")

# World geometry, not generated screenshot geography, still drives the vector layer INSIDE the hard playable frame.
for getter in (
    "GetTacticalRoads", "GetTacticalSidewalks", "GetTacticalBuildings",
    "GetTacticalResidentialRoofs", "GetTacticalLandmarkBlocks",
    "GetTacticalLandmarkRoofs", "GetTacticalStadiumGeometry", "GetTacticalParkGeometry",
):
    require(getter in world_h, f"world semantic getter missing: {getter}")
    require(getter in visual, f"vector renderer does not consume: {getter}")
require("GetInstanceTransform" in visual, "vector map is not built from actual instance transforms")
require("WorldToMap(WorldLocation)" in visual, "world geometry is not projected through the tactical-map projection")

# Pass 44 supersedes the old component/anchor auto-fit. The M-map frame is now the exact user-approved
# playable boundary. Landmarks are rendered as POIs inside that frame, but they must not enlarge it.
require("ReframeProjectionForCentralOster" in map_h and "ReframeProjectionForCentralOster" in visual,
        "central-Oster production framing is missing")
for marker in (
    "Pass44PlayableMinX = -78000.0f",
    "Pass44PlayableMaxX =  18000.0f",
    "Pass44PlayableMinY = -12000.0f",
    "Pass44PlayableMaxY =  82000.0f",
    "FBox2D CompactWorldBounds(ForceInit)",
    "Projection.WorldMin = CompactWorldBounds.Min",
    "Projection.WorldMax = CompactWorldBounds.Max",
    "PASS44_TACTICAL_MAP_COMPACT_BOUNDS_READY",
    "auto_component_fit=0",
    "old_min_halfwidth_800m=0",
):
    require(marker in visual, f"Pass 44 hard tactical framing missing: {marker}")
for stale in (
    "AccumulateComponentBounds2D",
    "HalfSize += FVector2D(30000.0f, 26000.0f)",
    "FMath::Clamp(HalfSize.X, 80000.0f, 120000.0f)",
):
    require(stale not in visual, f"superseded tactical auto-fit returned: {stale}")
require("ResolveWorldMapSource() && CaptureWorldMap()" in map_h,
        "Pass 44 map snapshot does not re-resolve compact central-Oster bounds before capture")

# Current user-facing POIs remain inside the compact frame. Their presence is a display contract, not a framing input.
for poi in (
    'AddLandmarkMarker(TEXT("МУЗЕЙ")',
    'AddLandmarkMarker(TEXT("СТАДІОН")',
    'AddLandmarkMarker(TEXT("ПАРК")',
    'AddLandmarkMarker(TEXT("ЦЕНТР")',
):
    require(poi in visual, f"production POI chip missing: {poi}")
require("FOCGeoReference::Silpo" in visual, "Silpo POI is missing from the compact tactical map")

# Decluttering / production chrome.
require("TacticalMapPlayerCoordinates" in visual and "Collapsed" in visual,
        "debug world coordinates are still part of the production chrome")
require("ОСТЕР · ТАКТИЧНА СІТКА · NORTH-UP" in visual,
        "production sector heading is missing")
require("TacticalMapProductionAccent" in visual,
        "approved amber tactical accent is missing")
require("ПКМ  ПОСТАВИТИ МІТКУ" in visual,
        "RMB control hint does not describe the tactical marker action")

# Prevent stale-runtime testing without forbidding legitimate pre-merge runtime-fix branches.
# Normal gameplay fetches/compares the exact current allowed branch; Sandbox remains intentionally main-only.
for marker in (
    'set "FETCH_BRANCH="',
    'set "REMOTE_REF="',
    'git fetch origin "%FETCH_BRANCH%"',
    'git rev-parse "%REMOTE_REF%"',
    'Local %CURRENT_BRANCH% is not current GitHub %REMOTE_REF%',
    '/C:"fix/runtime-map-spawn-fps-assets-"',
):
    require(marker in normal_run, f"normal launcher branch-aware stale-source guard missing: {marker}")
require("LOCAL_HEAD" in normal_run and "REMOTE_HEAD" in normal_run,
        "normal launcher does not compare local and exact remote branch")

for marker in (
    "git fetch origin main",
    "git rev-parse origin/main",
    "Local main is not current GitHub main",
):
    require(marker in sandbox_run, f"sandbox main-only stale-source guard missing: {marker}")
require("LOCAL_HEAD" in sandbox_run and "REMOTE_HEAD" in sandbox_run,
        "sandbox launcher does not compare local and GitHub main")

print("Tactical Map production visual design + Pass 44 hard-bound launch contracts: PASS")
