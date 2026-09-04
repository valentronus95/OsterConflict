from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent


def read(path):
    path = ROOT / path
    if not path.is_file():
        raise SystemExit(f"RUNTIME ACCEPTANCE PASS 3 FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(text, needle, where):
    if needle not in text:
        raise SystemExit(f"RUNTIME ACCEPTANCE PASS 3 FAIL: {where}: missing {needle!r}")


spawn_cpp = read("OsterConflict/Source/OsterConflict/Private/OCTeamSpawnPoint.cpp")
foliage_cpp = read("OsterConflict/Source/OsterConflict/Private/OCDenseGroundFoliageSubsystem.cpp")
fx = read("OsterConflict/Source/OsterConflict/Private/OCTransientVisualFX.cpp")
fallback = read("OsterConflict/Source/OsterConflict/Private/OCRealWeaponFallbackSubsystem.cpp")
frontend = read("OsterConflict/Source/OsterConflict/Private/OCR13FrontendMenuSubsystem.cpp")
launcher = read("RUN_R14_CURRENT_GAMEPLAY.cmd")
start_here = read("START_HERE.cmd")
all_asset_import = read("OsterConflict/IMPORT_ALL_LOCAL_INBOX_UE58.cmd")
lfs_verify = read("OsterConflict/Scripts/verify_playtest_lfs_payloads.ps1")
production_import = read("OsterConflict/IMPORT_PRODUCTION_VEHICLES_UE58.cmd")
fresh_vehicle_verify = read("OsterConflict/Scripts/verify_production_vehicle_fresh_load.py")
source_recovery = read("OsterConflict/Scripts/prepare_local_production_sources.ps1")
inbox_audit = read("OsterConflict/Scripts/audit_local_model_inbox.ps1")
weapon_source_recovery = read("OsterConflict/Scripts/prepare_local_weapon_sources.ps1")
weapon_import = read("OsterConflict/Scripts/import_local_production_weapon_assets.py")
weapon_fresh_verify = read("OsterConflict/Scripts/verify_local_production_weapon_fresh_load.py")
runtime_safe = read("OsterConflict/Source/OsterConflict/Private/OCGameModeRuntimeSafe.cpp")

# The underlying Museum BASE/rack source remains, while Pass 44 adds stronger live-pawn proof.
for needle in (
    "AOCWorldSectorOster::MuseumAnchor()",
    "SpawnRuntimeBaseWeaponRack",
    "PASS37_BASE_RELOCATED_VISIBLE_MUSEUM_APPROACH",
    "FVector(-1400.0f, -2400.0f, 120.0f)",
    "FVector(1400.0f, -2400.0f, 120.0f)",
):
    require(spawn_cpp, needle, "Museum BASE source")
for needle in (
    "PASS44_ACTUAL_PAWN_MUSEUM_BASE_READY",
    "MaxMuseumBaseDistanceCm = 4500.0f",
    "RestartPlayerAtTransform",
):
    require(runtime_safe, needle, "Pass 44 actual pawn proof")

# Dense foliage must remain incremental/bounded.
for needle in ("TryPopulateWhenGameplayReady", "PopulationBatchTimer", "PopulateBatch"):
    require(foliage_cpp, needle, "incremental foliage")
batch_match = re.search(r"constexpr\s+int32\s+CellsPerBatch\s*=\s*(\d+)\s*;", foliage_cpp)
if not batch_match or not 1 <= int(batch_match.group(1)) <= 96:
    raise SystemExit("RUNTIME ACCEPTANCE PASS 3 FAIL: invalid full-profile foliage batch ceiling")

# Weapon presentation still resolves the actual firing weapon/muzzle.
for needle in (
    "ResolveFiringWeapon", "ResolveWeaponMuzzle", "Character->GetCurrentWeapon()",
    "OC_ProductionWeaponVisual", "TryResolveSocketMuzzle", "TryResolveBoundsMuzzle", "GetLocalBounds",
):
    require(fx, needle, "muzzle/tracer presentation")

# Generic real fallbacks remain finite and never become exact production art. Missing authored materials are fail-visible.
for needle in (
    "GenericPistol", "OC_ProductionWeaponVisual", "MaxRefreshPasses = 12", "ClearTimer(RefreshTimer)",
    "PASS38_WEAPON_FALLBACK_SCAN_STOPPED", "PASS44_WEAPON_AUTHORED_MATERIAL_GAP",
    "exact_production=0 playable_fallback=1",
):
    require(fallback, needle, "bounded fallback/material truth")
if "UMaterialInstanceDynamic::Create" in fallback or "Component->SetMaterial(Slot" in fallback:
    raise SystemExit("RUNTIME ACCEPTANCE PASS 3 FAIL: grey runtime material repair returned")

require(frontend, "PanelSlot->SetPosition(FVector2D(112.0f, 92.0f));", "frontend canonical menu geometry")

# Current normal/strict split. START_HERE owns the full asset ingest before gameplay; the internal launcher
# owns the strict production acceptance stage. Asset-fix branches must be testable before merge.
for needle in (
    "IMPORT_PRODUCTION_VEHICLES_UE58.cmd",
    'if "%IS_ACCEPTANCE%"=="1" (',
    "[3/4] STRICT ACCEPTANCE: importing and validating REAL production HMMWV + M2 Browning + BTR-4 assets",
    'call "%PRODUCTION_IMPORT%"',
    "Missing exact production models remain visible content gaps; no proxy is called production-ready.",
    "git lfs pull origin",
    "git lfs checkout >nul",
    "verify_playtest_lfs_payloads.ps1",
    '/C:"fix/runtime-map-spawn-fps-assets-"',
    '/C:"fix/pass45-asset-"',
):
    require(launcher, needle, "current normal/strict gameplay split")
if "--include=" in launcher:
    raise SystemExit("RUNTIME ACCEPTANCE PASS 3 FAIL: unsupported Git LFS --include flag returned")

for needle in (
    'set "ALL_ASSET_IMPORT=%~dp0OsterConflict\\IMPORT_ALL_LOCAL_INBOX_UE58.cmd"',
    "call :ingest_all_assets",
    'set "OC_FORCE_ACCEPTANCE=1"',
    'call "%CURRENT_GAMEPLAY%"',
):
    require(start_here, needle, "single launcher full-runtime model intake")
for needle in (
    "audit_local_model_inbox.ps1",
    "Інвентаризую ВСІ локальні ZIP",
    'powershell -NoProfile -ExecutionPolicy Bypass -File "%AUDIT%" -ProjectDir "%PROJECT_DIR%"',
    "prepare_all_local_inbox_assets.ps1",
    "dedupe_local_prepared_sources.py",
    "prepare_local_weapon_sources.ps1",
    "import_all_project_assets.py",
):
    require(all_asset_import, needle, "canonical all-asset ingest pipeline")

for needle in (
    "Content\\AK-47", "Content\\R13\\Weapons", "Content\\PN_FoliageCollection",
    "Unhydrated Git LFS model files remain", "version https://git-lfs.github.com/spec/v1",
):
    require(lfs_verify, needle, "LFS payload verification")

# Production vehicle intake remains independent per model and fresh-load verifies authored material truth.
for needle in (
    "import_production_vehicle_assets.py", "verify_production_vehicle_fresh_load.py",
    "production_import_success.txt", "production_fresh_load_success.txt", "-run=pythonscript",
    'set "HMMWV_IMPORTED=0"', 'set "M2_IMPORTED=0"', 'set "BTR_IMPORTED=0"',
    "prepare_local_weapon_sources.ps1", "import_local_production_weapon_assets.py",
    "verify_local_production_weapon_fresh_load.py", "production_weapon_fresh_load_result.txt",
):
    require(production_import, needle, "strict production importer")
for needle in (
    "/Game/Production/Vehicles/HMMWV/SM_HMMWV_UA",
    "/Game/Production/Weapons/M2/SM_M2_Browning",
    "/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus",
    "AUTHORED_MATERIALS_READY", "placeholder_slots", "basicshapematerial",
):
    require(fresh_vehicle_verify, needle, "fresh-load production material truth")
for needle in (
    "ukrainian_hmmwv_mk_19.glb", "m2_50cal_machinegun_cc0.glb", "BTR4_Bucephalus.fbx",
    "OsterConflict_vehicle_assets_ready.zip", "Find-BtrFbxInNamedArchive", "models_game_OC",
    "Other inbox models remain in the inventory for their own gameplay/world integration pass",
):
    require(source_recovery, needle, "local production source recovery")

# Every local ZIP is inventoried before strict acceptance. Exact M249/Remington are no longer allowed to hide behind R13 fallbacks.
for needle in (
    "models_game_OC", "asset_inventory.json", "Get-FileHash", "UNSAFE/UNREADABLE", "M249", "REMINGTON870",
    "PICKUP", "CHARACTER_SKIN", "BUILDING_WORLD",
):
    require(inbox_audit, needle, "local model inbox audit")
for needle in (
    "M249", "Remington870", "weapon_sources.json", "models_game_OC", "Unsafe ZIP entry",
):
    require(weapon_source_recovery, needle, "exact local weapon source staging")
for needle in (
    "/Game/Production/Weapons/M249", "SM_M249", "/Game/Production/Weapons/Remington870", "SM_Remington870",
    "production_weapon_import_result.txt",
):
    require(weapon_import, needle, "exact production weapon import")
for needle in (
    "/Game/Production/Weapons/M249/SM_M249",
    "/Game/Production/Weapons/Remington870/SM_Remington870",
    "placeholder_slots", "get_used_textures", "STATUS=",
):
    require(weapon_fresh_verify, needle, "exact production weapon fresh-load dependency validation")

print("RUNTIME ACCEPTANCE PASS 3 + PASS45 CURRENT CONTRACT PASS")
print("- Museum BASE source remains and actual live-pawn Museum proof is retained")
print("- foliage and weapon helper work stays bounded")
print("- START_HERE delegates one canonical all-asset ingest pipeline before strict runtime gameplay")
print("- the all-asset pipeline audits every local inbox ZIP, prepares/dedupes sources and imports project/Fab content")
print("- HMMWV/M2/BTR plus exact M249/Remington intake fail visible instead of promoting missing content")
print("- production fresh-load rejects placeholder material/dependency gaps")
print("STATUS: CODED_UNTESTED; local UE 5.8 build/playtest still required")
