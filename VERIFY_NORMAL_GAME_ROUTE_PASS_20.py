#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
START = ROOT / "START_HERE.cmd"
NORMAL = ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd"
BATCH_CMD = ROOT / "OsterConflict" / "PASS45_BATCH_RUNTIME.cmd"
BATCH_PY = ROOT / "OsterConflict" / "Scripts" / "pass45_batch_runtime.py"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS20 VERIFY FAIL: missing {path}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        raise SystemExit(f"PASS20 VERIFY FAIL: {label}: missing {needle!r}")


start = read(START)
normal = read(NORMAL)
batch_cmd = read(BATCH_CMD)
batch_py = read(BATCH_PY)

for needle in (
    "1. ЗВИЧАЙНА ГРА", "2. ПОВНИЙ RUNTIME-ТЕСТ", "3. SAFE СУМІСНІСТЬ",
    'call "%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"',
    'call "%~dp0OsterConflict\\PASS45_BATCH_RUNTIME.cmd"',
    'set "OC_QUICK_NORMAL=1"', 'set "OC_RHI_COMPAT=1"', 'set "OC_RHI_COMPAT=0"',
):
    require(start, needle, "START_HERE route")

for stale in (":prepare_materials_strict", 'call "%~dp0RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd"', "TRY_PRODUCTION_VEHICLES_UE58.cmd", "TRY_PASS45_STEIN_WEAPON_MATERIALS_UE58.cmd"):
    if stale in start:
        raise SystemExit(f"PASS20 VERIFY FAIL: stale fail-fast/heavy route returned to START_HERE: {stale}")

for needle in (
    'if /I "%OC_QUICK_NORMAL%"=="1" goto quick_normal_game', ':quick_normal_game',
    '[QUICK NORMAL] Incremental C++ build only. Asset reimport is skipped.', 'Runtime acceptance: NOT RUN',
    '-windowed -ResX=1280 -ResY=720', '-ExecCmds="t.MaxFPS 60"',
):
    require(normal, needle, "quick normal route")

quick = normal.split(':quick_normal_game', 1)[1]
for forbidden in ('git lfs pull', 'verify_required_weapon_assets.py', 'call "%PRODUCTION_IMPORT%"', 'PASS7_PRODUCTION_VEHICLES_READY'):
    if forbidden in quick:
        raise SystemExit(f"PASS20 VERIFY FAIL: quick route regained heavy work: {forbidden}")

require(batch_cmd, "pass45_batch_runtime.py", "batch command wrapper")
for needle in (
    "IMPORT_ALL_LOCAL_INBOX_UE58.cmd", "PASS45_REIMPORT_STEIN_WEAPON_MATERIALS_UE58.cmd",
    "PASS45_IMPORT_MANUAL_ACTION_AUDIO_UE58.cmd", "PASS45_IMPORT_REMINGTON870_PRODUCTION_UE58.cmd",
    "IMPORT_PRODUCTION_VEHICLES_UE58.cmd", "verify_required_weapon_assets.py", "RUN_PASS45_STRICT_MATERIAL_GATE.cmd",
    "VERIFY_PASS45_GATE_K_RUNTIME_LOG.py", "VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py", "VERIFY_PASS45_MANUAL_ACTION_RUNTIME.py",
    "VERIFY_PASS45_GRENADE_THROW_ANIMATION_RUNTIME.py", "VERIFY_PASS45_GRENADE_FLASH_RUNTIME.py",
    "/Game/Maps/OsterConflict_Runtime", "PASS45_REQUIRED_AVAILABLE_WEAPONS_READY",
    "PASS45_PRODUCTION_VEHICLE_VISUALS_VALIDATED_READY", "PASS14_PERF_30FPS_READY", "PASS45_BATCH_RUNTIME_REPORT.txt",
):
    require(batch_py, needle, "batch runtime orchestrator")

for destructive in ("git reset", "git clean", "git stash", "checkout --", "restore --"):
    if destructive in batch_py.lower():
        raise SystemExit(f"PASS20 VERIFY FAIL: batch runtime can mutate user Changes: {destructive}")

print("NORMAL GAME ROUTE PASS20 + PASS45 BATCH-FIRST SOURCE CONTRACT PASS")
print("- START_HERE remains the only user-facing launcher")
print("- option 1/3 remain lightweight normal/compat routes")
print("- option 2 batches local/Fab assets, weapons, audio, Remington, vehicles, material gates and one runtime")
print("- tracked local Changes are preserved; failures are consolidated into one report")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 runtime evidence remains factual")
