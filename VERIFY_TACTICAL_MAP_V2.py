from pathlib import Path

ROOT = Path(__file__).resolve().parent
HEADER = ROOT / "OsterConflict/Source/OsterConflict/Public/OCTacticalMapSubsystem.h"
CPP = ROOT / "OsterConflict/Source/OsterConflict/Private/OCTacticalMapSubsystem.cpp"
CONTROLLER_H = ROOT / "OsterConflict/Source/OsterConflict/Public/OCPlayerController.h"
PING_NETWORK_CPP = ROOT / "OsterConflict/Source/OsterConflict/Private/OCTacticalPingNetwork.cpp"
LOBBY_TYPES_H = ROOT / "OsterConflict/Source/OsterConflict/Public/OCLobbyTypes.h"
PROJECTION_H = ROOT / "OsterConflict/Source/OsterConflict/Public/OCTacticalMapProjection.h"
PROJECTION_CPP = ROOT / "OsterConflict/Source/OsterConflict/Private/OCTacticalMapProjection.cpp"
TESTS = ROOT / "OsterConflict/Source/OsterConflict/Private/Tests/OCTacticalMapProjectionTests.cpp"
TZ = ROOT / "TACTICAL_MAP_TZ.md"
RUNTIME_ACCEPTANCE = ROOT / "TACTICAL_MAP_RUNTIME_ACCEPTANCE.md"
BUILD_HELPER = ROOT / "BUILD_EDITOR_LAUNCHER_UE58.cmd"


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def text(path: Path) -> str:
    require(path.exists(), f"missing required file: {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8")


header = text(HEADER)
cpp = text(CPP)
controller_h = text(CONTROLLER_H)
ping_network_cpp = text(PING_NETWORK_CPP)
lobby_types_h = text(LOBBY_TYPES_H)
projection_h = text(PROJECTION_H)
projection_cpp = text(PROJECTION_CPP)
tests = text(TESTS)
tz = text(TZ)
runtime_acceptance = text(RUNTIME_ACCEPTANCE)
build_helper = text(BUILD_HELPER)

# Input: M is event-driven Enhanced Input, not raw polling, and vehicle possession must not kill the map.
for token in ("IMC_TacticalMapRuntime", "IA_TacticalMapRuntime", "AddMappingContext(MapMappingContext, 100)",
              "ETriggerEvent::Started"):
    require(token in cpp, f"missing tactical-map input contract: {token}")
require("IsInputKeyDown(EKeys::M)" not in cpp and "PollInput" not in cpp and "PollInput" not in header,
        "legacy raw M polling returned")
require("!Character && !Cast<AOCVehicleBase>(PC->GetPawn())" in cpp,
        "map lifecycle must tolerate vehicle possession")

# Geography: actual Oster world owns placement. Large/far proxies must not define the default city framing.
for token in ("ResolveSectorContentBounds", "GetComponentsBoundingBox(true)", "TActorIterator<AOCWorldSectorOster>",
              "ResolveSectorWorldLocation", "MuseumAnchor", "StadiumAnchor", "ParkAnchor", "FOCGeoReference::Silpo",
              "FitProjectionBoundsToAspect"):
    require(token in cpp, f"missing world-alignment contract: {token}")
for excluded in ("Ground", "Waterways", "Bridges", "ReferenceMarkers"):
    require(f'ComponentName == TEXT("{excluded}")' in cpp,
            f"{excluded} must not define default tactical-map framing")

# Background: actual gameplay scene, orthographic, one-shot capture, then tactical treatment in UMG.
for token in ("ASceneCapture2D", "UTextureRenderTarget2D", "ECameraProjectionMode::Orthographic",
              "PRM_RenderScenePrimitives", "ClearHiddenComponents", "HideActorComponents(Pawn, true)",
              "CaptureComponent->CaptureScene()", "TacticalMapWorldCapture", "ConfigureWorldMap"):
    require(token in cpp or token in header, f"missing world-capture contract: {token}")
require("ShowOnlyActorComponents(Sector, true)" not in cpp,
        "capture regressed to one actor and would omit separately-owned landmarks")
