from pathlib import Path

ROOT = Path(__file__).resolve().parent

FRONTEND = ROOT / "OsterConflict/Source/OsterConflict/Private/OCR13FrontendMenuSubsystem.cpp"
DEPLOY_PRESENTATION = ROOT / "OsterConflict/Source/OsterConflict/Private/OCR13DeploymentPresentationSubsystem.cpp"
LOADING = ROOT / "OsterConflict/Source/OsterConflict/Private/OCDeploymentLoadingSubsystem.cpp"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"RUNTIME ACCEPTANCE PASS 7 FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8")


def require(text: str, needle: str, where: str) -> None:
    if needle not in text:
        raise SystemExit(f"RUNTIME ACCEPTANCE PASS 7 FAIL: {where}: missing {needle!r}")


frontend = read(FRONTEND)
deploy = read(DEPLOY_PRESENTATION)
loading = read(LOADING)

# Main-menu START remains the single top-level start action. Final deployment is semantically distinct.
require(frontend, 'NSLOCTEXT("OCR13Frontend", "MainStart", "СТАРТ")', "main-menu START")
require(deploy, 'NSLOCTEXT("OCR13DeploymentPresentation", "DeployEnterBattle", "У БІЙ")', "final deployment action")
if 'NSLOCTEXT("OCR13DeploymentPresentation", "DeployStart", "СТАРТ")' in deploy:
    raise SystemExit("RUNTIME ACCEPTANCE PASS 7 FAIL: deployment presentation reverted to a second START action")

# Pre-game settings must never expose the live 3D world through the panel or during the open transition.
require(frontend, 'const bool bSettingsOverGameplay = bPauseMenuActive && bLiveGameplay;', "settings gameplay-context guard")
require(frontend, 'Root->GetWidgetFromName(TEXT("SettingsPanel"))', "settings panel lookup")
require(frontend, 'SettingsPanel->SetBrushColor(FLinearColor(0.045f, 0.055f, 0.066f, 1.0f));', "opaque settings panel")
require(frontend, 'SetPresentationVisibility(false, !bSettingsOverGameplay, bSettingsOverGameplay);', "settings backdrop preservation")
require(frontend, 'const bool bSettingsOverGameplay = bPauseMenuActive && (bGameplayStarted || PC->GetPawn() != nullptr);', "settings click pre-transition guard")

# Deployment loading must fully block the shifting panel and visibly progress from 0 to 100 before removal.
require(loading, 'Scrim->SetBrushColor(FLinearColor(0.006f, 0.009f, 0.012f, 1.0f));', "opaque deployment loading scrim")
require(loading, 'Title->SetText(FText::FromString(TEXT("ЗАВАНТАЖЕННЯ")));', "distinct loading title")
require(loading, 'Widget->AddToViewport(5000);', "blocking loading z-order")
require(loading, 'Widget->SetLoadingProgress(0.0f);', "loading starts at zero")
require(loading, 'if (!bReadySent && Elapsed >= 0.12)', "rendered zero-percent frame")
require(loading, 'Widget->SetLoadingProgress(1.0f);', "loading reaches 100 before removal")
require(loading, 'Controller->GetPawn() != nullptr && !Controller->IsDeploymentPanelVisible()', "possession and deployment-release completion gate")

print("RUNTIME ACCEPTANCE PASS 7 SOURCE CONTRACT PASS")
print("- one main START meaning; final deployment action is У БІЙ")
print("- pre-game settings keep the frontend backdrop and use an opaque panel")
print("- deployment transition fully blocks the underlying panel and reaches 100% after possession")
print("STATUS: SOURCE VERIFIED ONLY; UE 5.8 runtime acceptance is still required")
