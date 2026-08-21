from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"

FILES = {
    "menu_h": SRC / "Public" / "OCR13FrontendMenuSubsystem.h",
    "menu": SRC / "Private" / "OCR13FrontendMenuSubsystem.cpp",
    "viewport_h": SRC / "Public" / "OCR13UIViewportStabilizerSubsystem.h",
    "viewport": SRC / "Private" / "OCR13UIViewportStabilizerSubsystem.cpp",
    "deploy_h": SRC / "Public" / "OCR13DeploymentFlowSubsystem.h",
    "deploy": SRC / "Private" / "OCR13DeploymentFlowSubsystem.cpp",
    "deploy_present_h": SRC / "Public" / "OCR13DeploymentPresentationSubsystem.h",
    "deploy_present": SRC / "Private" / "OCR13DeploymentPresentationSubsystem.cpp",
    "compat": SRC / "Private" / "OCR148DeploymentCompatibility.cpp",
    "controller_h": SRC / "Public" / "OCPlayerController.h",
}


def fail(message: str) -> None:
    raise SystemExit("FRONTEND/DEPLOYMENT REGRESSION GUARD FAIL: " + message)


for label, path in FILES.items():
    if not path.is_file():
        fail(f"missing {label}: {path.relative_to(ROOT)}")

text = {
    label: path.read_text(encoding="utf-8", errors="replace")
    for label, path in FILES.items()
}

# The approved frontend must own startup/pause presentation and permanently suppress the old gray shell.
for token in [
    'TEXT("R13_MenuWorldBlocker")',
    'TEXT("R13_MenuBackground")',
    'TEXT("R13_MenuPanel")',
    'TEXT("FrontendPanel")',
    'LegacyFrontend->SetVisibility(ESlateVisibility::Collapsed)',
    'LegacyFrontend->RemoveFromParent()',
    'bLocalTravelPending = true',
    'SetPresentationVisibility(true, true, false)',
    'open /Game/Maps/OsterConflict_Runtime',
    'DisconnectFromServer()',
]:
    if token not in text["menu"]:
        fail(f"frontend marker missing: {token}")

# World rendering may be hidden only before possession. This prevents the HUD-only black screen after deployment.
for token in [
    'const bool bHasGameplayPawn = IsValid(PC->GetPawn())',
    'const bool bPreGamePresentationVisible = !bHasGameplayPawn',
    'SetWorldRenderingSuppressed(bPreGamePresentationVisible)',
    'SetWorldRenderingSuppressed(false)',
    'GEngine->GameViewport->bDisableWorldRendering = bSuppress',
]:
    if token not in text["viewport"]:
        fail(f"viewport safety marker missing: {token}")

# The recovered staged deployment must stay present and must make the legacy deployment widget inert.
for token in [
    'TEXT("R13_DeploymentFlowPanel")',
    'TEXT("DeploymentPanel")',
    'Legacy->SetRenderOpacity(0.0f)',
    'Legacy->SetIsEnabled(false)',
    'PC->UIRequestSquad(',
    'PC->UIRequestRole(',
    'PC->UICommitDeployment()',
]:
    if token not in text["deploy"]:
        fail(f"deployment marker missing: {token}")

for token in [
    'R13_DeploymentBackdropBlur',
    'Blur->SetBlurStrength(0.0f)',
    'SetPresentationVisible(PC->IsDeploymentPanelVisible() && !PC->IsSettingsVisible())',
]:
    if token not in text["deploy_present"]:
        fail(f"deployment presentation marker missing: {token}")

# Current R14 controller backend must expose the explicit API consumed by the staged UI.
for token in [
    'void UIRequestSquad(int32 SquadId)',
    'void UIRequestRole(EOCPlayerRole RequestedRole)',
    'void UICommitDeployment()',
]:
    if token not in text["controller_h"]:
        fail(f"controller declaration missing: {token}")

for token in [
    'void AOCPlayerController::UIRequestSquad',
    'void AOCPlayerController::UIRequestRole',
    'void AOCPlayerController::UICommitDeployment',
    'UIReadyDeploy()',
]:
    if token not in text["compat"]:
        fail(f"deployment compatibility marker missing: {token}")

print("FRONTEND/DEPLOYMENT REGRESSION GUARD: PASS")
print("Approved frontend, staged deployment and post-possession world rendering safety are present.")
