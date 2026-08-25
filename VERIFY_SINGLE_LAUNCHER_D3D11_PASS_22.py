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

# START_HERE remains the only user-facing launcher. Pass45 keeps one explicit renderer-compatibility
# A/B route, so the editor is option 4. Pass15 and Pass21 remain internal helpers.
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

for internal in (
    'RUN_R21_LANDMARK_OWNERSHIP_RUNTIME_ACCEPTANCE.cmd',
    'RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd',
    'RUN_R14_MAIN_SANDBOX_TEST.cmd',
):
    if internal in start:
        raise SystemExit(f"PASS22 VERIFY FAIL: internal/technical launcher leaked into START_HERE: {internal}")

# The user-facing full runtime test wraps the canonical normal launcher and checks current playflow/performance evidence.
for needle in (
    'RUN_R14_CURRENT_GAMEPLAY.cmd',
    'PASS29_MAIN_START_DIRECT_HOST_QUEUED',
    'PASS29_STATIC_FRONTEND_HOST_TRAVEL_EXECUTE',
    'PASS14_PERF_SAMPLE',
    'PASS14_PERF_30FPS_READY',
):
    require(playflow, needle, "full runtime playflow wrapper")

# Pass45 renderer contract: DX11 + SM5 + HDR off. Normal gameplay keeps normal RHI threading;
# -norhithread exists only behind the explicit compatibility selector.
for needle in (
    'set "RHI_FLAGS=-d3d11 -sm5 -nohdr"',
    'if /I "%OC_RHI_COMPAT%"=="1"',
    'set "RHI_FLAGS=-d3d11 -sm5 -nohdr -norhithread"',
    'set "RHI_MODE=dx11_sm5_rhi_thread"',
    'set "RHI_MODE=dx11_sm5_no_rhi_thread_compat"',
):
    require(normal, needle, "Pass45 normal/compatibility renderer contract")
for forbidden in ("-d3d12", "-dx12", "-sm6"):
    if forbidden in normal.lower():
        raise SystemExit(f"PASS22 VERIFY FAIL: normal gameplay re-enabled forbidden renderer flag {forbidden}")

# Focused recovery is an internal DX11 compatibility diagnostic. The verifier checks behavior/flags,
# not whether the launcher preserves historical prose mentioning a rejected renderer.
for needle in ("-d3d11", "-sm5", "-nohdr"):
    require(recovery, needle, "focused runtime acceptance renderer contract")
for forbidden in ("-d3d12", "-dx12", "-sm6"):
    if forbidden in recovery.lower():
        raise SystemExit(f"PASS22 VERIFY FAIL: focused recovery forces forbidden renderer flag {forbidden}")

# Pass21 remains internal and now validates current Pass45 single-owner evidence instead of old duplicate repair markers.
require(landmark, "RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd", "Pass21 chains focused recovery")
for marker in (
    "PASS45_LANDMARK_STARTUP_COORDINATED_READY",
    "PASS45_MUSEUM_LAYER_VALIDATION_READY",
    "PASS45_LANDMARK_SEPARATION_VALIDATION_READY",
):
    require(landmark, marker, f"Pass21 current runtime evidence {marker}")
for stale in (
    "PASS21_LANDMARK_DUPLICATE_REPAIRED",
    "PASS21_LANDMARK_OWNERSHIP_READY",
    "PASS21_LANDMARK_OWNERSHIP_FAIL",
):
    if stale in landmark:
        raise SystemExit(f"PASS22 VERIFY FAIL: retired Pass21 repair marker returned: {stale}")

# Dirty local content should be visible rather than silently ignored during a test.
require(normal, "[LOCAL CHANGE]", "normal game local-change visibility")
require(recovery, "[LOCAL CHANGE]", "acceptance local-change visibility")

print("SINGLE LAUNCHER / D3D11 PASS22/PASS45 SOURCE CONTRACT PASS")
print("- START_HERE is still the only user-facing entry point")
print("- option 3 is the explicit no-RHI-thread compatibility A/B route; editor is option 4")
print("- normal gameplay uses DX11/SM5/no-HDR with normal RHI threading")
print("- focused recovery is checked by current flags, not obsolete renderer-history prose")
print("- internal Pass21 follows validation-only landmark ownership")
print("STATUS: SOURCE CONTRACT ONLY; local UE runtime must confirm renderer stability and FPS")
