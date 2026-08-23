#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
START = ROOT / "START_HERE.cmd"
NORMAL = ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd"
PLAYFLOW = ROOT / "RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd"
RECOVERY = ROOT / "RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd"
LANDMARK = ROOT / "RUN_R21_LANDMARK_OWNERSHIP_RUNTIME_ACCEPTANCE.cmd"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS22 VERIFY FAIL: missing {path.name}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        raise SystemExit(f"PASS22 VERIFY FAIL: {label}: missing {needle!r}")


start = read(START)
normal = read(NORMAL)
playflow = read(PLAYFLOW)
recovery = read(RECOVERY)
landmark = read(LANDMARK)

# START_HERE is the only user-facing launcher. Option 2 is the current Pass 14/29
# playflow+performance acceptance wrapper. Pass 15 and Pass 21 remain internal diagnostic helpers.
for needle in (
    "1. ЗВИЧАЙНА ГРА",
    "2. ПОВНИЙ RUNTIME-ТЕСТ",
    "3. ВІДКРИТИ UNREAL EDITOR",
    'call "%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"',
    'call "%~dp0RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd"',
    "-d3d11",
):
    require(start, needle, "START_HERE canonical route")

if 'RUN_R21_LANDMARK_OWNERSHIP_RUNTIME_ACCEPTANCE.cmd' in start:
    raise SystemExit("PASS22 VERIFY FAIL: internal Pass 21 acceptance helper leaked into START_HERE")
if 'RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd' in start:
    raise SystemExit("PASS22 VERIFY FAIL: focused Pass 15 helper leaked into START_HERE")
if 'RUN_R14_MAIN_SANDBOX_TEST.cmd' in start:
    raise SystemExit("PASS22 VERIFY FAIL: technical sandbox leaked back into START_HERE user menu")

# The user-facing full runtime test wraps the canonical normal launcher and checks current
# Pass 29 playflow plus the 30 FPS acceptance evidence.
for needle in (
    'RUN_R14_CURRENT_GAMEPLAY.cmd',
    'PASS29_MAIN_START_DIRECT_HOST_QUEUED',
    'PASS29_STATIC_FRONTEND_HOST_TRAVEL_EXECUTE',
    'PASS14_PERF_SAMPLE',
    'PASS14_PERF_30FPS_READY',
):
    require(playflow, needle, "full runtime playflow wrapper")

# The actual normal and focused-acceptance game processes must use the same safe renderer.
for text, label in ((normal, "normal game"), (recovery, "focused runtime acceptance")):
    require(text, "-d3d11", f"{label} safe renderer")
    require(text, "D3D12", f"{label} crash rationale")
    if "-d3d12" in text.lower() or "-dx12" in text.lower():
        raise SystemExit(f"PASS22 VERIFY FAIL: {label} still forces D3D12")

# Pass 21 still exists as an internal acceptance wrapper and chains the focused recovery launcher.
require(landmark, "RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd", "Pass 21 chains focused recovery")
require(landmark, "PASS21_LANDMARK_OWNERSHIP_READY", "Pass 21 runtime evidence")

# Dirty local content should be visible rather than silently ignored during a test.
require(normal, "[LOCAL CHANGE]", "normal game local-change visibility")
require(recovery, "[LOCAL CHANGE]", "acceptance local-change visibility")

print("SINGLE LAUNCHER / D3D11 PASS 22 SOURCE CONTRACT PASS")
print("- START_HERE is the only user-facing entry point")
print("- option 2 uses the Pass 14/29 playflow+performance wrapper; Pass 15/21 remain internal")
print("- normal and diagnostic acceptance launches use DirectX 11 after the reproduced D3D12RHI startup crash")
print("- technical sandbox is no longer exposed in START_HERE")
print("STATUS: SOURCE CONTRACT ONLY; local UE runtime must confirm the renderer and FPS path")
