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
require("!Character && !Cast<AOCVehicleBase>(PC->GetPawn())" in cpp,
        "map lifecycle must tolerate local possession switching from character to vehicle")

# World alignment contract: actual AOCWorldSectorOster geometry owns map bounds and POI projection.
require("ResolveSectorContentBounds" in cpp, "actual sector content-bounds resolver is missing")
require("GetComponentsBoundingBox(true)" in cpp, "world-sector bounds fallback is missing")
require("TActorIterator<AOCWorldSectorOster>" in cpp, "actual Oster world sector lookup is missing")
require("ResolveSectorWorldLocation" in cpp, "sector-local POIs are not being transformed into world space")
require("MuseumAnchor" in cpp and "StadiumAnchor" in cpp and "ParkAnchor" in cpp, "core Oster POIs are missing")
require("FOCGeoReference::Silpo" in cpp, "Silpo source anchor is missing")
require("ComponentName == TEXT(\"Ground\")" in cpp, "content bounds must not be forced by the 2.4 km ground proxy")
require("FitProjectionBoundsToAspect" in cpp, "map capture/projection aspect fit is missing")

# Actual-game background contract: sector determines extent, but texture captures the current gameplay scene.
require("ASceneCapture2D" in header and "ASceneCapture2D" in cpp, "orthographic world capture actor is missing")
require("UTextureRenderTarget2D" in header and "UTextureRenderTarget2D" in cpp, "map render target is missing")
require("ECameraProjectionMode::Orthographic" in cpp, "world capture must remain orthographic")
require("PRM_RenderScenePrimitives" in cpp, "map background must render the complete current gameplay scene")
require("ClearHiddenComponents" in cpp, "capture hidden-list reset is missing")
require("HideActorComponents(Pawn, true)" in cpp, "dynamic pawns must not be baked into the background")
require("ShowOnlyActorComponents(Sector, true)" not in cpp,
        "regression: capture is restricted to AOCWorldSectorOster and would omit separately-owned landmarks")
require("captured current gameplay world" in cpp, "full-scene capture contract log is missing")
require("CaptureComponent->bCaptureEveryFrame = false" in cpp, "map background must not capture every frame")
require("CaptureComponent->bCaptureOnMovement = false" in cpp, "map background must not recapture on camera movement")
require("CaptureComponent->CaptureScene()" in cpp, "explicit world capture is missing")
require("TacticalMapWorldCapture" in cpp, "captured world texture is not presented in the map widget")
require("ConfigureWorldMap" in header and "ConfigureWorldMap" in cpp, "subsystem/widget world-map configuration contract is missing")
require("MapWidget = CreateWidget<UOCTacticalMapWidget>" in cpp,
        "map widget must rebuild on open so it receives the current world capture")

# Interactive viewport contract: clipping, wheel zoom, LMB pan with clamp, RMB local ping.
require("MapContentCanvas" in header and "TacticalMapContent" in cpp, "transformable map content layer is missing")
require("NativeOnMouseWheel" in header and "NativeOnMouseWheel" in cpp, "mouse-wheel zoom handler is missing")
require("NativeOnMouseButtonDown" in header and "NativeOnMouseButtonDown" in cpp, "map mouse-button handler is missing")
require("NativeOnMouseMove" in header and "NativeOnMouseMove" in cpp, "map drag handler is missing")
require("SetRenderScale(FVector2D(MapZoom, MapZoom))" in cpp, "zoom transform is not applied to map content")
require("SetRenderTranslation(MapPan)" in cpp, "pan transform is not applied to map content")
require("ClampMapPan" in header and "ClampMapPan" in cpp, "map pan clamp is missing")
require("MaxMapZoom" in cpp and "MinMapZoom" in cpp, "map zoom bounds are missing")
require("ViewportToContent" in cpp, "viewport-to-content inverse transform is missing")
require("GetEffectingButton() == EKeys::RightMouseButton" in cpp, "RMB tactical ping input is missing")
require("Projection.UVToWorld" in cpp, "tactical ping does not convert map UV back to world-space")
require("TacticalMapLocalPing" in cpp and "◆ PING" in cpp, "local ping marker presentation is missing")
require("КОЛЕСО  МАСШТАБ" in cpp and "ЛКМ + РУХ  ПЕРЕМІЩЕННЯ" in cpp and "ПКМ  ТАКТИЧНИЙ МАРКЕР" in cpp,
        "implemented map controls are not exposed in the HUD hint bar")

