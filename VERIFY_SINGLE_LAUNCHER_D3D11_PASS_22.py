#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
START = ROOT / "START_HERE.cmd"
NORMAL = ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd"
BATCH_CMD = ROOT / "OsterConflict" / "PASS45_BATCH_RUNTIME.cmd"
BATCH_ENTRY = ROOT / "OsterConflict" / "Scripts" / "pass45_batch_runtime_progress_entry.py"
BATCH_RUNTIMEFIX = ROOT / "OsterConflict" / "Scripts" / "pass45_batch_runtime_runtimefix.py"
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
batch_entry = read(BATCH_ENTRY)
batch_runtimefix = read(BATCH_RUNTIMEFIX)
batch_py = read(BATCH_PY)
recovery = read(RECOVERY)

for needle in (
    "1. ЗВИЧАЙНА ГРА", "2. ПОВНИЙ RUNTIME-ТЕСТ", "3. SAFE СУМІСНІСТЬ", "4. ВІДКРИТИ UNREAL EDITOR",
    'call "%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"', 'call "%~dp0OsterConflict\\PASS45_BATCH_RUNTIME.cmd"',
    'set "OC_RHI_COMPAT=1"', 'set "OC_RHI_COMPAT=0"',
    'call "%~dp0BUILD_EDITOR_LAUNCHER_UE58.cmd" /nopause',
    '/Game/Maps/OsterConflict_Runtime -NoFrontend -d3d11 -sm5 -nohdr',
):
    require(start, needle, "START_HERE canonical route")

for internal in ('RUN_R21_LANDMARK_OWNERSHIP_RUNTIME_ACCEPTANCE.cmd', 'RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd', 'RUN_R14_MAIN_SANDBOX_TEST.cmd'):
    if internal in start:
        raise SystemExit(f"PASS22 VERIFY FAIL: internal launcher leaked into START_HERE: {internal}")

# Pass45 now has a progress/recovery wrapper in front of the canonical one-runtime orchestrator.
# Validate the whole chain instead of requiring the .cmd file to invoke the old base script directly.
require(batch_cmd, "pass45_batch_runtime_progress_entry.py", "batch wrapper entry")
for needle in ("import pass45_batch_runtime_progress as progress", 'with_name("pass45_batch_runtime_runtimefix.py")'):
    require(batch_entry, needle, "batch progress entry")
for needle in (
    "import pass45_batch_runtime as base",
    "subprocess.run = _patched_subprocess_run",
    'rewritten.append("-windowed")',
    'rewritten.append("-ResX=1280")',
    'rewritten.append("-ResY=720")',
    'rewritten.append("-norhithread")',
):
    require(batch_runtimefix, needle, "batch responsive-window runtime shim")

for needle in (
    "subprocess.run(runtime_cmd", "/Game/Maps/OsterConflict_Runtime", '"-game", "-Frontend"',
    '"-d3d11", "-sm5", "-nohdr"', "PASS45_BATCH_RUNTIME_REPORT.txt",
    "C++ чистий. Проганяю всі content/asset diagnostics", "Проганяю ВСІ post-runtime verifier-и",
):
    require(batch_py, needle, "batch one-runtime contract")
if batch_py.count("subprocess.run(runtime_cmd") != 1:
    raise SystemExit("PASS22 VERIFY FAIL: batch orchestrator must own exactly one direct gameplay run")
for forbidden in ("-d3d12", "-dx12", '"-sm6"'):
    if forbidden in batch_py.lower():
        raise SystemExit(f"PASS22 VERIFY FAIL: batch runtime re-enabled forbidden renderer flag {forbidden}")
for destructive in ("git reset", "git clean", "git stash", "checkout --", "restore --"):
    if destructive in (batch_py + batch_runtimefix + batch_entry).lower():
        raise SystemExit(f"PASS22 VERIFY FAIL: batch runtime mutates local Changes: {destructive}")

for needle in (
    'set "RHI_FLAGS=-d3d11 -sm5 -nohdr -nosplash"', 'if /I "%OC_RHI_COMPAT%"=="1"',
    'set "RHI_FLAGS=-d3d11 -sm5 -nohdr -norhithread -nosplash"', 'set "RHI_MODE=dx11_sm5_rhi_thread"',
    'set "RHI_MODE=dx11_sm5_no_rhi_thread_compat"', "start /wait", "[LOCAL CHANGE]",
    'set "QUALITY_CMDS=', 'r.ScreenPercentage 100', '-windowed -ForceRes -ResX=1280 -ResY=720',
):
    require(normal, needle, "normal/compat renderer contract")
for forbidden in ("-d3d12", "-dx12", "-sm6"):
    if forbidden in normal.lower():
        raise SystemExit(f"PASS22 VERIFY FAIL: normal gameplay re-enabled {forbidden}")

for needle in ("-d3d11", "-sm5", "-nohdr", "[LOCAL CHANGE]"):
    require(recovery, needle, "focused recovery compatibility contract")

print("SINGLE LAUNCHER / D3D11 PASS22 + PASS45 BATCH SOURCE CONTRACT PASS")
print("- START_HERE is the only user-facing entry point")
print("- editor option builds current code, opens OsterConflict_Runtime directly, and uses -NoFrontend for gameplay inspection")
print("- batch wrapper chain keeps one canonical gameplay runtime while adding progress/recovery and a responsive test window")
print("- DX11/SM5/no-HDR remains canonical; option 3 owns explicit -norhithread compatibility")
print("- normal route suppresses the separate splash and restores 100% normal visual quality")
print("- local Changes are reported and preserved, never reset/stashed/cleaned")
print("STATUS: SOURCE CONTRACT ONLY; local UE runtime must confirm actual content")
