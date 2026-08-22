from pathlib import Path

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

# Visual hierarchy: roads win over residential noise without deleting real world geometry.
require("TacticalRoadPrimary" in visual and "TacticalRoadSecondary" in visual,
        "primary/secondary road hierarchy missing")
require("NaturalThickness >= 2.25f" in visual, "road hierarchy must be derived from rendered road thickness")
require("3.0f : 1.45f" in visual, "primary and secondary roads need distinct overview thickness")
require("TacticalResidentialOutline" in visual and "TacticalResidentialFill" in visual,
        "residential detail must have its own quieter palette")
require("GetTacticalResidentialRoofs(), TacticalResidentialFill, TacticalResidentialOutline" in visual,
        "residential geometry is not routed through the quieter style")

# Grid hierarchy and player readability.
require("PolishGrid" in polish and "0.075f" in polish, "grid was not visually reduced")
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

# POI and objectives should read as game-map symbols, not debug text.
require("LegacyDot->SetVisibility(ESlateVisibility::Collapsed)" in polish,
        "old POI bullet anchors must be retired")
for token in ("MapPOIIconMuseum", "MapPOIIconStadium", "MapPOIIconPark", "MapPOIIconCenter", "MapPOIIconSilpo"):
    require(token in polish, f"POI vector pin missing: {token}")
require("TActorIterator<AOCCapturePoint>" in polish, "objective backplates are not derived from actual capture actors")
require("ObjectiveBackplateOuter_" in polish and "ObjectiveBackplateInner_" in polish,
        "A/B/C objective backplates missing")
require("Point->GetActorLocation()" in polish, "objective backplates must use actual world position")

# Small production status detail.
require("LIVE · WORLD SYNC" in polish, "live world-sync status chip missing")

print("Tactical Map production polish contracts: PASS")
