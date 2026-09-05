#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
START = ROOT / "START_HERE.cmd"
BATCH = ROOT / "OsterConflict" / "Scripts" / "run_pass45_batch_runtime_test.ps1"
NORMAL = ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd"
EVIDENCE = ROOT / "VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS22 VERIFY FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        raise SystemExit(f"PASS22 VERIFY FAIL: {label}: missing {needle!r}")


def forbid(text: str, needle: str, label: str) -> None:
    if needle in text:
        raise SystemExit(f"PASS22 VERIFY FAIL: {label}: forbidden {needle!r}")


start = read(START)
batch = read(BATCH)
normal = read(NORMAL)
evidence = read(EVIDENCE)

for needle in (
    "1. ЗВИЧАЙНА ГРА", "2. ПОВНИЙ RUNTIME-ТЕСТ ^(ПАКЕТНИЙ^)",
    "3. SAFE СУМІСНІСТЬ", "4. ВІДКРИТИ UNREAL EDITOR",
    'set "CURRENT_GAMEPLAY=%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"',
    'set "BATCH_RUNTIME=%~dp0RUN_PASS45_BATCH_RUNTIME_TEST.cmd"',
    'call "%BATCH_RUNTIME%"', 'set "OC_RHI_COMPAT=1"', 'set "OC_RHI_COMPAT=0"',
    'call "%CURRENT_GAMEPLAY%"', '-d3d11', '-sm5', '-nohdr',
):
    require(start, needle, "START_HERE canonical route")

for needle in (
    'IMPORT_ALL_LOCAL_INBOX_UE58.cmd', 'RUN_PASS45_STRICT_MATERIAL_GATE.cmd',
    'RUN_R14_CURRENT_GAMEPLAY.cmd', 'VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py',
    'PASS45_BATCH_RUNTIME_REPORT.txt', '$env:OC_FORCE_ACCEPTANCE = "0"', '$env:OC_FORCE_ACCEPTANCE = "1"',
):
    require(batch, needle, "batch runtime canonical route")

for retired in (
    "RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd", "RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd",
    "RUN_R21_LANDMARK_OWNERSHIP_RUNTIME_ACCEPTANCE.cmd", "RUN_R14_MAIN_SANDBOX_TEST.cmd",
    "PASS45_REIMPORT_STEIN_WEAPON_MATERIALS_UE58.cmd", "PASS45_IMPORT_MANUAL_ACTION_AUDIO_UE58.cmd",
    "PASS45_IMPORT_REMINGTON870_PRODUCTION_UE58.cmd",
):
    forbid(start + "\n" + batch, retired, "retired/internal launcher leaked into canonical route")

for marker in (
    "PASS29_MAIN_START_DIRECT_HOST_QUEUED", "PASS29_STATIC_FRONTEND_HOST_TRAVEL_EXECUTE",
    "PASS45_LANDMARK_STARTUP_COORDINATED_READY", "PASS45_MUSEUM_LAYER_VALIDATION_READY",
    "PASS19_PLAYABLE_WEAPON_SET_READY", "PASS14_PERF_SAMPLE", "PASS14_PERF_30FPS_READY",
):
    require(evidence, marker, f"canonical runtime evidence {marker}")
for marker in ("PASS19_PLAYABLE_WEAPON_SET_FAIL", "PASS45_MUSEUM_LAYER_VALIDATION_FAIL", "PASS14_PERF_BELOW_TARGET"):
    require(evidence, marker, f"fail-closed runtime evidence {marker}")

for needle in (
    'set "RHI_FLAGS=-d3d11 -sm5 -nohdr"', 'if /I "%OC_RHI_COMPAT%"=="1"',
    'set "RHI_FLAGS=-d3d11 -sm5 -nohdr -norhithread"',
    'set "RHI_MODE=dx11_sm5_rhi_thread"', 'set "RHI_MODE=dx11_sm5_no_rhi_thread_compat"',
):
    require(normal, needle, "Pass45 normal/compatibility renderer contract")
for forbidden in ("-d3d12", "-dx12", "-sm6"):
    if forbidden in normal.lower():
        raise SystemExit(f"PASS22 VERIFY FAIL: normal gameplay re-enabled forbidden renderer flag {forbidden}")
require(normal, "[LOCAL CHANGE]", "normal game local-change visibility")

print("SINGLE LAUNCHER / D3D11 PASS22/PASS45 SOURCE CONTRACT PASS")
print("- START_HERE remains the only user-facing entry point")
print("- normal and safe routes launch the canonical gameplay directly without strict asset reimport")
print("- option 2 uses one internal packet runner and one consolidated report")
print("- normal gameplay uses DX11/SM5/no-HDR; -norhithread remains compatibility-only")
print("STATUS: SOURCE CONTRACT ONLY; local UE runtime must confirm stability and FPS")
