#!/usr/bin/env python3
from pass45_runtime_route_contract import ROOT, read, require, forbid, validate_runtime_route

SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
def src(rel: str) -> str:
    return (SRC / rel).read_text(encoding="utf-8", errors="replace")

fallback = src("Private/OCRealWeaponFallbackSubsystem.cpp")
pass19 = src("Private/OCContentReadinessPass19Subsystem.cpp")
strict = src("Private/OCProductionVehicleRuntimeValidationSubsystem.cpp")
vehicle_import = read("OsterConflict/Scripts/import_production_vehicle_assets.py")
vehicle_cmd = read("OsterConflict/IMPORT_PRODUCTION_VEHICLES_UE58.cmd")
vehicle_fresh = read("OsterConflict/Scripts/verify_production_vehicle_fresh_load.py")
route = validate_runtime_route()

require(fallback, 'RealFallbackComponentTag(TEXT("OC_RealFallbackWeaponVisual"))', "fallback identity")
require(fallback, "exact_production=0 playable_fallback=1", "fallback truth")
forbid(fallback, "Visual->ComponentTags.Add(ProductionVisualTag);", "fallback production impersonation")
for needle in ("PASS7_PRODUCTION_WEAPONS_READY", "PASS7_PRODUCTION_WEAPON_RUNTIME_FAIL", "OC_ProductionWeaponVisual"):
    require(strict, needle, "strict production gate")
for needle in ("AllRequiredRackWeaponClassesMask", "OC_RuntimeBaseWeaponRack", "OC_ProductionWeaponVisual", "OC_RealFallbackWeaponVisual", "AOCWeapon_M14", "AOCWeapon_Mac10", "AOCWeapon_Tec9", "AOCWeapon_LeverAction", "AOCAntiArmorLauncher", "PrimitiveOnlyCount == 0", "PASS19_PLAYABLE_WEAPON_SET_READY", "PASS19_PLAYABLE_WEAPON_SET_FAIL"):
    require(pass19, needle, "11-class playable readiness")
for needle in ("PASS19_PLAYABLE_WEAPON_SET_READY", "PASS19_PLAYABLE_WEAPON_SET_FAIL"):
    require(route["evidence"], needle, "Pass19 runtime evidence")
for needle in ("ukrainian_hmmwv_mk_19.glb", "m2_50cal_machinegun_cc0.glb", "BTR4_Bucephalus.fbx", 'attempt("HMMWV"', 'attempt("M2"', "if BTR_SOURCE.exists():", "build_btr4_glb(authored_btr)", "BTR4_IMPORT_FAILED=", "CONTENT_GAP="):
    require(vehicle_import, needle, "independent production vehicle intake")
for needle in ('set "HMMWV_IMPORTED=0"', 'set "M2_IMPORTED=0"', 'set "BTR_IMPORTED=0"', "Continuing independent intake for available source files"):
    require(vehicle_cmd, needle, "per-model command results")
for needle in ("AUTHORED_MATERIALS_READY", "placeholder_slots", "basicshapematerial", "defaultmaterial"):
    require(vehicle_fresh, needle, "fresh-load material truth")

print("CONTENT READINESS PASS 19 + PASS45 CONTRACT PASS")
print("- playable fallback and exact production certification remain separate")
print("- packet runtime owns acceptance; START_HERE remains delegation-only")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 runtime and exact asset intake remain required")
