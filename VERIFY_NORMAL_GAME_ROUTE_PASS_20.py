#!/usr/bin/env python3
from pass45_runtime_route_contract import ROOT, read, require, forbid, validate_runtime_route

route = validate_runtime_route()
start = route["start"]
normal = route["normal"]
importer = read("OsterConflict/IMPORT_PRODUCTION_VEHICLES_UE58.cmd")
import_py = read("OsterConflict/Scripts/import_production_vehicle_assets.py")
source_recovery = read("OsterConflict/Scripts/prepare_local_production_sources.ps1")

# Ordinary game must be cheap: no strict reimport/fresh-load fan-out before gameplay.
for needle in ('set "CURRENT_GAMEPLAY=%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"', 'call "%CURRENT_GAMEPLAY%"'):
    require(start, needle, "START_HERE normal route")
for stale in ("call :ingest_all_assets", "RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd"):
    forbid(start, stale, "ordinary game route")

for needle in ("verify_required_weapon_assets.py", "required_weapon_asset_preflight_success.txt", "Opening every required REAL/playable weapon visual", "Launching CURRENT NORMAL GAME frontend", "-Frontend", '/C:"fix/pass45-asset-"'):
    require(normal, needle, "normal playable route")
strict_stage = normal.find("[3/4] STRICT ACCEPTANCE")
acceptance_gate = normal.rfind('if "%IS_ACCEPTANCE%"=="1" (', 0, strict_stage)
import_call = normal.find('call "%PRODUCTION_IMPORT%"', strict_stage)
if min(strict_stage, acceptance_gate, import_call) < 0 or not acceptance_gate < strict_stage < import_call:
    raise SystemExit("PASS20 VERIFY FAIL: production importer escaped strict acceptance")
for needle in ("IMPORT_PRODUCTION_VEHICLES_UE58.cmd", "PASS7_PRODUCTION_VEHICLES_READY", "PASS7_PRODUCTION_WEAPONS_READY"):
    require(normal, needle, "strict runtime route")
for needle in ('set "HMMWV_IMPORTED=0"', 'set "M2_IMPORTED=0"', 'set "BTR_IMPORTED=0"', "Continuing independent intake for available source files"):
    require(importer, needle, "independent production intake")
for needle in ("ukrainian_hmmwv_mk_19.glb", "m2_50cal_machinegun_cc0.glb", "BTR4_Bucephalus.fbx", 'attempt("HMMWV"', 'attempt("M2"', "build_btr4_glb(authored_btr)"):
    require(import_py, needle, "production implementation")
for needle in ("Find-BtrFbxInNamedArchive", "Other inbox models remain in the inventory for their own gameplay/world integration pass"):
    require(source_recovery, needle, "source recovery")

print("NORMAL GAME ROUTE PASS 20 + PASS45 SOURCE CONTRACT PASS")
print("- normal game does not run strict asset ingest; option 2 owns acceptance")
print("- exact production import remains acceptance-only and fail-visible")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 runtime still required")
