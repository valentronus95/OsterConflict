#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
START = ROOT / "START_HERE.cmd"
NORMAL = ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd"
EVIDENCE = ROOT / "VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS22 VERIFY FAIL: missing {path.name}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        raise SystemExit(f"PASS22 VERIFY FAIL: {label}: missing {needle!r}")


def forbid(text: str, needle: str, label: str) -> None:
    if needle in text:
        raise SystemExit(f"PASS22 VERIFY FAIL: {label}: forbidden {needle!r}")


start = read(START)
normal = read(NORMAL)
evidence = read(EVIDENCE)

# START_HERE is the only user-facing launcher. The full runtime test now uses one canonical gameplay
# launcher plus live asset/world proof, strict material gate and the canonical Python evidence verifier.
for needle in (
    "1. ЗВИЧАЙНА ГРА",
    "2. ПОВНИЙ RUNTIME-ТЕСТ",
    "3. SAFE СУМІСНІСТЬ",
    "4. ВІДКРИТИ UNREAL EDITOR",
    'set "CURRENT_GAMEPLAY=%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"',
    'set "MATERIAL_GATE=%~dp0OsterConflict\\RUN_PASS45_STRICT_MATERIAL_GATE.cmd"',
    'set "EVIDENCE_VERIFY=%~dp0VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py"',
    "call :ingest_all_assets",
    "call :full_runtime_test",
    'set "OC_RHI_COMPAT=1"',
    'set "OC_RHI_COMPAT=0"',
    'call "%CURRENT_GAMEPLAY%"',
    'call "%MATERIAL_GATE%"',
    '%PY_CMD% "%EVIDENCE_VERIFY%"',
    "-d3d11",
    "-sm5",
    "-nohdr",
):
    require(start, needle, "START_HERE canonical route")

for retired in (
    "RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd",
    "RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd",
    "RUN_R21_LANDMARK_OWNERSHIP_RUNTIME_ACCEPTANCE.cmd",
    "RUN_R14_MAIN_SANDBOX_TEST.cmd",
):
    forbid(start, retired, "retired/internal launcher leaked into START_HERE")

# Canonical evidence retains playflow, current landmark ownership, weapon readiness and >=30 FPS.
for marker in (
    "PASS29_MAIN_START_DIRECT_HOST_QUEUED",
    "PASS29_STATIC_FRONTEND_HOST_TRAVEL_EXECUTE",
    "PASS45_LANDMARK_STARTUP_COORDINATED_READY",
    "PASS45_MUSEUM_LAYER_VALIDATION_READY",
    "PASS19_PLAYABLE_WEAPON_SET_READY",
    "PASS14_PERF_SAMPLE",
    "PASS14_PERF_30FPS_READY",
):
    require(evidence, marker, f"canonical runtime evidence {marker}")
for marker in (
    "PASS19_PLAYABLE_WEAPON_SET_FAIL",
    "PASS45_MUSEUM_LAYER_VALIDATION_FAIL",
    "PASS14_PERF_BELOW_TARGET",
):
    require(evidence, marker, f"fail-closed runtime evidence {marker}")

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

# Dirty local content stays visible during a test rather than being silently hidden.
require(normal, "[LOCAL CHANGE]", "normal game local-change visibility")

print("SINGLE LAUNCHER / D3D11 PASS22/PASS45 SOURCE CONTRACT PASS")
print("- START_HERE remains the only user-facing entry point")
print("- option 3 remains the explicit no-RHI-thread compatibility A/B route; editor is option 4")
print("- normal gameplay uses DX11/SM5/no-HDR with normal RHI threading")
print("- full runtime acceptance uses current gameplay + live asset/world proof + material/evidence gates")
print("- retired per-pass acceptance launchers are not required")
print("STATUS: SOURCE CONTRACT ONLY; local UE runtime must confirm renderer stability and FPS")
