from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
FILES = {
    "presentation_h": SRC / "Public" / "OCR13DeploymentPresentationSubsystem.h",
    "presentation": SRC / "Private" / "OCR13DeploymentPresentationSubsystem.cpp",
    "fps_h": SRC / "Public" / "OCR13FrameRateGuardSubsystem.h",
    "fps": SRC / "Private" / "OCR13FrameRateGuardSubsystem.cpp",
}


def fail(message: str) -> None:
    raise SystemExit("R13.6 DEPLOYMENT PRESENTATION/FPS VERIFY FAIL: " + message)


for label, path in FILES.items():
    if not path.is_file():
        fail(f"missing {label}: {path.relative_to(ROOT)}")

text = {label: path.read_text(encoding="utf-8", errors="replace") for label, path in FILES.items()}
for header in ("presentation_h", "fps_h"):
    includes = [line.strip() for line in text[header].splitlines() if line.strip().startswith("#include")]
    if not includes or "generated.h" not in includes[-1]:
        fail(f"{header} generated.h must remain final include")

for token in [
    "UTickableWorldSubsystem",
    "R13_DeploymentFlowPanel",
    "R13_DeploymentBackdropBlur",
    "R13_DeploymentBackdropShade",
    "Blur->SetBlurStrength(3.5f)",
    "Blur->SetOverrideAutoRadiusCalculation(true)",
    "Blur->SetBlurRadius(4)",
    "Style.Normal.TintColor",
    "Style.Hovered.TintColor",
    "Style.Pressed.TintColor",
    "FlowPanel->SetPadding(FMargin(28.0f))",
    "PC->IsDeploymentPanelVisible() && !PC->IsSettingsVisible()",
    "FillCanvas(Canvas->AddChildToCanvas(Blur), 9188)",
    "FillCanvas(Canvas->AddChildToCanvas(Shade), 9189)",
    "RETURN_QUICK_DECLARE_CYCLE_STAT(UOCR13DeploymentPresentationSubsystem",
]:
    if token not in text["presentation"]:
        fail(f"deployment presentation marker missing: {token}")

for forbidden in [
    "UIRequestRole",
    "UICommitDeployment",
    "ServerCommitDeployment",
    "SpawnActor",
]:
    if forbidden in text["presentation"]:
        fail(f"visual-only deployment presentation mutated gameplay ownership: {forbidden}")

for token in [
    "FrontendCap = 45.0f",
    "MaximumGameplayCap = 60.0f",
    "PreviousEngineCap = GEngine->GetMaxFPS()",
    "FMath::Clamp(PreviousEngineCap, 30.0f, MaximumGameplayCap)",
    "PC->IsFrontendMenuVisible() && PC->GetPawn() == nullptr",
    "GEngine->SetMaxFPS(NewCap)",
    "GEngine->SetMaxFPS(PreviousEngineCap)",
    "RETURN_QUICK_DECLARE_CYCLE_STAT(UOCR13FrameRateGuardSubsystem",
]:
    if token not in text["fps"]:
        fail(f"frame-rate guard marker missing: {token}")

for forbidden in [
    "SaveSettings",
    "SetFrameRateLimit(",
    "MaximumGameplayCap = 120.0f",
]:
    if forbidden in text["fps"]:
        fail(f"runtime thermal guard must not persist/raise FPS settings: {forbidden}")

print("R13.6 DEPLOYMENT PRESENTATION/FPS VERIFY: PASS")
print("Checks main-menu-style translucent deployment presentation with restrained UE background blur plus non-persistent 45/60 FPS thermal caps.")
