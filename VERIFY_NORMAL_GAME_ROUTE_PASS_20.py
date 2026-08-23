#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
START = ROOT / "START_HERE.cmd"
NORMAL = ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd"
PLAYABLE = ROOT / "RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd"


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

# START_HERE stays on the canonical normal-game helper rather than abusing an acceptance launcher.
require(start, 'call "%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"', "START_HERE normal-game route")
if 'call "%~dp0RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd"' in start:
    raise SystemExit("PASS20 VERIFY FAIL: START_HERE option 1 is incorrectly routed through the recovery acceptance launcher")

# Normal gameplay still keeps the real/playable weapon preflight and frontend route.
for needle in (
    "verify_required_weapon_assets.py",
    "required_weapon_asset_preflight_success.txt",
    "Opening every required REAL/playable weapon visual",
    "Launching CURRENT NORMAL GAME frontend",
    "-Frontend",
):
    require(normal, needle, "normal playable route")

# Production vehicle intake is still present, but only inside the strict acceptance stage.
strict_stage = normal.find("[3/4] STRICT ACCEPTANCE")
if strict_stage < 0:
    raise SystemExit("PASS20 VERIFY FAIL: strict production stage is missing")
acceptance_gate = normal.rfind('if "%IS_ACCEPTANCE%"=="1" (', 0, strict_stage)
if acceptance_gate < 0:
    raise SystemExit("PASS20 VERIFY FAIL: production intake is not guarded by IS_ACCEPTANCE")
import_call = normal.find('call "%PRODUCTION_IMPORT%"', strict_stage)
normal_else = normal.find(") else (", strict_stage)
if import_call < 0 or normal_else < 0 or not (acceptance_gate < strict_stage < import_call < normal_else):
    raise SystemExit("PASS20 VERIFY FAIL: production importer escaped the strict acceptance branch")

for needle in (
    "IMPORT_PRODUCTION_VEHICLES_UE58.cmd",
    "ukrainian_hmmwv_mk_19.glb",
    "m2_50cal_machinegun_cc0.glb",
    "BTR4_Bucephalus.fbx",
    "PASS7_PRODUCTION_VEHICLES_READY",
    "PASS7_PRODUCTION_WEAPONS_READY",
):
    require(normal, needle, "strict production route")

for needle in (
    "[3/4] NORMAL GAME: skipping strict production vehicle intake.",
    "Exact HMMWV/M2/BTR production source files remain an open content gap",
):
    require(normal, needle, "normal-game production bypass")

# The focused Pass 15-19 route remains available for explicit Museum/FPS diagnostics.
for needle in (
    "PASS19_PLAYABLE_WEAPON_SET_READY",
    "PASS15_MUSEUM_BASES_WEAPONS_READY",
    "PASS16_RUNTIME_GRAPHICS_IDENTITY",
    "PASS15_PERF_SAMPLE",
):
    require(playable, needle, "focused recovery route remains intact")

print("NORMAL GAME ROUTE PASS 20 SOURCE CONTRACT PASS")
print("- START_HERE option 1 stays on the canonical normal-game launcher")
print("- normal gameplay keeps real/playable weapon preflight and reaches the frontend without exact vehicle source intake")
print("- exact HMMWV/M2/BTR intake and Pass 7 vehicle/art evidence remain mandatory in strict acceptance mode")
print("- focused Pass 15-19 recovery remains available separately")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 runtime still required")
