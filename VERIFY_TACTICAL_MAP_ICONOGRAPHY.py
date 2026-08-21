from pathlib import Path

ROOT = Path(__file__).resolve().parent
HEADER = ROOT / "OsterConflict/Source/OsterConflict/Public/OCTacticalMapSubsystem.h"
VISUAL = ROOT / "OsterConflict/Source/OsterConflict/Private/OCTacticalMapVisual.cpp"
ICONS_CPP = ROOT / "OsterConflict/Source/OsterConflict/Private/OCTacticalMapIconography.cpp"
GAME_INI = ROOT / "OsterConflict/Config/DefaultGame.ini"
ICON_DIR = ROOT / "OsterConflict/Content/UI/TacticalMap/Icons"


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def read(path: Path) -> str:
    require(path.exists(), f"missing file: {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8")


header = read(HEADER)
visual = read(VISUAL)
icons_cpp = read(ICONS_CPP)
game_ini = read(GAME_INI)

expected_icons = {
    "navigation.svg",
    "users.svg",
    "truck.svg",
    "crosshair.svg",
    "map-pin.svg",
    "mouse.svg",
    "move.svg",
}

for name in sorted(expected_icons):
    svg = read(ICON_DIR / name)
    require("<svg" in svg and "viewBox=\"0 0 24 24\"" in svg, f"invalid vector icon: {name}")
    require('stroke="#FFFFFF"' in svg, f"icon must stay tint-friendly white: {name}")

require((ICON_DIR / "LUCIDE_LICENSE.txt").exists(), "Lucide license notice is missing")
require("InstallTacticalIconography" in header, "widget iconography hook is missing")
require("BuildProductionVisualLayer();\n    InstallTacticalIconography();" in visual,
        "iconography must install after production restyling")
require("FSlateVectorImageBrush" in icons_cpp, "Slate vector brush path is missing")
require("FPaths::ProjectContentDir()" in icons_cpp, "icon resources must resolve from project Content")
require("IFileManager::Get().FileExists" in icons_cpp, "missing-resource guard is required")
require("LegendIconPlayer" in icons_cpp and "LegendIconSquad" in icons_cpp and "LegendIconVehicle" in icons_cpp,
        "legend icon set is incomplete")
require("LegendIconObjective" in icons_cpp and "LegendIconPOI" in icons_cpp,
        "objective/POI icons are missing")
require("TacticalMapKeyCapM" in icons_cpp, "M keycap prompt is missing")
require("TacticalMapIconZoom" in icons_cpp and "TacticalMapIconPan" in icons_cpp and "TacticalMapIconPing" in icons_cpp,
        "footer control icons are incomplete")
require("▲   ГРАВЕЦЬ" not in icons_cpp and "●   ЧЛЕНИ ЗАГОНУ" not in icons_cpp,
        "iconography layer must not reintroduce debug glyph legend rows")
require('+DirectoriesToAlwaysStageAsNonUFS=(Path="UI/TacticalMap/Icons")' in game_ini,
        "packaged builds must stage tactical-map SVG resources")

print("Tactical Map vector iconography contract: PASS")
