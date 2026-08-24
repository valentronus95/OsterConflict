#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
START = ROOT / "START_HERE.cmd"
NORMAL = ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd"
PLAYABLE = ROOT / "RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd"
IMPORTER = ROOT / "OsterConflict" / "IMPORT_PRODUCTION_VEHICLES_UE58.cmd"
SOURCE_RECOVERY = ROOT / "OsterConflict" / "Scripts" / "prepare_local_production_sources.ps1"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS20 VERIFY FAIL: missing {path.name}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        raise SystemExit(f"PASS20 VERIFY FAIL: {label}: missing {needle!r}")


start = read(START)
normal = read(NORMAL)
playable = read(PLAYABLE)
importer = read(IMPORTER)
source_recovery = read(SOURCE_RECOVERY)

require(start, 'call "%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"', "START_HERE normal-game route")
if 'call "%~dp0RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd"' in start:
    raise SystemExit("PASS20 VERIFY FAIL: START_HERE option 1 is incorrectly routed through recovery acceptance")

for needle in (
    "verify_required_weapon_assets.py",
    "required_weapon_asset_preflight_success.txt",
    "Opening every required REAL/playable weapon visual",
    "Launching CURRENT NORMAL GAME frontend",
    "-Frontend",
    '/C:"fix/runtime-map-spawn-fps-assets-"',
):
    require(normal, needle, "normal playable route")

# Strict acceptance still owns fail-closed exact fleet validation.
strict_stage = normal.find("[3/4] STRICT ACCEPTANCE")
if strict_stage < 0:
    raise SystemExit("PASS20 VERIFY FAIL: strict production stage is missing")
acceptance_gate = normal.rfind('if "%IS_ACCEPTANCE%"=="1" (', 0, strict_stage)
import_call = normal.find('call "%PRODUCTION_IMPORT%"', strict_stage)
normal_else = normal.find(") else (", strict_stage)
if acceptance_gate < 0 or import_call < 0 or normal_else < 0 or not (acceptance_gate < strict_stage < import_call < normal_else):
    raise SystemExit("PASS20 VERIFY FAIL: production importer escaped strict acceptance")

for needle in (
    "IMPORT_PRODUCTION_VEHICLES_UE58.cmd",
    "PASS7_PRODUCTION_VEHICLES_READY",
    "PASS7_PRODUCTION_WEAPONS_READY",
):
    require(normal, needle, "strict production runtime route")

# Pass 44 intentionally moves literal local source filenames back to the source/intake owner instead of duplicating
# those details in the gameplay launcher. Missing BTR must not block available HMMWV/M2.
for needle in (
    "ukrainian_hmmwv_mk_19.glb",
    "m2_50cal_machinegun_cc0.glb",
    "BTR4_Bucephalus.fbx",
    'set "HMMWV_IMPORTED=0"',
    'set "M2_IMPORTED=0"',
    'set "BTR_IMPORTED=0"',
):
    require(importer, needle, "independent production intake owner")
require(source_recovery, "Find-BtrFbxInNamedArchive", "BTR archive recovery")
require(source_recovery, "Available models may still be imported independently", "partial-source truth")

for needle in (
    "[3/4] NORMAL GAME: optional production model intake is handled by START_HERE before this launcher.",
    "Missing exact production models remain visible content gaps; no proxy is called production-ready.",
):
    require(normal, needle, "normal-game content truth")

for needle in (
    "PASS19_PLAYABLE_WEAPON_SET_READY",
    "PASS15_MUSEUM_BASES_WEAPONS_READY",
    "PASS16_RUNTIME_GRAPHICS_IDENTITY",
    "PASS15_PERF_SAMPLE",
):
    require(playable, needle, "focused recovery route remains intact")

print("NORMAL GAME ROUTE PASS 20 + PASS 44 SOURCE CONTRACT PASS")
print("- START_HERE option 1 stays on the canonical normal-game launcher")
print("- normal gameplay keeps the real/playable weapon preflight and branch-aware pre-merge test route")
print("- local source filenames belong to the independent production importer/source-recovery scripts")
print("- missing BTR cannot block available HMMWV/M2, but strict acceptance still rejects incomplete exact fleet art")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 runtime still required")
