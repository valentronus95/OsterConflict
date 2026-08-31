#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
START = ROOT / "START_HERE.cmd"
NORMAL = ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd"
PLAYABLE = ROOT / "RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd"
IMPORTER = ROOT / "OsterConflict" / "IMPORT_PRODUCTION_VEHICLES_UE58.cmd"
IMPORT_PY = ROOT / "OsterConflict" / "Scripts" / "import_production_vehicle_assets.py"
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
import_py = read(IMPORT_PY)
source_recovery = read(SOURCE_RECOVERY)

# User-facing option 1 must keep one canonical gameplay owner, but explicitly select its
# lightweight path. Heavy asset/import/evidence work belongs to option 2 only.
require(start, 'call "%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"', "START_HERE canonical normal-game route")
require(start, 'call "%~dp0RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd"', "START_HERE full runtime route")
require(start, 'set "OC_QUICK_NORMAL=1"', "START_HERE lightweight normal-game selector")
require(start, 'set "OC_RHI_COMPAT=1"', "START_HERE compatibility selector")
require(start, 'set "OC_RHI_COMPAT=0"', "START_HERE normal renderer selector")
if 'call "%~dp0RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd"' in start:
    raise SystemExit("PASS20 VERIFY FAIL: START_HERE option 1 is incorrectly routed through recovery acceptance")
for forbidden in (
    ":prepare_materials_optional",
    "TRY_PRODUCTION_VEHICLES_UE58.cmd",
    "TRY_PASS45_STEIN_WEAPON_MATERIALS_UE58.cmd",
    "RUN_PASS45_FAST_PREVIEW.cmd",
):
    if forbidden in start:
        raise SystemExit(f"PASS20 VERIFY FAIL: heavy/experimental preflight leaked back into START_HERE normal route: {forbidden}")

for needle in (
    'if /I "%OC_QUICK_NORMAL%"=="1" goto quick_normal_game',
    ':quick_normal_game',
    '[QUICK NORMAL] Incremental C++ build only. Asset reimport is skipped.',
    'LFS hydration, weapon commandlet preflight, production vehicle import and acceptance gates are skipped.',
    'Runtime acceptance: NOT RUN',
    '-windowed -ResX=1280 -ResY=720',
    '-ExecCmds="t.MaxFPS 60"',
):
    require(normal, needle, "canonical lightweight normal-game route")

quick_guard = normal.find('if /I "%OC_QUICK_NORMAL%"=="1" goto quick_normal_game')
heavy_lfs = normal.find('git lfs pull origin')
heavy_weapon = normal.find('Opening every required REAL/playable weapon visual')
heavy_import = normal.find('call "%PRODUCTION_IMPORT%"')
if min(quick_guard, heavy_lfs, heavy_weapon, heavy_import) < 0 or not (quick_guard < heavy_lfs and quick_guard < heavy_weapon and quick_guard < heavy_import):
    raise SystemExit("PASS20 VERIFY FAIL: quick-normal guard does not bypass all heavy preflight/import stages")

quick = normal.split(':quick_normal_game', 1)[1]
for forbidden in (
    'git lfs pull',
    'verify_required_weapon_assets.py',
    '"%EDITOR_CMD%"',
    'call "%PRODUCTION_IMPORT%"',
    'PASS7_PRODUCTION_VEHICLES_READY',
):
    if forbidden in quick:
        raise SystemExit(f"PASS20 VERIFY FAIL: quick-normal section regained heavy/acceptance work: {forbidden}")

# Strict/canonical acceptance behavior remains intact for option 2.
for needle in (
    "verify_required_weapon_assets.py",
    "required_weapon_asset_preflight_success.txt",
    "Opening every required REAL/playable weapon visual",
    "Launching CURRENT NORMAL GAME frontend",
    "-Frontend",
    '/C:"fix/runtime-map-spawn-fps-assets-"',
):
    require(normal, needle, "strict canonical gameplay route")

strict_stage = normal.find("[3/4] STRICT ACCEPTANCE")
if strict_stage < 0:
    raise SystemExit("PASS20 VERIFY FAIL: strict production stage is missing")
acceptance_gate = normal.rfind('if "%IS_ACCEPTANCE%"=="1" (', 0, strict_stage)
import_call = normal.find('call "%PRODUCTION_IMPORT%"', strict_stage)
normal_else = normal.find(") else (", strict_stage)
if acceptance_gate < 0 or import_call < 0 or normal_else < 0 or not (acceptance_gate < strict_stage < import_call < normal_else):
    raise SystemExit("PASS20 VERIFY FAIL: production importer escaped strict acceptance inside RUN_R14_CURRENT_GAMEPLAY")

