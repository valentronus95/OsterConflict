#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
START = ROOT / "START_HERE.cmd"
PLAYABLE = ROOT / "RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd"
STRICT = ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS20 VERIFY FAIL: missing {path.name}")
    return path.read_text(encoding="utf-8", errors="replace")

start = read(START)
playable = read(PLAYABLE)
strict = read(STRICT)

if 'call "%~dp0RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd"' not in start:
    raise SystemExit("PASS20 VERIFY FAIL: START_HERE option 1 is not routed through playable recovery")
if 'call "%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"' in start:
    raise SystemExit("PASS20 VERIFY FAIL: START_HERE still routes normal game through strict production intake")
for needle in (
    "PASS19_PLAYABLE_WEAPON_SET_READY",
    "PASS15_MUSEUM_BASES_WEAPONS_READY",
    "PASS16_RUNTIME_GRAPHICS_IDENTITY",
    "PASS15_PERF_SAMPLE",
    "-Frontend",
):
    if needle not in playable:
        raise SystemExit(f"PASS20 VERIFY FAIL: playable route missing {needle!r}")

# Strict production route remains available and still requires the real vehicle source intake.
for needle in (
    "IMPORT_PRODUCTION_VEHICLES_UE58.cmd",
    "ukrainian_hmmwv_mk_19.glb",
    "m2_50cal_machinegun_cc0.glb",
    "BTR4_Bucephalus.fbx",
    "PASS7_PRODUCTION_VEHICLES_READY",
):
    if needle not in strict:
        raise SystemExit(f"PASS20 VERIFY FAIL: strict production route weakened: missing {needle!r}")

print("NORMAL GAME ROUTE PASS 20 SOURCE CONTRACT PASS")
print("- START_HERE option 1 enters the playable frontend/server/Museum/FPS route")
print("- missing exact HMMWV/M2/BTR source files no longer block reaching the normal game frontend")
print("- strict production-art/vehicle acceptance remains separate and unchanged")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 runtime still required")
