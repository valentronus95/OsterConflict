from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"

files = {
    "theme": SRC / "Private" / "OCR13UIThemeSubsystem.cpp",
    "frontend": SRC / "Private" / "OCR13FrontendMenuSubsystem.cpp",
    "viewport": SRC / "Private" / "OCR13UIViewportStabilizerSubsystem.cpp",
    "flow_h": SRC / "Public" / "OCR13DeploymentFlowSubsystem.h",
    "flow_cpp": SRC / "Private" / "OCR13DeploymentFlowSubsystem.cpp",
    "bridge": SRC / "Private" / "OCR13DeploymentSelectionBridge.cpp",
    "controller_h": SRC / "Public" / "OCPlayerController.h",
    "spawn_h": SRC / "Public" / "OCR13SpawnSafetySubsystem.h",
    "spawn_cpp": SRC / "Private" / "OCR13SpawnSafetySubsystem.cpp",
    "world": SRC / "Private" / "OCWorldSectorOster.cpp",
    "geo_h": SRC / "Public" / "OCGeoReference.h",
}


def fail(message: str) -> None:
    raise SystemExit("R13.3 DEPLOYMENT/SPAWN VERIFY FAIL: " + message)


for label, path in files.items():
    if not path.is_file():
        fail(f"missing {label}: {path.relative_to(ROOT)}")

text = {label: path.read_text(encoding="utf-8", errors="replace") for label, path in files.items()}

# One system owns the approved menu image. The theme layer may recognize stale hot-reload objects,
# but it must not instantiate its own copy anymore.
if 'NewObject<UImage>(Root, TEXT("R13_ThemeMenuBackdrop"))' in text["theme"]:
    fail("theme subsystem still instantiates a second menu backdrop")
if "OCR13FrontendMenuSubsystem is the sole owner" not in text["theme"]:
    fail("single menu-backdrop ownership contract is missing")
for token in ["R13_MenuBackground", "R13_MenuWorldBlocker", "/Game/R13/UI/Oster_Menu_BG.Oster_Menu_BG"]:
    if token not in text["frontend"]:
        fail(f"frontend menu ownership marker missing: {token}")

# The deployment handoff must keep the static presentation stable until the safe-spawn confirmation closes it.
for token in [
    "PC->IsDeploymentPanelVisible();",
    "SetWorldRenderingSuppressed(bPreGamePresentationVisible)",
    "if (bWorldRenderingSuppressed == bSuppress) return;",
]:
    if token not in text["viewport"]:
        fail(f"stable deployment presentation marker missing: {token}")

flow_required = [
    "R13_DeploymentFlowPanel",
    "R13_DeploymentStepSwitcher",
    "КРОК %d / 4",
    "ОБЕРІТЬ КОМАНДУ",
    "ОБЕРІТЬ ГРУПУ",
    "ОБЕРІТЬ РОЛЬ",
    "ОБЕРІТЬ ТОЧКУ ПОЯВИ",
    "МАТЧ",
    "ВАШ ВИБІР",
    "СКЛАД ГРУПИ",
    "ГРАВЦІ: %d\\nБОТИ: %d",
    "SetRenderOpacity(0.0f)",
    "UIRequestSquad",
    "UIRequestRole",
    "UICommitDeployment",
    "IsRoleAvailable",
    "IsSpawnAvailable",
    "ЗАПОВНЕНО",
    "ЗАЙНЯТО",
    "НЕДОСТУПНА",
]
for token in flow_required:
    if token not in text["flow_cpp"]:
        fail(f"staged deployment marker missing: {token}")
if '#include "OCGameplayMode.h"' not in text["flow_h"]:
    fail("deployment flow must include the role enum definition explicitly")

for token in [
    "UIRequestSquad(int32 SquadId)",
    "UIRequestRole(EOCPlayerRole Role)",
    "UICommitDeployment()",
    "ClientCompleteDeployment(bool bSuccess)",
    "ServerRequestRole(EOCPlayerRole RequestedRole)",
]:
    if token not in text["controller_h"]:
        fail(f"controller deployment API missing: {token}")

for token in [
    "AOCPlayerController::UICommitDeployment()",
    "ServerSetLobbyReady_Implementation(true)",
    "AOCPlayerController::ClientCompleteDeployment_Implementation",
    "AOCGameMode::RequestRoleChange",
    "RequestedRole != EOCPlayerRole::Rifleman",
    "Other->GetPlayerRole() == RequestedRole",
]:
    if token not in text["bridge"]:
        fail(f"authoritative selection bridge marker missing: {token}")
if "State->GetPlayerRole() == RequestedRole) return true" in text["bridge"]:
    fail("legacy default role can still bypass specialist uniqueness validation")

spawn_required = [
    "LineTraceSingleByChannel",
    "ECC_Visibility",
    "ImpactNormal.Z < 0.55f",
    "SpawnCapsuleCenterOffsetCm = 104.0f",
    "ResolveSafeTeamFallback",
    "Point->IsBaseSpawn()",
    "ClientCompleteDeployment(true)",
    "ClientCompleteDeployment(false)",
    "State->SetLobbyReadyServer(false)",
    "Character->Destroy()",
    "TWeakObjectPtr<AOCPlayerController> PCKey(PC)",
]
for token in spawn_required:
    if token not in text["spawn_cpp"]:
        fail(f"spawn safety marker missing: {token}")

if "UTickableWorldSubsystem" not in text["flow_h"] or "UTickableWorldSubsystem" not in text["spawn_h"]:
    fail("deployment/spawn safety subsystems are not world-tickable")

# The museum is a protected Oster landmark and remains the local geographic origin.
for token in [
    "BuildMuseumAndStadium();",
    "MuseumAnchor()",
    "OSTER LOCAL HISTORY MUSEUM",
    "LandmarkBlocks",
    "LandmarkRoofs",
    "LandmarkWindows",
    'Ground->SetCollisionProfileName(TEXT("BlockAll"))',
]:
    if token not in text["world"]:
        fail(f"museum/world preservation marker missing: {token}")
for token in [
    "Museum/Solonyna estate is the local origin",
    "OriginLatitude = 50.948239",
    "OriginLongitude = 30.883865",
]:
    if token not in text["geo_h"]:
        fail(f"museum origin marker missing: {token}")

print("R13.3 DEPLOYMENT/SPAWN VERIFY: PASS")
print("Checks single menu backdrop ownership, stable staged team->squad->role->spawn UX, authoritative role selection, collision-grounded spawning and museum preservation/origin.")
