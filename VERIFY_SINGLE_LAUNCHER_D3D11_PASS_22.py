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

# START_HERE remains the only user-facing launcher. Pass 45 inserts one explicit renderer-compatibility
# A/B route, so the editor moves from menu option 3 to 4. Pass 15 and Pass 21 stay internal helpers.
for needle in (
    "1. ЗВИЧАЙНА ГРА",
    "2. ПОВНИЙ RUNTIME-ТЕСТ",
    "3. SAFE СУМІСНІСТЬ",
    "4. ВІДКРИТИ UNREAL EDITOR",
    'set "OC_RHI_COMPAT=1"',
    'set "OC_RHI_COMPAT=0"',
    'call "%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"',
    'call "%~dp0RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd"',
    "-d3d11",
    "-sm5",
    "-nohdr",
):
    require(start, needle, "START_HERE canonical route")

if 'RUN_R21_LANDMARK_OWNERSHIP_RUNTIME_ACCEPTANCE.cmd' in start:
    raise SystemExit("PASS22 VERIFY FAIL: internal Pass 21 acceptance helper leaked into START_HERE")
if 'RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd' in start:
    raise SystemExit("PASS22 VERIFY FAIL: focused Pass 15 helper leaked into START_HERE")
if 'RUN_R14_MAIN_SANDBOX_TEST.cmd' in start:
    raise SystemExit("PASS22 VERIFY FAIL: technical sandbox leaked back into START_HERE user menu")

# The user-facing full runtime test wraps the canonical normal launcher and checks current playflow/performance evidence.
for needle in (
    'RUN_R14_CURRENT_GAMEPLAY.cmd',
    'PASS29_MAIN_START_DIRECT_HOST_QUEUED',
    'PASS29_STATIC_FRONTEND_HOST_TRAVEL_EXECUTE',
    'PASS14_PERF_SAMPLE',
    'PASS14_PERF_30FPS_READY',
):
    require(playflow, needle, "full runtime playflow wrapper")

# Pass 45 keeps DirectX 11 + SM5 + HDR off. Normal gameplay no longer hard-disables the RHI thread;
# -norhithread is preserved only behind the explicit compatibility selector after the factual 8-FPS menu run.
for needle in (
    'set "RHI_FLAGS=-d3d11 -sm5 -nohdr"',
    'if /I "%OC_RHI_COMPAT%"=="1"',
    'set "RHI_FLAGS=-d3d11 -sm5 -nohdr -norhithread"',
    'set "RHI_MODE=dx11_sm5_rhi_thread"',
    'set "RHI_MODE=dx11_sm5_no_rhi_thread_compat"',
):
    require(normal, needle, "Pass 45 normal/compatibility renderer contract")
if "-d3d12" in normal.lower() or "-dx12" in normal.lower() or "-sm6" in normal.lower():
    raise SystemExit("PASS22 VERIFY FAIL: normal gameplay re-enabled D3D12/SM6")

# Focused historical recovery remains an internal DX11 diagnostic helper; it may retain stronger compatibility flags.
for needle in ("-d3d11", "D3D12"):
    require(recovery, needle, "focused runtime acceptance renderer history")
if "-d3d12" in recovery.lower() or "-dx12" in recovery.lower():
    raise SystemExit("PASS22 VERIFY FAIL: focused runtime acceptance still forces D3D12")

# Pass 21 still exists as an internal acceptance wrapper and chains the focused recovery launcher.
require(landmark, "RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd", "Pass 21 chains focused recovery")
require(landmark, "PASS21_LANDMARK_OWNERSHIP_READY", "Pass 21 runtime evidence")

# Dirty local content should be visible rather than silently ignored during a test.
require(normal, "[LOCAL CHANGE]", "normal game local-change visibility")
require(recovery, "[LOCAL CHANGE]", "acceptance local-change visibility")

print("SINGLE LAUNCHER / D3D11 PASS 22/45 SOURCE CONTRACT PASS")
print("- START_HERE is still the only user-facing entry point")
print("- option 3 is the explicit no-RHI-thread compatibility A/B route; editor is option 4")
print("- normal gameplay uses DX11/SM5/no-HDR with normal RHI threading")
print("- D3D12/SM6 remain disabled while startup/runtime recovery is unresolved")
print("- technical Pass 15/21/sandbox launchers remain internal")
print("STATUS: SOURCE CONTRACT ONLY; local UE runtime must confirm renderer stability and FPS")
