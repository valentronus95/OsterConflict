#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
START = ROOT / "START_HERE.cmd"
NORMAL = ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd"
BATCH_CMD = ROOT / "OsterConflict" / "PASS45_BATCH_RUNTIME.cmd"
BATCH_PY = ROOT / "OsterConflict" / "Scripts" / "pass45_batch_runtime.py"
RECOVERY = ROOT / "RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS22 VERIFY FAIL: missing {path}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        raise SystemExit(f"PASS22 VERIFY FAIL: {label}: missing {needle!r}")


start = read(START)
normal = read(NORMAL)
batch_cmd = read(BATCH_CMD)
batch_py = read(BATCH_PY)
recovery = read(RECOVERY)

for needle in (
    "1. ЗВИЧАЙНА ГРА", "2. ПОВНИЙ RUNTIME-ТЕСТ", "3. SAFE СУМІСНІСТЬ", "4. ВІДКРИТИ UNREAL EDITOR",
    'call "%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"', 'call "%~dp0OsterConflict\\PASS45_BATCH_RUNTIME.cmd"',
    'set "OC_RHI_COMPAT=1"', 'set "OC_RHI_COMPAT=0"',
):
    require(start, needle, "START_HERE canonical route")

for internal in ('RUN_R21_LANDMARK_OWNERSHIP_RUNTIME_ACCEPTANCE.cmd', 'RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd', 'RUN_R14_MAIN_SANDBOX_TEST.cmd'):
    if internal in start:
        raise SystemExit(f"PASS22 VERIFY FAIL: internal launcher leaked into START_HERE: {internal}")

require(batch_cmd, "pass45_batch_runtime.py", "batch wrapper")
for needle in (
    "subprocess.run(runtime_cmd", "/Game/Maps/OsterConflict_Runtime", '"-game", "-Frontend"',
    '"-d3d11", "-sm5", "-nohdr"', "PASS45_BATCH_RUNTIME_REPORT.txt",
    "Проганяю ВСІ незалежні етапи", "Проганяю ВСІ post-runtime verifier-и",
):
    require(batch_py, needle, "batch one-runtime contract")
if batch_py.count("subprocess.run(runtime_cmd") != 1:
    raise SystemExit("PASS22 VERIFY FAIL: batch orchestrator must own exactly one direct gameplay run")
for forbidden in ("-d3d12", "-dx12", '"-sm6"'):
    if forbidden in batch_py.lower():
        raise SystemExit(f"PASS22 VERIFY FAIL: batch runtime re-enabled forbidden renderer flag {forbidden}")
for destructive in ("git reset", "git clean", "git stash", "checkout --", "restore --"):
    if destructive in batch_py.lower():
        raise SystemExit(f"PASS22 VERIFY FAIL: batch runtime mutates local Changes: {destructive}")

for needle in (
    'set "RHI_FLAGS=-d3d11 -sm5 -nohdr -nosplash"', 'if /I "%OC_RHI_COMPAT%"=="1"',
    'set "RHI_FLAGS=-d3d11 -sm5 -nohdr -norhithread -nosplash"', 'set "RHI_MODE=dx11_sm5_rhi_thread"',
    'set "RHI_MODE=dx11_sm5_no_rhi_thread_compat"', "start /wait", "[LOCAL CHANGE]",
    'set "QUALITY_CMDS=', 'r.ScreenPercentage 100',
):
    require(normal, needle, "normal/compat renderer contract")
for forbidden in ("-d3d12", "-dx12", "-sm6"):
    if forbidden in normal.lower():
        raise SystemExit(f"PASS22 VERIFY FAIL: normal gameplay re-enabled {forbidden}")

for needle in ("-d3d11", "-sm5", "-nohdr", "[LOCAL CHANGE]"):
    require(recovery, needle, "focused recovery compatibility contract")

print("SINGLE LAUNCHER / D3D11 PASS22 + PASS45 BATCH SOURCE CONTRACT PASS")
print("- START_HERE is the only user-facing entry point")
print("- DX11/SM5/no-HDR remains canonical; option 3 owns explicit -norhithread compatibility")
print("- normal route suppresses the separate splash and restores 100% normal visual quality")
print("- local Changes are reported and preserved, never reset/stashed/cleaned")
print("STATUS: SOURCE CONTRACT ONLY; local UE runtime must confirm actual content")