require("CaptureComponent->bCaptureEveryFrame = false" in cpp and
        "CaptureComponent->bCaptureOnMovement = false" in cpp,
        "tactical background must remain one-shot, not per-frame")

# Runtime visual polish from the 2026-08-22 playtest screenshots.
require("TacticalMapWorldFilter" in cpp,
        "dark tactical overlay is missing")
require("WorldMapImage->SetColorAndOpacity(FLinearColor(0.34f, 0.40f, 0.36f, 1.0f))" in cpp,
        "raw green world capture is not tinted")
require("UBorder* Chip" in cpp and "SetTextSize(Text, 10)" in cpp,
        "compact POI label chips are missing")
for label in ("ЦЕНТР", "СІЛЬПО", "МУЗЕЙ", "СТАДІОН", "ПАРК"):
    require(f'Label == TEXT("{label}")' in cpp or f'AddLandmarkMarker(TEXT("{label}")' in cpp,
            f"POI label contract missing: {label}")
require("TacticalMapFooterPanel" in cpp and "FVector2D(20.0f, 806.0f)" in cpp,
        "safe in-frame footer is missing")
for hint in ("M  ЗАКРИТИ", "КОЛЕСО  МАСШТАБ", "ЛКМ + РУХ  ПЕРЕМІЩЕННЯ", "ПКМ  ПОСТАВИТИ МІТКУ"):
    require(hint in cpp, f"implemented control hint missing: {hint}")
require('TEXT("◆ МІТКА")' in cpp,
        "RMB ping must render as a tactical marker")
require('Ping.IssuerName.IsEmpty() ? TEXT("PING") : *Ping.IssuerName' not in cpp,
        "network ping must not render the issuer/player name as its map label")

# Interactive viewport: clipped content, wheel zoom, LMB pan, inverse map transform and RMB ping.
for token in ("MapContentCanvas", "TacticalMapContent", "NativeOnMouseWheel", "NativeOnMouseButtonDown",
              "NativeOnMouseMove", "SetRenderScale(FVector2D(MapZoom, MapZoom))",
              "SetRenderTranslation(MapPan)", "ClampMapPan", "ViewportToContent",
              "GetEffectingButton() == EKeys::RightMouseButton", "Projection.UVToWorld"):
    require(token in cpp or token in header, f"missing viewport interaction contract: {token}")
require("MaxMapZoom" in cpp and "MinMapZoom" in cpp, "zoom bounds are missing")

# Squad markers: authoritative team/squad state, cached widgets, actual character/vehicle position.
for token in ('#include "OCPlayerState.h"', "RefreshSquadMarkers", "GetTeamId()", "GetSquadId()",
              "TActorIterator<AOCCharacter>", "SquadMarkers.Find(Character)", "SquadMarkers.Add(Character, Marker)",
              "Character->IsInVehicle() ? Character->GetCurrentVehicle()", "SpatialActor->GetActorLocation()"):
    require(token in cpp or token in header, f"missing squad marker contract: {token}")
require('TEXT("▣")' in cpp and 'TEXT("●")' in cpp, "squad/vehicle marker distinction is missing")

# Objectives: replicated AOCCapturePoint actors, no screen-space A/B/C hardcoding.
for token in ('#include "OCCapturePoint.h"', "RefreshObjectiveMarkers", "TActorIterator<AOCCapturePoint>",
              "Point->GetPointId()", "Point->GetOwnerTeam()", "Point->GetCaptureProgress()", "Point->IsContested()",
              "ObjectiveMarkers.Find(Point)", "ObjectiveMarkers.Add(Point, Marker)",
              "WorldToMap(Point->GetActorLocation())"):
    require(token in cpp or token in header, f"missing objective contract: {token}")
require('TEXT("A")' not in cpp and 'TEXT("B")' not in cpp and 'TEXT("C")' not in cpp,
        "tactical map must not hardcode objective IDs")

