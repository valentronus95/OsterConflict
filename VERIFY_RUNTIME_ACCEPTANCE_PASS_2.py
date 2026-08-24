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
lfs_verify = read("OsterConflict/Scripts/verify_playtest_lfs_payloads.ps1")
production_import = read("OsterConflict/IMPORT_PRODUCTION_VEHICLES_UE58.cmd")
fresh_vehicle_verify = read("OsterConflict/Scripts/verify_production_vehicle_fresh_load.py")
source_recovery = read("OsterConflict/Scripts/prepare_local_production_sources.ps1")
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

# Pass 44 current normal/strict split. Normal mode does not run a second strict importer here because START_HERE
# already performs optional independent intake; strict acceptance still calls the canonical importer and fails closed.
for needle in (
    "IMPORT_PRODUCTION_VEHICLES_UE58.cmd",
    'if "%IS_ACCEPTANCE%"=="1" (',
    "[3/4] STRICT ACCEPTANCE: importing and validating REAL production HMMWV + M2 Browning + BTR-4 assets",
    'call "%PRODUCTION_IMPORT%"',
    "[3/4] NORMAL GAME: optional production model intake is handled by START_HERE before this launcher.",
    "Missing exact production models remain visible content gaps; no proxy is called production-ready.",
    "git lfs pull origin",
    "git lfs checkout >nul",
    "verify_playtest_lfs_payloads.ps1",
    '/C:"fix/runtime-map-spawn-fps-assets-"',
):
    require(launcher, needle, "current normal/strict gameplay split")
if "--include=" in launcher:
    raise SystemExit("RUNTIME ACCEPTANCE PASS 3 FAIL: unsupported Git LFS --include flag returned")

for needle in (
    "Content\\AK-47", "Content\\R13\\Weapons", "Content\\PN_FoliageCollection",
    "Unhydrated Git LFS model files remain", "version https://git-lfs.github.com/spec/v1",
):
    require(lfs_verify, needle, "LFS payload verification")

# Production intake is independent per model and fresh-load verifies authored material truth.
for needle in (
    "import_production_vehicle_assets.py", "verify_production_vehicle_fresh_load.py",
    "production_import_success.txt", "production_fresh_load_success.txt", "-run=pythonscript",
    'set "HMMWV_IMPORTED=0"', 'set "M2_IMPORTED=0"', 'set "BTR_IMPORTED=0"',
):
    require(production_import, needle, "independent production importer")
for needle in (
    "/Game/Production/Vehicles/HMMWV/SM_HMMWV_UA",
    "/Game/Production/Weapons/M2/SM_M2_Browning",
    "/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus",
    "AUTHORED_MATERIALS_READY", "placeholder_slots", "basicshapematerial",
):
    require(fresh_vehicle_verify, needle, "fresh-load production material truth")
for needle in (
    "ukrainian_hmmwv_mk_19.glb", "m2_50cal_machinegun_cc0.glb", "BTR4_Bucephalus.fbx",
    "OsterConflict_vehicle_assets_ready.zip", "Find-BtrFbxInNamedArchive",
    "Available models may still be imported independently; missing models remain explicit content gaps.",
):
    require(source_recovery, needle, "local production source recovery")

print("RUNTIME ACCEPTANCE PASS 3 + PASS 44 CURRENT CONTRACT PASS")
print("- Museum BASE source remains and actual live-pawn Museum proof is now stronger")
print("- foliage and weapon helper work stays bounded")
print("- normal/strict launch flow follows current independent content intake instead of the retired all-or-nothing rule")
print("- production fresh-load rejects placeholder materials")
print("STATUS: CODED_UNTESTED; local UE 5.8 build/playtest still required")