# Replicated squad marker contract: map consumes existing team/squad state instead of a parallel UI roster.
require('#include "OCPlayerState.h"' in cpp, "replicated PlayerState feed is not connected to tactical map")
require("RefreshSquadMarkers" in header and "RefreshSquadMarkers" in cpp, "squad marker refresh path is missing")
require("GetTeamId()" in cpp and "GetSquadId()" in cpp, "squad markers are not filtered by authoritative team/squad state")
require("TActorIterator<AOCCharacter>" in cpp, "squad marker actor feed is missing")
require("SquadMarkers.Find(Character)" in cpp and "SquadMarkers.Add(Character, Marker)" in cpp,
        "squad markers must be cached instead of recreated every tick")
require("Character->IsInVehicle() ? Character->GetCurrentVehicle()" in cpp,
        "squad member vehicle transition is not reflected in marker position")
require('TEXT("▣")' in cpp and 'TEXT("●")' in cpp, "squad/vehicle marker distinction is missing")
require("SpatialActor->GetActorLocation()" in cpp, "dynamic squad marker does not use actual actor world position")

# Replicated objective contract: the map consumes AOCCapturePoint actors, not hardcoded A/B/C screen coordinates.
require('#include "OCCapturePoint.h"' in cpp, "capture-point objective feed is not connected")
require("TMap<TWeakObjectPtr<AOCCapturePoint>" in header, "objective marker cache is missing")
require("RefreshObjectiveMarkers" in header and "RefreshObjectiveMarkers" in cpp, "objective refresh path is missing")
require("TActorIterator<AOCCapturePoint>" in cpp, "objective actor iteration is missing")
require("Point->GetPointId()" in cpp, "objective IDs must come from AOCCapturePoint")
require("Point->GetOwnerTeam()" in cpp, "objective ownership state is not consumed")
require("Point->GetCaptureProgress()" in cpp, "objective capture progress is not consumed")
require("Point->IsContested()" in cpp, "objective contested state is not consumed")
require("ObjectiveMarkers.Find(Point)" in cpp and "ObjectiveMarkers.Add(Point, Marker)" in cpp,
        "objective markers must be cached instead of recreated every tick")
require("WorldToMap(Point->GetActorLocation())" in cpp,
        "objective marker must project the authoritative capture-point actor location")
require("TEXT(\"A\")" not in cpp and "TEXT(\"B\")" not in cpp and "TEXT(\"C\")" not in cpp,
        "tactical map must not hardcode objective IDs")

# Squad order contract: Move/Regroup use their world location; Attack/Defend resolve ObjectiveId to a capture actor.
require("RefreshSquadOrderMarker" in header and "RefreshSquadOrderMarker" in cpp, "squad order marker path is missing")
require("ResolveSquadOrderWorldLocation" in header and "ResolveSquadOrderWorldLocation" in cpp,
        "squad order world-location resolver is missing")
require("PC->GetCurrentSquadOrder()" in cpp, "map is not using the existing squad-order state")
require("Order.Type == EOCSquadOrderType::Move || Order.Type == EOCSquadOrderType::Regroup" in cpp,
        "Move/Regroup world-location path is missing")
require("Order.Type == EOCSquadOrderType::AttackObjective || Order.Type == EOCSquadOrderType::DefendObjective" in cpp,
        "Attack/Defend objective-resolution path is missing")
require("Point->GetPointId() == Order.ObjectiveId" in cpp,
        "Attack/Defend order is not matched to the authoritative capture point")
require("OutWorldLocation = Point->GetActorLocation()" in cpp,
        "Attack/Defend order does not use the resolved objective actor world location")
require("WorldToMap(OrderWorldLocation)" in cpp, "resolved squad order is not projected from world-space")
require("WorldToMap(FVector::ZeroVector)" not in cpp,
        "regression: squad order must never deliberately plot an unresolved objective at world origin")
require("TacticalMapSquadOrder" in cpp, "squad order marker widget is missing")

# Projection contract: one reversible transform is shared by static markers, player, objectives and ping.
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
