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

# World geometry, not generated screenshot geography, must drive the vector layer.
for getter in (
    "GetTacticalRoads", "GetTacticalSidewalks", "GetTacticalBuildings",
    "GetTacticalResidentialRoofs", "GetTacticalLandmarkBlocks",
    "GetTacticalLandmarkRoofs", "GetTacticalStadiumGeometry", "GetTacticalParkGeometry",
):
    require(getter in world_h, f"world semantic getter missing: {getter}")
    require(getter in visual, f"vector renderer does not consume: {getter}")
require("GetInstanceTransform" in visual, "vector map is not built from actual instance transforms")
require("WorldToMap(WorldLocation)" in visual, "world geometry is not projected through the tactical-map projection")

# Start framing must target central Oster rather than distant hydro/debug extents.
require("ReframeProjectionForCentralOster" in map_h and "ReframeProjectionForCentralOster" in visual,
        "central-Oster production framing is missing")
for anchor in ("MuseumAnchor", "StadiumAnchor", "ParkAnchor", "CollegeAnchor", "FormerCityAdministrationAnchor"):
    require(anchor in visual, f"core framing is missing anchor: {anchor}")
require("FOCGeoReference::Silpo" in visual, "Silpo is missing from central-Oster framing")
require("120000.0f" in visual and "TacticalMapAspect" in visual,
        "production framing needs a bounded north-up 16:9 city-core extent")

# Decluttering / production chrome.
require("TacticalMapPlayerCoordinates" in visual and "Collapsed" in visual,
        "debug world coordinates are still part of the production chrome")
require("ОСТЕР · ТАКТИЧНА СІТКА · NORTH-UP" in visual,
        "production sector heading is missing")
require("TacticalMapProductionAccent" in visual,
        "approved amber tactical accent is missing")
require("ПКМ  ПОСТАВИТИ МІТКУ" in visual,
        "RMB control hint does not describe the tactical marker action")
require("AddLandmarkMarker(TEXT(\"МУЗЕЙ\")" in visual and "AddLandmarkMarker(TEXT(\"СТАДІОН\")" in visual,
        "POI chips are not rebuilt against the production projection")

# Prevent the exact stale-runtime failure demonstrated by the playtest screenshots.
for launcher_name, launcher in (("normal", normal_run), ("sandbox", sandbox_run)):
    require("git fetch origin main" in launcher, f"{launcher_name} launcher does not fetch origin/main before playtest")
    require("LOCAL_HEAD" in launcher and "REMOTE_HEAD" in launcher,
            f"{launcher_name} launcher does not compare local and GitHub main")
    require("Local main is not current GitHub main" in launcher or "LOCAL MAIN" in launcher.upper(),
            f"{launcher_name} launcher does not block stale source playtests")

print("Tactical Map production visual design contracts: PASS")
