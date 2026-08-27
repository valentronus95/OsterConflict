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

require(start, 'call "%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"', "START_HERE normal-game route")
require(start, 'call "%~dp0RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd"', "START_HERE full runtime route")
if 'call "%~dp0RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd"' in start:
    raise SystemExit("PASS20 VERIFY FAIL: START_HERE option 1 is incorrectly routed through recovery acceptance")

# Pass45 item 12 makes material freshness part of the user launcher while option 1 remains the normal frontend.
for needle in (
    ":prepare_materials_optional",
    "TRY_PRODUCTION_VEHICLES_UE58.cmd",
    "TRY_PASS45_STEIN_WEAPON_MATERIALS_UE58.cmd",
):
    require(start, needle, "START_HERE Pass45 material preflight")

for needle in (
    "verify_required_weapon_assets.py",
    "required_weapon_asset_preflight_success.txt",
    "Opening every required REAL/playable weapon visual",
    "Launching CURRENT NORMAL GAME frontend",
    "-Frontend",
    '/C:"fix/runtime-map-spawn-fps-assets-"',
):
    require(normal, needle, "normal playable route")

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

# Command wrapper still owns independent per-model intake results. A failed local source recovery may no longer
# imply a missing BTR because the canonical importer owns a repository-authored +X-forward fallback.
for needle in (
    'set "HMMWV_IMPORTED=0"',
    'set "M2_IMPORTED=0"',
    'set "BTR_IMPORTED=0"',
    'set "BTR_AXIS_READY=0"',
    "Continuing independent intake; canonical BTR authored fallback remains available.",
    'set "REQUIRED_REVISION=PASS45_BTR_AXIS_OPTIC_20260827_R2"',
    'BTR4_FORWARD_AXIS=+X',
):
    require(importer, needle, "independent production intake command")

# Pass45 supersedes the old `attempt("BTR4")` contract. HMMWV/M2 remain independent external sources;
# BTR has a dedicated canonical resolver and R2 requires explicit +X-forward provenance for the remote-optic path.
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
    'IMPORT_CONTRACT_REVISION = "PASS45_BTR_AXIS_OPTIC_20260827_R2"',
):
    require(import_py, needle, "current production asset implementation")
if 'attempt("BTR4"' in import_py:
    raise SystemExit("PASS20 VERIFY FAIL: obsolete BTR-only-missing-source attempt path returned")

# Local source recovery may still discover a higher-authority user FBX. Its absence no longer means canonical
# BTR presentation must vanish, because the authored repository fallback is now the current normal intake fallback.
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
    require(normal, needle, "normal-game content truth")

for needle in (
    "PASS19_PLAYABLE_WEAPON_SET_READY",
    "PASS15_MUSEUM_BASES_WEAPONS_READY",
    "PASS16_RUNTIME_GRAPHICS_IDENTITY",
    "PASS15_PERF_SAMPLE",
):
    require(playable, needle, "focused recovery route remains intact")

print("NORMAL GAME ROUTE PASS 20 + PASS45 BTR R2 MATERIAL/AXIS INTAKE SOURCE CONTRACT PASS")
print("- START_HERE option 1 stays on the canonical normal-game launcher; option 2 uses the strict main wrapper")
print("- HMMWV/M2 remain independent external-source imports")
print("- BTR R2 canonical intake keeps repository-authored fallback and requires +X-forward provenance")
print("- Gate F uses exact production OR explicit real fallback without false production READY")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 runtime still required")
