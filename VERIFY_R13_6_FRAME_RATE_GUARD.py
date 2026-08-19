from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
H = SRC / "Public" / "OCR13FrameRateGuardSubsystem.h"
CPP = SRC / "Private" / "OCR13FrameRateGuardSubsystem.cpp"


def fail(message: str) -> None:
    raise SystemExit("R13.6 FRAME RATE GUARD VERIFY FAIL: " + message)


for path in (H, CPP):
    if not path.is_file():
        fail(f"missing source: {path.relative_to(ROOT)}")

h = H.read_text(encoding="utf-8", errors="replace")
cpp = CPP.read_text(encoding="utf-8", errors="replace")

includes = [line.strip() for line in h.splitlines() if line.strip().startswith("#include")]
if not includes or "generated.h" not in includes[-1]:
    fail("generated.h must remain final frame-rate guard header include")

# Tickable lifecycle is declared in the reflected header; runtime thermal-cap behavior is implemented in the .cpp.
if "UTickableWorldSubsystem" not in h:
    fail("frame-rate guard is no longer declared as UTickableWorldSubsystem")

for token in [
    "FrontendCap = 45.0f",
    "MaximumGameplayCap = 60.0f",
    "PreviousEngineCap = GEngine->GetMaxFPS()",
    "FMath::Clamp(PreviousEngineCap, 30.0f, MaximumGameplayCap)",
    "PC->IsFrontendMenuVisible() && PC->GetPawn() == nullptr",
    "FMath::Min(FrontendCap, GameplayCap)",
    "GEngine->SetMaxFPS(NewCap)",
    "GEngine->SetMaxFPS(PreviousEngineCap)",
    "no GameUserSettings config is written or saved here",
    "RETURN_QUICK_DECLARE_CYCLE_STAT(UOCR13FrameRateGuardSubsystem",
]:
    if token not in cpp:
        fail(f"thermal/FPS runtime contract missing: {token}")

for token in [
    "float PreviousEngineCap = 0.0f",
    "float GameplayCap = 60.0f",
    "bool bCapturedPreviousCap = false",
    "virtual void Deinitialize() override",
]:
    if token not in h:
        fail(f"frame-rate lifecycle state missing: {token}")

for forbidden in [
    "SaveSettings",
    "ApplySettings",
    "SetFrameRateLimit",
    "120.0f",
    "240.0f",
]:
    if forbidden in cpp:
        fail(f"playtest thermal guard must not mutate saved settings or restore a high hardcoded cap: {forbidden}")

print("R13.6 FRAME RATE GUARD VERIFY: PASS")
print("Checks tickable lifecycle in the reflected header, 45 FPS pregame / max 60 FPS gameplay runtime cap, respect for an existing lower cap and restoration of the previous engine cap without saving settings.")
