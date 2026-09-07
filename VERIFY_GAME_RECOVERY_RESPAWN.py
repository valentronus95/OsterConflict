#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
GM_H = SRC / "Public" / "OCGameMode.h"
GM_CPP = SRC / "Private" / "OCGameMode.cpp"
PC_H = SRC / "Public" / "OCPlayerController.h"
PC_RESPAWN = SRC / "Private" / "OCPlayerControllerRespawn.cpp"

errors: list[str] = []


def read(path: Path) -> str:
    if not path.is_file():
        errors.append(f"missing file: {path.relative_to(ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


gm_h = read(GM_H)
gm_cpp = read(GM_CPP)
pc_h = read(PC_H)
pc_respawn = read(PC_RESPAWN)

# Server gameplay truth: exactly ten seconds, one authoritative timer route, then RestartPlayer.
req("static constexpr float RespawnDelay = 10.0f;" in gm_h,
    "GAME_RECOVERY respawn delay is no longer fixed at 10.0 seconds")
for needle in (
    "DeadCharacter->DetachFromControllerPendingDestroy();",
    "RespawnDelegate.BindUObject(this, &AOCGameMode::RespawnController, DeadController);",
    "GetWorldTimerManager().SetTimer(RespawnTimer, RespawnDelegate, RespawnDelay, false);",
    "void AOCGameMode::RespawnController(AController* ControllerToRespawn)",
    "RestartPlayer(ControllerToRespawn);",
):
    req(needle in gm_cpp, f"server death/respawn ownership contract missing: {needle}")

# Client presentation is driven by the real pawn destruction/possession lifecycle. No spectator/debug pawn is spawned.
for needle in (
    "virtual void OnPossess(APawn* InPawn) override;",
    "virtual void PawnPendingDestroy(APawn* InPawn) override;",
    "virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;",
    "ClientBeginRespawnWait(AActor* DeathViewTarget, float DelaySeconds)",
    "ClientFinishRespawnWait(APawn* NewPawn)",
    "IsRespawnWaiting() const",
    "GetRespawnSecondsRemaining() const",
    "bRespawnWaiting = false",
    "bServerAwaitingRespawnPossession = false",
    "TSharedPtr<SWidget> RespawnOverlayWidget;",
    "void ShowRespawnOverlay();",
    "void HideRespawnOverlay();",
):
    req(needle in pc_h, f"player-controller respawn API/state missing: {needle}")

for needle in (
    "constexpr float GameRecoveryRespawnDelaySeconds = 10.0f;",
    "const bool bConfirmedGameplayDeath = Health && Health->IsDead();",
    "Super::PawnPendingDestroy(InPawn);",
    "bServerAwaitingRespawnPossession = true;",
    "ClientBeginRespawnWait(InPawn, GameRecoveryRespawnDelaySeconds);",
    "GAME_RECOVERY_RESPAWN_WAIT_BEGIN",
    "server_timer_owner=GameMode spectator_pawn=0 gameplay_debugger=0 corpse_view=1",
    "if (!HasAuthority() || !bServerAwaitingRespawnPossession || !IsValid(InPawn))",
    "ClientFinishRespawnWait(InPawn);",
    "GAME_RECOVERY_RESPAWN_POSSESSION_RESTORED",
    "bRespawnWaiting = true;",
    "SetIgnoreMoveInput(true);",
    "SetIgnoreLookInput(true);",
    "SetInputMode(FInputModeGameOnly());",
    "SetViewTargetWithBlend(DeathViewTarget, 0.15f);",
    "ShowRespawnOverlay();",
    "GAME_RECOVERY_RESPAWN_CLIENT_WAIT",
    "respawn_hud=1",
    "bRespawnWaiting = false;",
    "ResetIgnoreMoveInput();",
    "ResetIgnoreLookInput();",
    "SetViewTarget(NewPawn);",
    "ConfigureControllerInput();",
    "GAME_RECOVERY_RESPAWN_CLIENT_READY",
    "input_lock_cleared=1",
    "respawn_hud_removed=1",
    "void AOCPlayerController::ShowRespawnOverlay()",
    "GetRespawnSecondsRemaining()",
    "RESPAWN  {0}",
    "GAME_RECOVERY_RESPAWN_HUD_READY",
    "countdown=1 debug_text=0 spectator_overlay=0",
    "void AOCPlayerController::HideRespawnOverlay()",
    "RemoveViewportWidgetContent",
    "void AOCPlayerController::EndPlay",
):
    req(needle in pc_respawn, f"death-to-respawn client/HUD contract missing: {needle}")

req("StartSpectatingOnly" not in pc_respawn,
    "respawn presentation introduced a spectator pawn owner")
req("GameplayDebugger" not in pc_respawn,
    "respawn presentation introduced GameplayDebugger code")
req("AddOnScreenDebugMessage" not in pc_respawn,
    "respawn countdown regressed to debug text instead of production HUD")

# Ensure the death callback runs after base pending-destroy camera handling so our corpse view remains the final explicit choice.
super_pos = pc_respawn.find("Super::PawnPendingDestroy(InPawn);")
client_wait_pos = pc_respawn.find("ClientBeginRespawnWait(InPawn, GameRecoveryRespawnDelaySeconds);")
req(super_pos >= 0 and client_wait_pos > super_pos,
    "corpse-view client transition must run after base PawnPendingDestroy")

# The production countdown must be created after the local wait state is armed and removed before readiness returns.
wait_state_pos = pc_respawn.find("bRespawnWaiting = true;")
show_hud_pos = pc_respawn.find("ShowRespawnOverlay();")
hide_hud_pos = pc_respawn.find("HideRespawnOverlay();", show_hud_pos + 1)
ready_state_pos = pc_respawn.find("bRespawnWaiting = false;", hide_hud_pos)
req(wait_state_pos >= 0 and show_hud_pos > wait_state_pos,
    "respawn countdown must be created only after the local wait state is active")
req(hide_hud_pos >= 0 and ready_state_pos > hide_hud_pos,
    "respawn countdown must be removed before gameplay readiness is restored")

if errors:
    print("GAME RECOVERY RESPAWN: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("GAME RECOVERY RESPAWN: PASS")
print("- server keeps one fixed 10.0 second death-to-respawn timer and RestartPlayer remains the possession owner")
print("- confirmed death enters a corpse-camera wait state without spawning spectator/debug pawns")
print("- a production HUD countdown shows the remaining respawn time and is removed on respawn/end-play")
print("- death clears conflicting UI/input stacks; respawn restores GameOnly input and the new pawn view")
print("- runtime markers distinguish wait, HUD readiness, possession restoration and client readiness")
print("STATUS: SOURCE CONTRACT ONLY; factual UE 5.8 death/respawn HUD/input acceptance remains pending")