for needle in (
    "IMPORT_PRODUCTION_VEHICLES_UE58.cmd",
    "PASS7_PRODUCTION_VEHICLES_READY",
    "PASS45_REQUIRED_AVAILABLE_WEAPONS_READY",
    "PASS36_WEAPON_MATERIAL_AUDIT_READY",
    "PASS45_REQUIRED_AVAILABLE_WEAPON_RUNTIME_FAIL",
    "PASS44_WEAPON_RACK_AUTHORED_MATERIAL_GAP",
):
    require(normal, needle, "strict runtime route")
if "PASS7_PRODUCTION_WEAPONS_READY" in normal or "PASS7_PRODUCTION_WEAPON_RUNTIME_FAIL" in normal:
    raise SystemExit("PASS20 VERIFY FAIL: obsolete all-exact rack acceptance returned")

for needle in (
    'set "HMMWV_IMPORTED=0"',
    'set "M2_IMPORTED=0"',
    'set "BTR_IMPORTED=0"',
    'set "BTR_AXIS_READY=0"',
    'set "BTR_GLTF_UP_READY=0"',
    'set "BTR_INTERNAL_UP_READY=0"',
    "Continuing independent intake; canonical BTR authored fallback remains available.",
    'set "REQUIRED_REVISION=PASS45_BTR_GLTF_Y_UP_20260827_R3"',
    'BTR4_FORWARD_AXIS=+X',
    'BTR4_GLTF_UP_AXIS=+Y',
    'BTR4_INTERNAL_UP_AXIS=+Z',
):
    require(importer, needle, "independent production intake command")
if "PASS45_BTR_AXIS_OPTIC_20260827_R2" in importer:
    raise SystemExit("PASS20 VERIFY FAIL: strict production intake still permits stale BTR R2")

for needle in (
    "ukrainian_hmmwv_mk_19.glb",
    "m2_50cal_machinegun_cc0.glb",
    "BTR4_Bucephalus.fbx",
    'attempt("HMMWV"',
    'attempt("M2"',
    "def import_btr4(",
    "BTR_GENERATED_SOURCE",
    "build_btr4_glb(BTR_GENERATED_SOURCE)",
    "authored_external_visual",
    "M_BTR4_OC_Authored",
    'IMPORT_CONTRACT_REVISION = "PASS45_BTR_GLTF_Y_UP_20260827_R3"',
    "BTR4_FORWARD_AXIS=+X",
    "BTR4_GLTF_UP_AXIS=+Y",
    "BTR4_INTERNAL_UP_AXIS=+Z",
):
    require(import_py, needle, "current production asset implementation")
if 'attempt("BTR4"' in import_py:
    raise SystemExit("PASS20 VERIFY FAIL: obsolete BTR-only-missing-source attempt path returned")
if "PASS45_BTR_AXIS_OPTIC_20260827_R2" in import_py:
    raise SystemExit("PASS20 VERIFY FAIL: current production importer regressed to BTR R2")

for needle in (
    "ukrainian_hmmwv_mk_19.glb",
    "m2_50cal_machinegun_cc0.glb",
    "BTR4_Bucephalus.fbx",
    "Find-BtrFbxInNamedArchive",
    "Available models may still be imported independently",
):
    require(source_recovery, needle, "production source recovery truth")

for needle in (
    "[3/4] NORMAL GAME: optional production model intake is handled by START_HERE before this launcher.",
    "Missing exact production models remain visible content gaps; no proxy is called production-ready.",
):
    require(normal, needle, "legacy non-acceptance fallback remains source-compatible")

for needle in (
    "PASS19_PLAYABLE_WEAPON_SET_READY",
    "PASS15_MUSEUM_BASES_WEAPONS_READY",
    "PASS16_RUNTIME_GRAPHICS_IDENTITY",
    "PASS15_PERF_SAMPLE",
):
    require(playable, needle, "focused recovery route remains intact")

print("NORMAL GAME ROUTE PASS 20 + PASS45 QUICK/CANONICAL SPLIT SOURCE CONTRACT PASS")
print("- START_HERE option 1/3 selects lightweight mode on the one canonical gameplay launcher")
print("- quick normal bypasses LFS hydration, commandlet weapon preflight, vehicle/material import and acceptance gates")
print("- option 2 retains strict HMMWV/M2/BTR + weapon/material/evidence acceptance")
print("- BTR R3 canonical intake remains +X-forward / glTF +Y-up with explicit provenance")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 runtime still required")