# Squad orders: world-space Move/Regroup, objective actor resolution for Attack/Defend.
for token in ("RefreshSquadOrderMarker", "ResolveSquadOrderWorldLocation", "PC->GetCurrentSquadOrder()",
              "Order.Type == EOCSquadOrderType::Move || Order.Type == EOCSquadOrderType::Regroup",
              "Order.Type == EOCSquadOrderType::AttackObjective || Order.Type == EOCSquadOrderType::DefendObjective",
              "Point->GetPointId() == Order.ObjectiveId", "OutWorldLocation = Point->GetActorLocation()",
              "WorldToMap(OrderWorldLocation)"):
    require(token in cpp or token in header, f"missing squad-order contract: {token}")
require("WorldToMap(FVector::ZeroVector)" not in cpp,
        "unresolved objective must never be plotted at world origin")

# Server-routed tactical pings: validated, throttled, squad-scoped, separate from squad orders.
require("struct FOCTacticalPing" in lobby_types_h, "tactical ping payload is missing")
for field in ("WorldLocation", "IssuerName", "Team", "SquadId", "ServerTime"):
    require(field in lobby_types_h, f"tactical ping payload missing {field}")
for token in ("SubmitTacticalPing", "ServerSubmitTacticalPing", "ClientReceiveTacticalPing",
              "GetRecentTacticalPings", "GetTacticalPingRevision"):
    require(token in controller_h, f"player controller tactical-ping API missing: {token}")
for token in ("ServerSubmitTacticalPing_Implementation", "TacticalPingCooldownSeconds",
              "TacticalPingMaxDistanceCm", "FVector::DistSquared2D",
              "RecipientState->GetTeamId() == Ping.Team", "RecipientState->GetSquadId() == Ping.SquadId",
              "ClientReceiveTacticalPing(Ping)", "RecentTacticalPings.Add(Ping)", "++TacticalPingRevision"):
    require(token in ping_network_cpp, f"server/client tactical-ping contract missing: {token}")
require("ServerSubmitSquadOrder" not in ping_network_cpp,
        "tactical ping must not hijack squad orders")
for token in ("RefreshTacticalPingMarkers", "TacticalPingLifetimeSeconds", "GetServerWorldTimeSeconds",
              "PC->GetTacticalPingRevision()", "PC->GetRecentTacticalPings()", "WorldToMap(Ping.WorldLocation)",
              "PC->SubmitTacticalPing(WorldPing)", "TacticalMapLocalPing"):
    require(token in cpp or token in header, f"map tactical-ping renderer missing: {token}")

# Projection: one reversible world/UV transform shared by all layers.
require("struct OSTERCONFLICT_API FOCTacticalMapProjection" in projection_h, "projection type missing")
require("WorldToUV" in projection_h and "UVToWorld" in projection_h, "reversible projection API missing")
require("Projection.WorldToUV" in cpp, "widget bypasses central projection")
require("WorldYawToMapDegrees" in projection_cpp, "heading projection missing")
require("Round-trip" in tests and "North yaw keeps marker pointing up" in tests,
        "projection automation tests are incomplete")

# Approved high-level UI identity.
for token in ("TACTICAL MAP", "OSTER CONFLICT", "ЛЕГЕНДА", "ГРАВЕЦЬ", "ЧЛЕНИ ЗАГОНУ",
              "ТРАНСПОРТ", "ЦІЛЬ", "ТОЧКА ІНТЕРЕСУ"):
    require(token in cpp, f"approved tactical-map UI token missing: {token}")

# Documentation and UE 5.8 acceptance gate.
require("джерело істини" in tz.lower(), "TZ must explicitly define a source of truth")
require("AI-згенерована географія" in tz, "TZ must reject generated geography as production placement")
require("Build.bat" in build_helper and "OsterConflictEditor Win64 Development" in build_helper,
        "UE 5.8 build helper no longer invokes the real editor build")
for token in ("UE_ROOT", "/nopause", "/clean", "-NoHotReloadFromIDE"):
    require(token in build_helper, f"UE 5.8 build helper contract missing: {token}")
require("CODED_UNTESTED" in runtime_acceptance and "VERIFIED RUNTIME" in runtime_acceptance,
        "runtime acceptance status gate is missing")
require("UE 5.8 build result and exact commit SHA" in runtime_acceptance,
        "runtime acceptance must record the exact UE build SHA")

print("Tactical Map 2.0 source contracts: PASS")
