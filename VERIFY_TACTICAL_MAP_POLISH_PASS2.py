from pathlib import Path

ROOT = Path(__file__).resolve().parent
POLISH = ROOT / "OsterConflict/Source/OsterConflict/Private/OCTacticalMapProductionPolish.cpp"
ICONOGRAPHY = ROOT / "OsterConflict/Source/OsterConflict/Private/OCTacticalMapIconography.cpp"
ICON_DIR = ROOT / "OsterConflict/Content/UI/TacticalMap/Icons"


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


polish = POLISH.read_text(encoding="utf-8")
iconography = ICONOGRAPHY.read_text(encoding="utf-8")

for filename in ("landmark.svg", "tree-pine.svg", "shopping-basket.svg", "goal.svg"):
    path = ICON_DIR / filename
    require(path.exists(), f"missing semantic POI icon: {filename}")
    svg = path.read_text(encoding="utf-8")
    require("<svg" in svg and "currentColor" in svg, f"invalid vector icon: {filename}")

require("DimOverviewResidentialNoise" in polish, "overview residential-noise reduction is missing")
require("Size.X <= 6.0f" in polish and "Size.X <= 12.0f" in polish,
        "overview small-footprint thresholds are missing")
require("SetRenderOpacity(0.30f)" in polish and "SetRenderOpacity(0.52f)" in polish,
        "overview footprint opacity hierarchy is missing")
require("EdgeSafePadding" in polish and "FMath::Clamp(Position.X" in polish and "FMath::Clamp(Position.Y" in polish,
        "POI edge-safe clamp is missing")
require('TEXT("landmark.svg")' in polish, "museum semantic icon is missing")
require('TEXT("goal.svg")' in polish, "stadium semantic icon is missing")
require('TEXT("tree-pine.svg")' in polish, "park semantic icon is missing")
require('TEXT("shopping-basket.svg")' in polish, "Silpo semantic icon is missing")
require("FVector2D(38.0f, 38.0f)" in polish and "FVector2D(29.0f, 29.0f)" in polish,
        "stronger A/B/C objective backplates are missing")
require("Point->IsContested() ? 0.42f : 0.22f" in polish,
        "objective contested-state emphasis is missing")
require("PolishGrid" in polish and "0.060f" in polish,
        "quiet grid alpha contract is missing")
require("LegendCanvasSlot->SetSize(FVector2D(270.0f, 310.0f))" in iconography,
        "compact legend panel size is missing")
for y in ("166.0f", "208.0f", "250.0f", "292.0f", "334.0f"):
    require(y in iconography, f"compact legend row placement missing: {y}")
require("compact Lucide vector legend" in iconography,
        "compact iconography contract log is missing")

print("Tactical Map polish pass 2 contracts: PASS")
