from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent
VISUAL = ROOT / "OsterConflict/Source/OsterConflict/Private/OCTacticalMapVisual.cpp"
POLISH = ROOT / "OsterConflict/Source/OsterConflict/Private/OCTacticalMapProductionPolish.cpp"
HEADER = ROOT / "OsterConflict/Source/OsterConflict/Public/OCTacticalMapSubsystem.h"


def read(path: Path) -> str:
    if not path.exists():
        raise AssertionError(f"missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8")


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


visual = read(VISUAL)
polish = read(POLISH)
header = read(HEADER)

# Production stage wiring.
require("void ApplyProductionPolish();" in header, "production polish API missing")
require("InstallTacticalIconography();\n    ApplyProductionPolish();" in visual,
        "production polish must run after vector map and iconography construction")

# Pass 45 visual hierarchy is explicit in the reference-traced road data rather than inferred from old ISM thickness.
require("TacticalRoadPrimary" in visual and "TacticalRoadSecondary" in visual,
        "primary/secondary road hierarchy missing")
require("WidthMeters" in visual and "Segment.bPrimary" in visual,
        "reference road hierarchy does not carry explicit width/primary identity")
require("Segment.bPrimary ? 4.0f : 2.0f" in visual,
        "primary and secondary reference roads need distinct overview thickness")
require("GetTacticalResidentialRoofs()" not in visual,
        "retired residential blockout geometry returned to the production M-map")
require("road_z2_dim=0" in polish,
        "Pass 45 polish does not prove that Z=2 reference roads are protected from old residential dimming")

# Grid hierarchy and player readability.
grid_match = re.search(r"PolishGrid\([^;]*?,\s*([0-9.]+)f\);", polish)
require(grid_match is not None, "PolishGrid declaration missing")
grid_alpha = float(grid_match.group(1))
require(grid_alpha <= 0.075, f"grid alpha regressed above 0.075: {grid_alpha}")
require("MapGridV_%02d" in polish and "MapGridH_%02d" in polish, "grid line restyle loop missing")
require("SetPolishTextSize(PlayerMarker, 24)" in polish, "player marker is not strengthened")
require("SetShadowOffset" in polish and "SetShadowColorAndOpacity" in polish,
        "player marker outline/shadow readability treatment missing")

# Proper scale-bar presentation replaces the old single-number sector width.
require("LegacyScale->SetVisibility(ESlateVisibility::Collapsed)" in polish,
        "legacy scale readout must be hidden")
for token in ("TacticalScaleBarMain", "TacticalScaleBarTick0", "TacticalScaleBarTickHalf", "TacticalScaleBarTickEnd"):
    require(token in polish, f"scale bar element missing: {token}")
require("BarMeters = WidthMeters <= 1800.0f ? 250.0f : 500.0f" in polish,
        "overview scale bar must choose a useful real-world distance")

# POI text and polish icons must share one FOCGeoReference authority. No old world-sector anchor may split them.
require("PASS45_TACTICAL_POLISH_GEO_AUTHORITY_READY" in polish,
        "Pass 45 tactical polish geo-authority marker missing")
require("legacy_worldsector_anchor=0" in polish and "poi_geo_authority=1" in polish,
        "tactical polish does not assert one geo authority")
require("AOCWorldSectorOster::" not in polish,
        "old world-sector POI anchor returned to tactical polish")
for geo in (
    "FOCGeoReference::Museum()",
    "FOCGeoReference::Stadium()",
    "FOCGeoReference::CentralPark()",
    "FOCGeoReference::CultureHouse()",
    "FOCGeoReference::FormerCityAdministration()",
    "FOCGeoReference::Silpo()",
):
    require(geo in polish, f"tactical polish missing geo POI authority: {geo}")

# POI and objectives should read as game-map symbols, not debug text.
require("LegacyDot->SetVisibility(ESlateVisibility::Collapsed)" in polish,
        "old POI bullet anchors must be retired")
for token in (
    "MapPOIIconMuseum", "MapPOIIconStadium", "MapPOIIconPark",
    "MapPOIIconCultureHouse", "MapPOIIconCenter", "MapPOIIconSilpo",
):
    require(token in polish, f"POI vector icon missing: {token}")
require("TActorIterator<AOCCapturePoint>" in polish, "objective backplates are not derived from actual capture actors")
require("ObjectiveBackplateOuter_" in polish and "ObjectiveBackplateInner_" in polish,
        "A/B/C objective backplates missing")
require("Point->GetActorLocation()" in polish, "objective backplates must use actual world position")

# Small production status detail.
require("LIVE · WORLD SYNC" in polish, "live world-sync status chip missing")

print("Tactical Map production polish + Pass 45 geo-authority contracts: PASS")
