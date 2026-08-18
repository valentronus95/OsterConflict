from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"

files = {
    "compact_h": SRC / "Public" / "OCR13CompactOsterSubsystem.h",
    "compact_cpp": SRC / "Private" / "OCR13CompactOsterSubsystem.cpp",
    "ui_h": SRC / "Public" / "OCR13UIViewportStabilizerSubsystem.h",
    "ui_cpp": SRC / "Private" / "OCR13UIViewportStabilizerSubsystem.cpp",
    "bots": SRC / "Private" / "OCR13BotMobilitySubsystem.cpp",
    "civilian": SRC / "Private" / "OCCivilianVehicle.cpp",
    "vehicle_art": SRC / "Private" / "OCR13VehicleArtSubsystem.cpp",
    "launcher": ROOT / "RUN_R11_LISTEN_TEST.cmd",
}


def fail(message: str) -> None:
    raise SystemExit("R13.1 STABILIZATION VERIFY FAIL: " + message)


for label, path in files.items():
    if not path.is_file():
        fail(f"missing {label}: {path.relative_to(ROOT)}")

text = {label: path.read_text(encoding="utf-8", errors="replace") for label, path in files.items()}

compact_required = [
    "CompactMinX = -70000.0f",
    "CompactMaxX =  25000.0f",
    "CompactMinY = -25000.0f",
    "CompactMaxY =  50000.0f",
    "ParkAnchor() + FVector(10000.0f, -7000.0f, 120.0f)",
    "CollegeAnchor() + FVector(5000.0f, -2500.0f, 120.0f)",
    "StadiumAnchor() + FVector(-5000.0f, 2500.0f, 120.0f)",
    "TeamOneBase(-64000.0f, 44000.0f, 160.0f)",
    "TeamTwoBase( 20000.0f,-19000.0f, 160.0f)",
    "Component->RemoveInstance(Index)",
    "Mesh->SetRelativeScale3D(FVector(CompactWidthCm / 100.0f, CompactHeightCm / 100.0f, 2.0f))",
    "IsFrontendOnlySession()",
    "bool bWorldCropped = false",
    "TSet<FName> ObjectivesMoved",
    "ObjectivesMoved.Add(Point->GetPointId())",
    "if (ObjectivesMoved.Num() < ObjectiveLocations.Num())",
    "ScheduleApply(World, RetryDelaySeconds)",
    "const FCompactVehicleSlot CompactVehicleSlots[]",
    "TActorIterator<AOCVehicleSpawnPoint>",
    "IsInsideCompactBounds(SpawnPoint->GetActorLocation())",
    "SpawnPoint->SetActorLocationAndRotation",
    "SpawnPoint->ResetForRoundServer()",
]
for token in compact_required:
    source = text["compact_h"] if token == "bool bWorldCropped = false" else text["compact_cpp"]
    if token not in source:
        fail(f"compact Oster guard missing: {token}")

# Capture-point movement is intentionally non-replicated, so deterministic objective relocation must not live only
# inside the server-only block. This coarse ordering check prevents a future refactor from reintroducing stale clients.
objective_loop = text["compact_cpp"].find("TSet<FName> ObjectivesMoved")
server_only = text["compact_cpp"].find("if (World.GetNetMode() != NM_Client)")
if objective_loop < 0 or server_only < 0 or objective_loop >= server_only:
    fail("capture-point relocation must execute on clients before server-only team-spawn relocation")

ui_required = [
    "DeploymentPanel->SetClipping(EWidgetClipping::ClipToBounds)",
    "const float ColumnWeights[] = { 0.58f, 0.18f, 0.24f }",
    "PC->GetPawn() == nullptr",
    "StartupSuppressedWidgets",
    "R13_MenuWorldBlocker",
    "Slot->SetZOrder(9000)",
    "Slot->SetZOrder(9010)",
    "Slot->SetPosition(FVector2D(90.0f, 60.0f))",
    "Slot->SetSize(FVector2D(470.0f, 780.0f))",
    "RETURN_QUICK_DECLARE_CYCLE_STAT(UOCR13UIViewportStabilizerSubsystem",
]
for token in ui_required:
    if token not in text["ui_cpp"]:
        fail(f"UI viewport stabilization guard missing: {token}")
if '#include "Components/SlateWrapperTypes.h"' not in text["ui_h"]:
    fail("UI stabilizer must include ESlateVisibility definition explicitly")

bot_required = [
    "const int32 LaneIndex = static_cast<int32>(StableHash % 7u) - 3",
    "LaneOffsetCm = static_cast<float>(LaneIndex) * 185.0f",
    "constexpr float SeparationRadiusCm = 350.0f",
    "Separation * 1.25f",
    "Bot->AddMovementInput(FinalDirection, 1.0f, true)",
]
for token in bot_required:
    if token not in text["bots"]:
        fail(f"bot anti-column guard missing: {token}")

vehicle_required = [
    "SuspensionTraceLengthCm = 92.0f",
    "WheelRadiusCm = 42.0f",
    "SpringStiffness = 22000.0f",
    "SuspensionDamping = 3800.0f",
    "ConfigureFourWheelSuspension(205.0f, 95.0f, -106.0f)",
]
for token in vehicle_required:
    if token not in text["civilian"]:
        fail(f"BoxTruck suspension guard missing: {token}")

art_required = [
    "ScaledMeshBottom",
    "DesiredVisualBottom = -PhysicsBody->GetUnscaledBoxExtent().Z - 60.0f",
    "DesiredVisualBottom - ScaledMeshBottom",
]
for token in art_required:
    if token not in text["vehicle_art"]:
        fail(f"vehicle grounding guard missing: {token}")

if "-NoScreenMessages" not in text["launcher"]:
    fail("player-facing launcher must suppress engine preparation/debug screen messages")

if "UTickableWorldSubsystem" not in text["ui_h"] or "UWorldSubsystem" not in text["compact_h"]:
    fail("R13.1 subsystem base classes changed unexpectedly")

print("R13.1 STABILIZATION VERIFY: PASS")
print("Checks compact map/client objective and vehicle-spawn sync, UI isolation/deployment sizing, bot separation, vehicle grounding and BoxTruck suspension.")
