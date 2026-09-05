from pathlib import Path
import re
from pass45_runtime_route_contract import ROOT, read, require, validate_runtime_route

route = validate_runtime_route()
spawn_cpp = read("OsterConflict/Source/OsterConflict/Private/OCTeamSpawnPoint.cpp")
foliage_cpp = read("OsterConflict/Source/OsterConflict/Private/OCDenseGroundFoliageSubsystem.cpp")
fx = read("OsterConflict/Source/OsterConflict/Private/OCTransientVisualFX.cpp")
fallback = read("OsterConflict/Source/OsterConflict/Private/OCRealWeaponFallbackSubsystem.cpp")
frontend = read("OsterConflict/Source/OsterConflict/Private/OCR13FrontendMenuSubsystem.cpp")
runtime_safe = read("OsterConflict/Source/OsterConflict/Private/OCGameModeRuntimeSafe.cpp")
all_asset_import = route["all_assets"]
production_import = read("OsterConflict/IMPORT_PRODUCTION_VEHICLES_UE58.cmd")
vehicle_fresh = read("OsterConflict/Scripts/verify_production_vehicle_fresh_load.py")
inbox_audit = read("OsterConflict/Scripts/audit_local_model_inbox.ps1")
weapon_source = read("OsterConflict/Scripts/prepare_local_weapon_sources.ps1")
weapon_import = read("OsterConflict/Scripts/import_local_production_weapon_assets.py")
weapon_fresh = read("OsterConflict/Scripts/verify_local_production_weapon_fresh_load.py")

for needle in ("AOCWorldSectorOster::MuseumAnchor()", "SpawnRuntimeBaseWeaponRack", "PASS37_BASE_RELOCATED_VISIBLE_MUSEUM_APPROACH"):
    require(spawn_cpp, needle, "Museum BASE source")
for needle in ("PASS44_ACTUAL_PAWN_MUSEUM_BASE_READY", "MaxMuseumBaseDistanceCm = 4500.0f", "RestartPlayerAtTransform"):
    require(runtime_safe, needle, "actual pawn BASE proof")
for needle in ("TryPopulateWhenGameplayReady", "PopulationBatchTimer", "PopulateBatch"):
    require(foliage_cpp, needle, "bounded foliage")
batch = re.search(r"constexpr\s+int32\s+CellsPerBatch\s*=\s*(\d+)\s*;", foliage_cpp)
if not batch or not 1 <= int(batch.group(1)) <= 96:
    raise SystemExit("RUNTIME ACCEPTANCE PASS 3 FAIL: invalid foliage batch ceiling")
for needle in ("ResolveFiringWeapon", "ResolveWeaponMuzzle", "Character->GetCurrentWeapon()", "TryResolveSocketMuzzle", "TryResolveBoundsMuzzle"):
    require(fx, needle, "weapon presentation")
for needle in ("GenericPistol", "OC_ProductionWeaponVisual", "MaxRefreshPasses = 12", "PASS38_WEAPON_FALLBACK_SCAN_STOPPED", "PASS44_WEAPON_AUTHORED_MATERIAL_GAP", "exact_production=0 playable_fallback=1"):
    require(fallback, needle, "fallback/material truth")
require(frontend, "PanelSlot->SetPosition(FVector2D(112.0f, 92.0f));", "frontend geometry")
for needle in ("audit_local_model_inbox.ps1", "prepare_all_local_inbox_assets.ps1", "dedupe_local_prepared_sources.py", "prepare_local_weapon_sources.ps1", "import_all_project_assets.py"):
    require(all_asset_import, needle, "aggregate all-asset intake")
for needle in ("import_production_vehicle_assets.py", "verify_production_vehicle_fresh_load.py", 'set "HMMWV_IMPORTED=0"', 'set "M2_IMPORTED=0"', 'set "BTR_IMPORTED=0"'):
    require(production_import, needle, "production vehicle intake")
for needle in ("/Game/Production/Vehicles/HMMWV/SM_HMMWV_UA", "/Game/Production/Weapons/M2/SM_M2_Browning", "/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus", "AUTHORED_MATERIALS_READY", "placeholder_slots"):
    require(vehicle_fresh, needle, "vehicle fresh-load truth")
for needle in ("models_game_OC", "asset_inventory.json", "UNSAFE/UNREADABLE", "M249", "REMINGTON870"):
    require(inbox_audit, needle, "local inbox audit")
for needle in ("M249", "Remington870", "weapon_sources.json", "models_game_OC"):
    require(weapon_source, needle, "exact weapon staging")
for needle in ("/Game/Production/Weapons/M249", "/Game/Production/Weapons/Remington870", "production_weapon_import_result.txt"):
    require(weapon_import, needle, "exact weapon import")
for needle in ("/Game/Production/Weapons/M249/SM_M249", "/Game/Production/Weapons/Remington870/SM_Remington870", "placeholder_slots", "STATUS="):
    require(weapon_fresh, needle, "weapon fresh-load truth")

print("RUNTIME ACCEPTANCE PASS 3 + PASS45 CURRENT CONTRACT PASS")
print("- packet runner owns all-asset intake and strict gameplay acceptance")
print("- exact vehicle/weapon content remains fail-visible")
print("STATUS: CODED_UNTESTED; local UE 5.8 build/playtest still required")
