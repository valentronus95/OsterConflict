from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
FILES = {
    "widget_h": SRC / "Public" / "OCR13TacticalMapWidget.h",
    "widget": SRC / "Private" / "OCR13TacticalMapWidget.cpp",
    "sub_h": SRC / "Public" / "OCR13TacticalMapSubsystem.h",
    "sub": SRC / "Private" / "OCR13TacticalMapSubsystem.cpp",
}


def fail(message: str) -> None:
    raise SystemExit("R13.6 TACTICAL MAP VERIFY FAIL: " + message)


for label, path in FILES.items():
    if not path.is_file():
        fail(f"missing {label}: {path.relative_to(ROOT)}")

text = {label: path.read_text(encoding="utf-8", errors="replace") for label, path in FILES.items()}
for header in ("widget_h", "sub_h"):
    includes = [line.strip() for line in text[header].splitlines() if line.strip().startswith("#include")]
    if not includes or "generated.h" not in includes[-1]:
        fail(f"{header} generated.h must remain final include")

for token in [
    "ТАКТИЧНА КАРТА  ·  ОСТЕР",
    "M / Ь  ЗАКРИТИ",
    "CompactMinX = -70000.0f",
    "CompactMaxX =  25000.0f",
    "CompactMinY = -25000.0f",
    "CompactMaxY =  50000.0f",
    'FindISM(Sector, TEXT("Roads"))',
    "MaxRoadSegmentsOnMap = 96",
    "WorldToMap",
    "AOCCapturePoint",
    "GetPointId()",
    "AOCWorldSectorOster::MuseumAnchor()",
    "AOCWorldSectorOster::ParkAnchor()",
    "AOCWorldSectorOster::CollegeAnchor()",
    "AOCWorldSectorOster::StadiumAnchor()",
    "GetOwningPlayer()",
    "Pawn->GetActorLocation()",
    "PlayerMarker->SetRenderTransformAngle",
]:
    if token not in text["widget"]:
        fail(f"tactical-map rendering marker missing: {token}")

for token in [
    "UTickableWorldSubsystem",
    "PC->IsInputKeyDown(EKeys::M)",
    "PC->IsFrontendMenuVisible()",
    "PC->IsDeploymentPanelVisible()",
    "CreateWidget<UOCR13TacticalMapWidget>",
    "Widget->AddToViewport(850)",
    "Existing->RemoveFromParent()",
    "same key labelled/typed Ь",
    "RETURN_QUICK_DECLARE_CYCLE_STAT(UOCR13TacticalMapSubsystem",
]:
    if token not in text["sub"]:
        fail(f"tactical-map toggle marker missing: {token}")

for forbidden in [
    "Server",
    "SpawnActor",
]:
    if forbidden in text["sub_h"] + text["sub"]:
        fail(f"local tactical map must not own server/gameplay spawning: {forbidden}")

print("R13.6 TACTICAL MAP VERIFY: PASS")
print("Checks physical M/Ь local toggle, current compact-road schematic, A/B/C, Oster landmarks and live player position/orientation without gameplay mutation.")
