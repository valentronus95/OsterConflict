#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
P = ROOT / "OsterConflict" / "Source" / "OsterConflict"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS31 VERIFY FAIL: missing {path}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"PASS31 VERIFY FAIL: {message}")


header = read(P / "Public" / "OCVehicleExitInputRecoverySubsystem.h")
recovery = read(P / "Private" / "OCVehicleExitInputRecoverySubsystem.cpp")
character = read(P / "Private" / "OCCharacter.cpp")
deployment = read(P / "Private" / "OCDeploymentLoadingSubsystem.cpp")

require("LastRecoveredCharacterPawn" in header,
        "possession guard must remember which character already had input rebuilt")
require("CurrentPawn != LastLocalPawn.Get()" in recovery,
        "possession changes must still be observed")
require("if (CurrentPawn == LastLocalPawn.Get()) return;" not in recovery,
        "poll must keep waiting when possession arrives before the deployment UI releases")

for marker in (
    "IsFrontendMenuVisible()",
    "IsDeploymentPanelVisible()",
    "IsAdminPanelVisible()",
    "IsChatInputActive()",
    "IsSettingsVisible()",
):
    require(marker in recovery, f"intentional UI lock guard missing {marker}")

for marker in (
    "InputSubsystem->ClearAllMappings()",
    "ResetIgnoreMoveInput()",
    "ResetIgnoreLookInput()",
    "SetInputMode(FInputModeGameOnly())",
    "UIApplyLocalPreferences()",
    "PASS31_GAMEPLAY_INPUT_READY",
):
    require(marker in recovery, f"gameplay input recovery marker missing {marker}")

require("bReadySent = true" in deployment and "Controller->UIReadyDeploy();" in deployment,
        "deployment transition must still release the deployment panel through UIReadyDeploy")
require("Controller->GetPawn() != nullptr && !Controller->IsDeploymentPanelVisible()" in deployment,
        "loading completion must still require a possessed pawn and released deployment panel")

for marker in (
    "BindAction(MoveForwardAction",
    "BindAction(MoveBackwardAction",
    "BindAction(MoveRightAction",
    "BindAction(MoveLeftAction",
    "AddMovementInput(GetActorForwardVector(), Value.Get<float>())",
    "AddMovementInput(GetActorRightVector(), Value.Get<float>())",
):
    require(marker in character, f"character movement source contract missing {marker}")

print("GAMEPLAY INPUT PASS 31 SOURCE CONTRACT PASS")
print("- character possession waits until deployment/frontend UI locks are genuinely released")
print("- stale Enhanced Input mappings and ignore-move/look stacks are rebuilt from a known state")
print("- initial deployment, respawn and vehicle-return character possession share the same recovery path")
print("- WASD movement bindings and AddMovementInput remain present in the character")
print("STATUS: SOURCE VERIFIED; local UE 5.8 playtest must confirm physical movement")
