from pathlib import Path

ROOT = Path(__file__).resolve().parent
HEADER = ROOT / "OsterConflict/Source/OsterConflict/Public/OCTacticalMapSubsystem.h"
CPP = ROOT / "OsterConflict/Source/OsterConflict/Private/OCTacticalMapSubsystem.cpp"
PROJECTION_H = ROOT / "OsterConflict/Source/OsterConflict/Public/OCTacticalMapProjection.h"
PROJECTION_CPP = ROOT / "OsterConflict/Source/OsterConflict/Private/OCTacticalMapProjection.cpp"
TESTS = ROOT / "OsterConflict/Source/OsterConflict/Private/Tests/OCTacticalMapProjectionTests.cpp"
TZ = ROOT / "TACTICAL_MAP_TZ.md"


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def text(path: Path) -> str:
    require(path.exists(), f"missing required file: {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8")


header = text(HEADER)
cpp = text(CPP)
projection_h = text(PROJECTION_H)
projection_cpp = text(PROJECTION_CPP)
tests = text(TESTS)
tz = text(TZ)

# Input contract: M must be event-driven through Enhanced Input, not sampled every frame/timer tick.
require("IMC_TacticalMapRuntime" in cpp, "tactical map mapping context is missing")
require("IA_TacticalMapRuntime" in cpp, "tactical map input action is missing")
require("AddMappingContext(MapMappingContext, 100)" in cpp, "tactical map input context priority must remain explicit")
require("ETriggerEvent::Started" in cpp, "M toggle must be bound as an Enhanced Input Started event")
require("IsInputKeyDown(EKeys::M)" not in cpp, "raw M polling regression detected")
require("PollInput" not in header and "PollInput" not in cpp, "legacy 25 ms PollInput path returned")

# World alignment contract: actual AOCWorldSectorOster geometry owns map bounds and POI projection.
require("ResolveSectorContentBounds" in cpp, "actual sector content-bounds resolver is missing")
require("GetComponentsBoundingBox(true)" in cpp, "world-sector bounds fallback is missing")
require("TActorIterator<AOCWorldSectorOster>" in cpp, "actual Oster world sector lookup is missing")
require("ResolveSectorWorldLocation" in cpp, "sector-local POIs are not being transformed into world space")
require("MuseumAnchor" in cpp and "StadiumAnchor" in cpp and "ParkAnchor" in cpp, "core Oster POIs are missing")
require("FOCGeoReference::Silpo" in cpp, "Silpo source anchor is missing")
require("ComponentName == TEXT(\"Ground\")" in cpp, "content bounds must not be forced by the 2.4 km ground proxy")
require("FitProjectionBoundsToAspect" in cpp, "map capture/projection aspect fit is missing")

# Actual-game map background contract: one orthographic snapshot from the current world sector, never AI geography.
require("ASceneCapture2D" in header and "ASceneCapture2D" in cpp, "orthographic world capture actor is missing")
require("UTextureRenderTarget2D" in header and "UTextureRenderTarget2D" in cpp, "map render target is missing")
require("ECameraProjectionMode::Orthographic" in cpp, "world capture must remain orthographic")
require("ShowOnlyActorComponents(Sector, true)" in cpp, "world capture no longer targets actual Oster sector components")
require("CaptureComponent->bCaptureEveryFrame = false" in cpp, "map background must not capture every frame")
require("CaptureComponent->bCaptureOnMovement = false" in cpp, "map background must not recapture on camera movement")
require("CaptureComponent->CaptureScene()" in cpp, "explicit one-shot world capture is missing")
require("TacticalMapWorldCapture" in cpp, "captured world texture is not presented in the map widget")
require("ConfigureWorldMap" in header and "ConfigureWorldMap" in cpp, "subsystem/widget world-map configuration contract is missing")
require("МАСШТАБ · ПЕРЕМІЩЕННЯ · МАРКЕРИ — НАСТУПНИЙ ЕТАП" in cpp,
        "unfinished controls must not be presented as already working")

# Projection contract: one reversible transform is shared by static markers and the player marker.
require("struct OSTERCONFLICT_API FOCTacticalMapProjection" in projection_h, "projection type missing")
require("WorldToUV" in projection_h and "UVToWorld" in projection_h, "reversible projection API missing")
require("Projection.WorldToUV" in cpp, "widget bypasses the central projection")
require("WorldYawToMapDegrees" in projection_cpp, "player heading projection missing")
require("Round-trip" in tests, "projection round-trip test missing")
require("North yaw keeps marker pointing up" in tests, "north-up heading test missing")

# Approved style/UI contract. This is textual/source-only; runtime visuals still require UE verification.
for token in (
    "TACTICAL MAP",
    "OSTER CONFLICT",
    "ЛЕГЕНДА",
    "ГРАВЕЦЬ",
    "ЧЛЕНИ ЗАГОНУ",
    "ТРАНСПОРТ",
    "ЦІЛЬ",
    "ТОЧКА ІНТЕРЕСУ",
):
    require(token in cpp, f"approved tactical-map UI token missing: {token}")

# Documentation contract: generated concept is style-only and the level/world is the source of truth.
require("джерело істини" in tz.lower(), "TZ must explicitly define a source of truth")
require("actual" in tz.lower() or "фактич" in tz.lower(), "TZ must require actual level/world placement")
require("AI-згенерована географія" in tz, "TZ must reject generated geography as production placement")

print("Tactical Map 2.0 source contracts: PASS")
