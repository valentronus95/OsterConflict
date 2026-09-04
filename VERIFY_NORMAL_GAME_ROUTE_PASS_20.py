#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
START = ROOT / "START_HERE.cmd"
NORMAL = ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd"
RUNTIME_EVIDENCE = ROOT / "VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py"
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


def forbid(text: str, needle: str, label: str) -> None:
    if needle in text:
        raise SystemExit(f"PASS20 VERIFY FAIL: {label}: forbidden {needle!r}")


start = read(START)
normal = read(NORMAL)
runtime_evidence = read(RUNTIME_EVIDENCE)
importer = read(IMPORTER)
import_py = read(IMPORT_PY)
source_recovery = read(SOURCE_RECOVERY)

# START_HERE remains the only user-facing route: ingest all assets first, then call the canonical gameplay launcher.
for needle in (
    'set "CURRENT_GAMEPLAY=%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"',
    "call :ingest_all_assets",
    'call "%CURRENT_GAMEPLAY%"',
):
    require(start, needle, "START_HERE normal-game route")
forbid(start, "RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd", "retired recovery launcher")

for needle in (
    "verify_required_weapon_assets.py",
    "required_weapon_asset_preflight_success.txt",
    "Opening every required REAL/playable weapon visual",
    "Launching CURRENT NORMAL GAME frontend",
    "-Frontend",
    '/C:"fix/runtime-map-spawn-fps-assets-"',
    '/C:"fix/pass45-runtime-rejection-"',
    '/C:"fix/pass45-asset-"',
):
    require(normal, needle, "normal playable route")

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

# Command wrapper owns independent per-model results; source filenames stay in source recovery/Python import.
for needle in (
    'set "HMMWV_IMPORTED=0"',
    'set "M2_IMPORTED=0"',
    'set "BTR_IMPORTED=0"',
    "Continuing independent intake for available source files",
):
    require(importer, needle, "independent production intake command")
for needle in (
    "ukrainian_hmmwv_mk_19.glb",
    "m2_50cal_machinegun_cc0.glb",
    "BTR4_Bucephalus.fbx",
    'attempt("HMMWV"',
    'attempt("M2"',
    "if BTR_SOURCE.exists():",
    "build_btr4_glb(authored_btr)",
    "BTR4 local FBX missing; generated and imported Oster-authored fallback",
):
    require(import_py, needle, "independent production asset implementation")
for needle in (
    "ukrainian_hmmwv_mk_19.glb",
    "m2_50cal_machinegun_cc0.glb",
    "BTR4_Bucephalus.fbx",
    "Find-BtrFbxInNamedArchive",
    "Other inbox models remain in the inventory for their own gameplay/world integration pass",
):
    require(source_recovery, needle, "production source recovery truth")

for needle in (
    "[3/4] NORMAL GAME: optional production model intake is handled by START_HERE before this launcher.",
    "Missing exact production models remain visible content gaps; no proxy is called production-ready.",
):
    require(normal, needle, "normal-game content truth")

# Historical focused launcher markers are now carried by the canonical evidence verifier.
for needle in (
    "PASS19_PLAYABLE_WEAPON_SET_READY",
    "PASS19_PLAYABLE_WEAPON_SET_FAIL",
    "PASS45_INITIAL_BASE_DEPLOYMENT_VALIDATED_ONCE",
    "PASS14_PERF_SAMPLE",
    "PASS14_PERF_30FPS_READY",
):
    require(runtime_evidence, needle, "canonical runtime readiness route")

print("NORMAL GAME ROUTE PASS 20 + PASS 45 SOURCE CONTRACT PASS")
print("- START_HERE ingests assets before calling the canonical normal-game launcher")
print("- normal gameplay keeps the real/playable weapon preflight and branch-aware pre-merge test route")
print("- current fix/pass45-asset-* branches are explicitly runtime-testable before merge")
print("- exact source filenames belong to source-recovery/Python import; command wrapper owns per-model outcomes")
print("- BTR4 may use local FBX or authored generated fallback without blocking independent HMMWV/M2 intake")
print("- historical Pass15/19 runtime evidence is carried by the canonical Pass45 verifier")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 runtime still required")
