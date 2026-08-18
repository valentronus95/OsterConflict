from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
FILES = {
    "audio_h": SRC / "Public" / "OCR13FrontendAudioSubsystem.h",
    "audio": SRC / "Private" / "OCR13FrontendAudioSubsystem.cpp",
    "layout_h": SRC / "Public" / "OCR13FrontendLayoutRepairSubsystem.h",
    "layout": SRC / "Private" / "OCR13FrontendLayoutRepairSubsystem.cpp",
    "settings_h": SRC / "Public" / "OCAudioUserSettings.h",
}


def fail(message: str) -> None:
    raise SystemExit("R13.6 FRONTEND AUDIO/LAYOUT VERIFY FAIL: " + message)


for label, path in FILES.items():
    if not path.is_file():
        fail(f"missing {label}: {path.relative_to(ROOT)}")

text = {label: path.read_text(encoding="utf-8", errors="replace") for label, path in FILES.items()}
for header in ("audio_h", "layout_h"):
    includes = [line.strip() for line in text[header].splitlines() if line.strip().startswith("#include")]
    if not includes or "generated.h" not in includes[-1]:
        fail(f"{header} generated.h must remain final include")

for token in [
    "UTickableWorldSubsystem",
    "PC->IsFrontendMenuVisible() && PC->GetPawn() == nullptr",
    "SavedWeaponsVolume = AudioSettings->WeaponsVolume",
    "AudioSettings->WeaponsVolume = 0.0f",
    "AudioSettings->WeaponsVolume = SavedWeaponsVolume",
    "/Game/R13/Audio/menu_ambient.menu_ambient",
    "AudioSettings->bMenuMusicEnabled",
    "GetBusVolume(EOCAudioBus::Music)",
    "SpawnSound2D",
    "OnAudioFinished.AddDynamic",
    "HandleMenuMusicFinished",
    "LeavePregameFrontendAudio()",
    "RETURN_QUICK_DECLARE_CYCLE_STAT(UOCR13FrontendAudioSubsystem",
]:
    if token not in text["audio"]:
        fail(f"frontend-audio marker missing: {token}")

if "float MusicVolume" not in text["settings_h"] or "bool bMenuMusicEnabled" not in text["settings_h"]:
    fail("frontend music must honor existing persistent music/menu toggles")

for token in [
    "RepairTimesSeconds[] = { 0.08f, 0.30f, 0.72f }",
    "R13_MenuWorldBlocker",
    "R13_MenuBackground",
    "R13_MenuShade",
    "R13_MenuPanel",
    "FillViewportSlot(Blocker, 70)",
    "FillViewportSlot(Background, 71)",
    "PanelSlot->SetPosition(FVector2D(112.0f, 92.0f))",
    "PanelSlot->SetSize(FVector2D(440.0f, 760.0f))",
    "Root->InvalidateLayoutAndVolatility()",
    "Root->ForceLayoutPrepass()",
    "LastRootSize.SizeSquared() > KINDA_SMALL_NUMBER",
    "FVector2D::Distance(RootSize, LastRootSize) > 4.0f",
    "RETURN_QUICK_DECLARE_CYCLE_STAT(UOCR13FrontendLayoutRepairSubsystem",
]:
    if token not in text["layout"]:
        fail(f"frontend-layout marker missing: {token}")

for forbidden in [
    "LastRootSize.IsNearlyZero()",
    "World.SpawnActor",
    "Server",
]:
    if forbidden in text["layout"]:
        fail(f"frontend layout repair contains unsafe/unnecessary marker: {forbidden}")

print("R13.6 FRONTEND AUDIO/LAYOUT VERIFY: PASS")
print("Checks pregame-only combat-audio suppression, menu-music ownership/restore and bounded startup/resize Slate geometry repair without gameplay mutation.")
