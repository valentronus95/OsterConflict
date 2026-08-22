from pathlib import Path

ROOT = Path(__file__).resolve().parent

FRONTEND = ROOT / "OsterConflict/Source/OsterConflict/Private/OCR13FrontendMenuSubsystem.cpp"
DEPLOY_PRESENTATION = ROOT / "OsterConflict/Source/OsterConflict/Private/OCR13DeploymentPresentationSubsystem.cpp"
LOADING = ROOT / "OsterConflict/Source/OsterConflict/Private/OCDeploymentLoadingSubsystem.cpp"
SPAWN_GUARD_H = ROOT / "OsterConflict/Source/OsterConflict/Public/OCMuseumSpawnGuardSubsystem.h"
SPAWN_GUARD_CPP = ROOT / "OsterConflict/Source/OsterConflict/Private/OCMuseumSpawnGuardSubsystem.cpp"
TEAM_SPAWN = ROOT / "OsterConflict/Source/OsterConflict/Private/OCTeamSpawnPoint.cpp"
GAME_MODE = ROOT / "OsterConflict/Source/OsterConflict/Private/OCGameMode.cpp"
LAUNCHER = ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd"


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
spawn_guard_h = read(SPAWN_GUARD_H)
spawn_guard = read(SPAWN_GUARD_CPP)
team_spawn = read(TEAM_SPAWN)
game_mode = read(GAME_MODE)
launcher = read(LAUNCHER)

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

# BASE spawn must have two layers of protection: the normal canonical relocation plus an automatic gameplay-world
# guard that repairs/creates missing server BASE actors before a human can fall into GameMode's legacy origin fallback.
require(team_spawn, 'const FVector Museum = AOCWorldSectorOster::MuseumAnchor();', "canonical Museum BASE anchor")
require(team_spawn, 'SpawnRuntimeBaseWeaponRack(*this, TeamId);', "Museum BASE weapon rack")
require(game_mode, 'const FVector FallbackLocation = bTeamTwo ? FVector(2800.0f, 0.0f, 120.0f) : FVector(-2800.0f, 0.0f, 120.0f);', "legacy fallback is explicitly tracked")
require(spawn_guard_h, 'class OSTERCONFLICT_API UOCMuseumSpawnGuardSubsystem : public UTickableWorldSubsystem', "Museum spawn guard subsystem")
require(spawn_guard, 'TActorIterator<AOCWorldSectorOster>', "guard waits for real gameplay sector")
require(spawn_guard, 'FVector::DistSquared2D(Point->GetActorLocation(), Museum) > FMath::Square(3500.0f)', "stale BASE relocation guard")
require(spawn_guard, 'Point->ConfigureServer(Team, true, NAME_None);', "canonical BASE repair path")
require(spawn_guard, 'EnsureTeamBase(EOCTeam::TeamOne, bHasTeamOneBase);', "TeamOne BASE guarantee")
require(spawn_guard, 'EnsureTeamBase(EOCTeam::TeamTwo, bHasTeamTwoBase);', "TeamTwo BASE guarantee")
require(spawn_guard, 'PASS7_MUSEUM_BASES_READY', "runtime Museum BASE evidence marker")

# Runtime acceptance must be executable before merge. The old launcher forced main and therefore tested stale code.
require(launcher, 'findstr /B /I /C:"fix/runtime-acceptance-"', "runtime-acceptance branch allow-list")
require(launcher, 'set "REMOTE_REF=origin/%CURRENT_BRANCH%"', "branch-specific remote ref")
require(launcher, 'git fetch origin "%FETCH_BRANCH%"', "branch-specific fetch")
require(launcher, 'git rev-parse "%REMOTE_REF%"', "branch-specific freshness check")
require(launcher, 'VERIFY_RUNTIME_ACCEPTANCE_PASS_7.py', "Pass 7 pre-build verifier hook")
if 'Normal gameplay playtest must run from branch main.' in launcher:
    raise SystemExit("RUNTIME ACCEPTANCE PASS 7 FAIL: launcher still hard-blocks pre-merge runtime acceptance")

print("RUNTIME ACCEPTANCE PASS 7 SOURCE CONTRACT PASS")
print("- one main START meaning; final deployment action is У БІЙ")
print("- pre-game settings keep the frontend backdrop and use an opaque panel")
print("- deployment transition fully blocks the underlying panel and reaches 100% after possession")
print("- gameplay world repairs/creates authoritative Museum BASE spawns before legacy origin fallback can be needed")
print("- normal frontend playtest can run from a fresh fix/runtime-acceptance-* branch before merge")
print("STATUS: SOURCE VERIFIED ONLY; UE 5.8 runtime acceptance is still required")
