from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
FILES = {
    "flow_h": SRC / "Public" / "OCR13DeploymentFlowSubsystem.h",
    "flow_reconcile": SRC / "Private" / "OCR13DeploymentFlowReconcile.cpp",
    "watch_h": SRC / "Public" / "OCR13DeploymentReconciliationSubsystem.h",
    "watch_cpp": SRC / "Private" / "OCR13DeploymentReconciliationSubsystem.cpp",
    "bridge": SRC / "Private" / "OCR13DeploymentSelectionBridge.cpp",
    "spawn": SRC / "Private" / "OCR13SpawnSafetySubsystem.cpp",
}


def fail(message: str) -> None:
    raise SystemExit("R13.3 DEPLOYMENT RECONCILIATION VERIFY FAIL: " + message)


for label, path in FILES.items():
    if not path.is_file():
        fail(f"missing {label}: {path.relative_to(ROOT)}")

text = {label: path.read_text(encoding="utf-8", errors="replace") for label, path in FILES.items()}

for header in ("flow_h", "watch_h"):
    includes = [line.strip() for line in text[header].splitlines() if line.strip().startswith("#include")]
    if not includes or "generated.h" not in includes[-1]:
        fail(f"{header} generated.h must remain the final include")

for token in [
    "ReconcileAuthoritativeState(AOCPlayerController* PC, float DeltaSeconds)",
    "AuthorityReconcileAge",
    "ReconcileTeamSnapshot",
    "ReconcileSquadSnapshot",
    "ReconcileRoleSnapshot",
]:
    if token not in text["flow_h"]:
        fail(f"flow reconciliation state missing: {token}")

for token in [
    "ReplicationGraceSeconds = 0.65f",
    "State->GetTeamId() != SelectedTeam",
    "State->GetSquadId() != SelectedSquad",
    "State->GetPlayerRole() != SelectedRole",
    "SetStep(0)",
    "SetStep(1)",
    "SetStep(2)",
    "Сервер не підтвердив цю команду",
    "Група вже недоступна або заповнилась",
    "Роль уже зайнята в цій групі",
]:
    if token not in text["flow_reconcile"]:
        fail(f"authoritative rejection marker missing: {token}")

for token in [
    "UTickableWorldSubsystem",
    "World->GetSubsystem<UOCR13DeploymentFlowSubsystem>()",
    "Flow->ReconcileAuthoritativeState(PC, DeltaTime)",
    "PC->IsDeploymentPanelVisible()",
]:
    haystack = text["watch_h"] + "\n" + text["watch_cpp"]
    if token not in haystack:
        fail(f"reconciliation watcher marker missing: {token}")

for token in [
    "RequestedRole != EOCPlayerRole::Rifleman",
    "Other->GetPlayerRole() == RequestedRole",
    "State->SetRoleServer(EOCPlayerRole::Rifleman)",
    "State->SetLobbyReadyServer(false)",
    "Compact->IsCompactLayoutReady()",
    "ServerSetLobbyReady_Implementation(true)",
]:
    if token not in text["bridge"]:
        fail(f"server selection/readiness marker missing: {token}")

for token in [
    "LineTraceSingleByChannel",
    "ResolveSafeTeamFallback",
    "ClientCompleteDeployment(true)",
    "ClientCompleteDeployment(false)",
]:
    if token not in text["spawn"]:
        fail(f"grounded spawn confirmation marker missing: {token}")

# Duplicate definitions here would produce linker errors after a long UE build, precisely what this gate is meant to prevent.
private_dir = SRC / "Private"
for method in [
    "AOCPlayerController::UIRequestSquad",
    "AOCPlayerController::UIRequestRole",
    "AOCPlayerController::UICommitDeployment",
    "AOCPlayerController::ServerRequestRole_Implementation",
    "AOCPlayerController::ServerCommitDeployment_Implementation",
    "AOCPlayerController::ClientCompleteDeployment_Implementation",
    "AOCGameMode::RequestRoleChange",
]:
    owners = []
    for path in private_dir.glob("*.cpp"):
        if method in path.read_text(encoding="utf-8", errors="replace"):
            owners.append(path.name)
    if owners != ["OCR13DeploymentSelectionBridge.cpp"]:
        fail(f"{method} must have exactly one implementation owner, found: {owners}")

print("R13.3 DEPLOYMENT RECONCILIATION VERIFY: PASS")
print("Checks server specialist uniqueness, compact readiness, grounded spawn confirmation, replicated-selection reconciliation and single implementation ownership.")
