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

for filename in ("landmark.svg", "tree-pine.svg", "shopping-basket.svg", "goal.svg", "map-pin.svg"):
    path = ICON_DIR / filename
    require(path.exists(), f"missing semantic POI icon: {filename}")
    svg = path.read_text(encoding="utf-8")
    require("<svg" in svg and "currentColor" in svg, f"invalid vector icon: {filename}")

# Pass 45 no longer renders residential blockout footprints in the production M-map. The historical
# Z=2 residential dimmer is therefore forbidden because Z=2 now belongs to reference-traced roads.
require("DimOverviewResidentialNoise" not in polish,
        "obsolete residential-noise dimmer returned and may fade Pass 45 roads")
require("road_z2_dim=0" in polish,
        "Pass 45 polish does not assert that the reference-road layer remains undimmed")
require("EdgeSafePadding" in polish and "FMath::Clamp(Position.X" in polish and "FMath::Clamp(Position.Y" in polish,
        "POI edge-safe clamp is missing")
require('TEXT("landmark.svg")' in polish, "museum/culture semantic icon is missing")
require('TEXT("goal.svg")' in polish, "stadium semantic icon is missing")
require('TEXT("tree-pine.svg")' in polish, "park semantic icon is missing")
require('TEXT("shopping-basket.svg")' in polish, "Silpo semantic icon is missing")
require('TEXT("map-pin.svg")' in polish, "central civic semantic icon is missing")
require("MapPOIIconCultureHouse" in polish, "Culture House semantic POI icon is missing")
require("poi_geo_authority=1" in polish and "legacy_worldsector_anchor=0" in polish,
        "POI polish is not tied to the Pass 45 geo-reference authority")
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

print("Tactical Map polish pass 2 + Pass 45 geo-authority contracts: PASS")
