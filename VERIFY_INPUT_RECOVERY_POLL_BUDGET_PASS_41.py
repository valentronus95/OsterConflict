#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
HEADER = SRC / "Public" / "OCVehicleExitInputRecoverySubsystem.h"
CPP = SRC / "Private" / "OCVehicleExitInputRecoverySubsystem.cpp"
LAUNCHER = ROOT / "RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS41 VERIFY FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        raise SystemExit(f"PASS41 VERIFY FAIL: {label}: missing {needle!r}")


def forbid(text: str, needle: str, label: str) -> None:
    if needle in text:
        raise SystemExit(f"PASS41 VERIFY FAIL: {label}: forbidden {needle!r}")


header = read(HEADER)
cpp = read(CPP)
launcher = read(LAUNCHER)

for needle in (
    "bool bPollBudgetLogged = false;",
    "void ScheduleNextPoll(float DelaySeconds);",
    "FTimerHandle PossessionPollTimer;",
):
    require(header, needle, "adaptive poll state")

for needle in (
    "constexpr float TransitionPollSeconds = 0.05f;",
    "constexpr float StablePollSeconds = 0.10f;",
    "ScheduleNextPoll(TransitionPollSeconds);",
    "ScheduleNextPoll(StablePollSeconds);",
    "ScheduleNextPoll(bLastPawnWasVehicle ? TransitionPollSeconds : StablePollSeconds);",
    "PASS41_INPUT_RECOVERY_POLL_BUDGET_READY",
    "transition_hz=20 stable_hz=10 repeating_timer=0",
    "PASS31_GAMEPLAY_INPUT_READY",
    "World->GetTimerManager().ClearTimer(PossessionPollTimer);",
):
    require(cpp, needle, "adaptive input recovery polling")

schedule_start = cpp.find("void UOCVehicleExitInputRecoverySubsystem::ScheduleNextPoll")
poll_start = cpp.find("void UOCVehicleExitInputRecoverySubsystem::PollLocalPossession")
if schedule_start < 0 or poll_start <= schedule_start:
    raise SystemExit("PASS41 VERIFY FAIL: could not isolate ScheduleNextPoll")
schedule_body = cpp[schedule_start:poll_start]
require(schedule_body, "World->GetTimerManager().SetTimer", "one-shot timer scheduling")
require(schedule_body, "false);", "timer must be one-shot")
forbid(schedule_body, "true);", "input recovery timer must not repeat forever")

# Do not regress to the old fixed 20 Hz repeating timer in world begin play.
begin_start = cpp.find("void UOCVehicleExitInputRecoverySubsystem::OnWorldBeginPlay")
deinit_start = cpp.find("void UOCVehicleExitInputRecoverySubsystem::Deinitialize")
if begin_start < 0 or deinit_start <= begin_start:
    raise SystemExit("PASS41 VERIFY FAIL: could not isolate OnWorldBeginPlay")
begin_body = cpp[begin_start:deinit_start]
forbid(begin_body, "SetTimer(", "world begin play must use adaptive ScheduleNextPoll")
require(begin_body, "ScheduleNextPoll(TransitionPollSeconds);", "initial transition response")

# Stable gameplay should not run the transition-rate poll forever. UI/vehicle transitions retain 20 Hz.
require(cpp, "if (bIntentionalUILock)", "intentional UI lock branch")
require(cpp, "if (LastRecoveredCharacterPawn.Get() == Character)", "stable recovered character branch")
if cpp.count("ScheduleNextPoll(StablePollSeconds);") < 3:
    raise SystemExit("PASS41 VERIFY FAIL: stable 10 Hz polling is not used on enough steady-state paths")
if cpp.count("ScheduleNextPoll(TransitionPollSeconds);") < 2:
    raise SystemExit("PASS41 VERIFY FAIL: transition 20 Hz polling is not preserved for responsive recovery")

for needle in (
    "PASS 29-41 RUNTIME ACCEPTANCE",
    "PASS41_INPUT_RECOVERY_POLL_BUDGET_READY",
    "PASS31_GAMEPLAY_INPUT_READY",
    "30 FPS acceptance target",
    "PASS14_PERF_30FPS_READY",
):
    require(launcher, needle, "full runtime acceptance")

print("INPUT RECOVERY POLL BUDGET PASS 41 SOURCE CONTRACT PASS")
print("- vehicle/deployment input recovery uses one-shot adaptive polling instead of a permanent 20 Hz repeating timer")
print("- transition/UI-lock paths retain 20 Hz response while stable gameplay drops to 10 Hz")
print("- PASS31 gameplay-input evidence remains mandatory")
print("- full runtime acceptance still requires >=30 FPS")
print("STATUS: CODED_UNTESTED; local UE 5.8 vehicle/deployment/input/FPS runtime remains authoritative")