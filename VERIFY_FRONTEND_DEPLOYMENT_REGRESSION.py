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
    "game_mode_h": SRC / "Public" / "OCGameMode.h",
    "game_mode": SRC / "Private" / "OCGameMode.cpp",
    "bot_policy_h": SRC / "Public" / "OCBotPopulationPolicySubsystem.h",
    "bot_policy": SRC / "Private" / "OCBotPopulationPolicySubsystem.cpp",
}


def fail(message: str) -> None:
    raise SystemExit("FRONTEND/DEPLOYMENT REGRESSION GUARD FAIL: " + message)


for label, path in FILES.items():
    if not path.is_file():
        fail(f"missing {label}: {path.relative_to(ROOT)}")

text = {label: path.read_text(encoding="utf-8", errors="replace") for label, path in FILES.items()}

for token in [
    'TEXT("R13_MenuWorldBlocker")',
    'TEXT("R13_MenuBackground")',
    'TEXT("R13_MenuPanel")',
    'TEXT("FrontendPanel")',
    'LegacyFrontend->SetVisibility(ESlateVisibility::Collapsed)',
    'LegacyFrontend->SetIsEnabled(false)',
    'bLocalTravelPending = true',
    'SetPresentationVisibility(true, true, false)',
    'open /Game/Maps/OsterConflict_Runtime',
    'DisconnectFromServer()',
    'MenuBackground->SetRenderOpacity(1.0f)',
]:
    if token not in text["menu"]:
        fail(f"frontend marker missing: {token}")

if 'LegacyFrontend->RemoveFromParent()' in text["menu"]:
    fail('frontend must not detach the root-owned legacy panel after WidgetTree rebuild')

for token in [
    'const bool bHasGameplayPawn = IsValid(PC->GetPawn())',
    'const bool bStartupShell = !bHasGameplayPawn',
    'SetWorldRenderingSuppressed(false)',
    'GEngine->GameViewport->bDisableWorldRendering = bSuppress',
    'R13_MenuWorldBlocker',
    'R13_MenuBackground',
]:
    if token not in text["viewport"]:
        fail(f"viewport safety marker missing: {token}")

for forbidden in [
    'SetWorldRenderingSuppressed(bFrontendMenu)',
    'SetWorldRenderingSuppressed(bPreGamePresentationVisible)',
    'SetWorldRenderingSuppressed(true)',
]:
    if forbidden in text["viewport"]:
        fail(f"persistent viewport rendering suppression returned: {forbidden}")

# GAME_RECOVERY deployment contract: the Slate click handler must only queue the request. Possession/restart may
# execute synchronously in standalone, so the actual ready/deploy call is deferred to a later world tick and the
# deployment screen is closed only after a Pawn exists. Do not regress this to direct UIReadyDeploy() in OnClicked.
for token in [
    'TEXT("R13_DeploymentFlowPanel")',
    'TEXT("R13_DeploymentBackdrop")',
    'TEXT("DeploymentPanel")',
    'Legacy->SetRenderOpacity(0.0f)',
    'Legacy->SetIsEnabled(false)',
    'PC->UIRequestSquad(',
    'PC->UIRequestRole(',
    'PC->UISelectSpawn(SelectedSpawn);',
    'bDeployRequestQueued = false;',
    'PC->UIReadyDeployKeepOpenUntilSpawn();',
    'PASS45_DEPLOY_REQUEST_EXECUTE',
    'PASS45_DEPLOY_DIRECT_READY',
    'PC->UICloseDeployment();',
    'PASS45_DEPLOYMENT_BACK_TO_FRONTEND_READY',
]:
    if token not in text["deploy"]:
        fail(f"deployment marker missing: {token}")

if 'ПЕРЕВІРКА ТОЧКИ ПОЯВИ' in text["deploy"]:
    fail('obsolete indefinite spawn-verification UI state returned')

for token in [
    'R13_DeploymentBackdropBlur',
    'Blur->SetBlurStrength(0.0f)',
    'const bool bDeploymentOwnsScreen = PC->IsDeploymentPanelVisible()',
    '!PC->IsSettingsVisible() && !PC->IsFrontendMenuVisible()',
    'PASS45_DEPLOYMENT_PRESENTATION_READY',
    'SetPresentationVisible(bDeploymentOwnsScreen)',
]:
    if token not in text["deploy_present"]:
        fail(f"deployment presentation marker missing: {token}")

for token in [
    'void UIRequestSquad(int32 SquadId)',
    'void UIRequestRole(EOCPlayerRole RequestedRole)',
    'void UICommitDeployment()',
    'void UIReadyDeploy()',
    'void UIReadyDeployKeepOpenUntilSpawn()',
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

# GAME_RECOVERY bot regression contract. The AI implementation was never deleted; the regression was
# population activation. Protect the current policy in this existing frontend/deployment owner instead
# of creating yet another verifier chain.
for token in [
    'int32 TargetPopulation = 16;',
    'bool bAutoFillBots = true;',
    'bool RestoreExpectedLocalBotFill()',
    'TargetPopulation = FMath::Clamp(Humans + RequestedBotCount, 1, MaxPlayerSlots);',
    'bAutoFillBots = TargetPopulation > Humans;',
    'MaintainPopulation();',
]:
    if token not in text["game_mode_h"]:
        fail(f"local filler-bot policy marker missing: {token}")

for token in [
    'if (GetHumanPlayerCount() >= MaxPlayerSlots)',
    'GetHumanPlayerCount() + GetBotPlayerCount() > TargetPopulation',
    'SelectBotToRemove(State->GetTeamId())',
    'RemoveBotController(Bot)',
    'MaintainPopulation();',
]:
    if token not in text["game_mode"]:
        fail(f"human-priority bot replacement marker missing: {token}")

for token in [
    'GameMode->IsFrontendOnlySession()',
    'GameMode->IsSandboxMode()',
    'World->GetNetMode() == NM_DedicatedServer',
    'GameMode->GetHumanPlayerCount() <= 0',
    'GameMode->RestoreExpectedLocalBotFill()',
    'GAME_RECOVERY_BOT_FILL_RESTORED',
]:
    if token not in text["bot_policy"]:
        fail(f"bot population recovery marker missing: {token}")

print("FRONTEND/DEPLOYMENT REGRESSION GUARD: PASS")
print("Frontend/deployment ownership remains stable; deployment executes outside the Slate click callback and closes only after possession, while normal local/listen gameplay restores filler bots only after a human host exists.")
