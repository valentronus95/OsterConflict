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

# Block0 requests all candidate foliage packages asynchronously, waits for completion, then resolves resident
# meshes only. Population is presentation work rather than a spawn gate, so live gameplay gets a strict
# per-frame CPU budget without changing authored density, cull distances, candidate meshes or surface guards.
for needle in (
    "RequestPreload",
    "RequestAsyncLoad",
    "PreloadHandle->HasLoadCompleted()",
    "BeginPopulation(*World)",
    "PopulateBatch",
    "ActiveCellsPerBatch = bLowCPUProfile ? LowCPUCellsPerBatch : FullCellsPerBatch",
    "FullPopulationFrameBudgetMilliseconds = 2.0",
    "LowCPUPopulationFrameBudgetMilliseconds = 1.25",
    "FSoftObjectPath(Path).ResolveObject()",
    "FrameBudgetMilliseconds",
    "live_gameplay_safe=1",
    "frame_budgeted=1",
    "sync_load=0",
    "full_playable_bounds=1",
):
    require(foliage_cpp, needle, "frame-budgeted resident-only Block0 foliage")
if "LoadObject<" in foliage_cpp:
    raise SystemExit("RUNTIME ACCEPTANCE PASS 3 FAIL: Block0 foliage reintroduced synchronous package loading")
full_batch = re.search(r"constexpr\s+int32\s+FullCellsPerBatch\s*=\s*(\d+)\s*;", foliage_cpp)
low_batch = re.search(r"constexpr\s+int32\s+LowCPUCellsPerBatch\s*=\s*(\d+)\s*;", foliage_cpp)
if not full_batch or not 1 <= int(full_batch.group(1)) <= 32:
    raise SystemExit("RUNTIME ACCEPTANCE PASS 3 FAIL: invalid full-profile foliage batch ceiling")
if not low_batch or not 1 <= int(low_batch.group(1)) <= 48:
    raise SystemExit("RUNTIME ACCEPTANCE PASS 3 FAIL: invalid LowCPU foliage batch ceiling")

# Weapon presentation still resolves the actual firing weapon/muzzle.
for needle in (
    "ResolveFiringWeapon", "ResolveWeaponMuzzle", "Character->GetCurrentWeapon()",
    "OC_ProductionWeaponVisual", "TryResolveSocketMuzzle", "TryResolveBoundsMuzzle", "GetLocalBounds",
):
    require(fx, needle, "muzzle/tracer presentation")

# Wrong-identity generic weapon substitution stays retired. Normal gameplay now performs only a one-shot
# BasicShape cleanup; expensive material/texture auditing is explicitly validation-only and may use a short,
# bounded retry loop there. Do not freeze production code around an obsolete exact pass count.
for needle in (
    "OC_ProductionWeaponVisual",
    'FParse::Param(FCommandLine::Get(), TEXT("ValidateProductionWeapons"))',
    "GAME_RECOVERY_WEAPON_FALLBACK_GAMEPLAY_READY",
    "material_audit=0",
    "repeated_timer=0",
    "PASS45_GENERIC_WEAPON_FALLBACK_RETIRED",
    "PASS38_WEAPON_FALLBACK_SCAN_BOUNDED_STOP",
    "PASS44_WEAPON_AUTHORED_MATERIAL_GAP",
    "generic_substitution=0",
    "Wrong-identity replacement is intentionally forbidden.",
):
    require(fallback, needle, "retired generic fallback / validation-only material audit truth")
for forbidden in (
    "exact_production=0 playable_fallback=1",
    "PASS38_WEAPON_FALLBACK_SCAN_STOPPED",
):
    if forbidden in fallback:
        raise SystemExit(f"RUNTIME ACCEPTANCE PASS 3 FAIL: retired generic fallback marker returned: {forbidden}")
if "UMaterialInstanceDynamic::Create" in fallback or "Component->SetMaterial(Slot" in fallback:
    raise SystemExit("RUNTIME ACCEPTANCE PASS 3 FAIL: grey runtime material repair returned")
refresh_passes = re.search(r"constexpr\s+int32\s+MaxRefreshPasses\s*=\s*(\d+)\s*;", fallback)
if not refresh_passes or not 1 <= int(refresh_passes.group(1)) <= 12:
    raise SystemExit("RUNTIME ACCEPTANCE PASS 3 FAIL: validation-only weapon audit retry ceiling is missing or unbounded")

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
    "Other inbox models remain in the inventory for their own gameplay/world integration pass; they are never silently called READY.",
):
    require(source_recovery, needle, "local production source recovery")

print("RUNTIME ACCEPTANCE PASS 3 + PASS 45 CURRENT CONTRACT PASS")
print("- Museum BASE source remains and actual live-pawn Museum proof is now stronger")
print("- Block0 foliage stays resident-only and preserves authored coverage while frame-budgeting live gameplay work")
print("- wrong-identity generic weapon substitution remains retired; gameplay cleanup is one-shot and material audit is validation-only")
print("- normal/strict launch flow follows current independent content intake instead of the retired all-or-nothing rule")
print("- production fresh-load rejects placeholder materials")
print("STATUS: SOURCE VERIFIED ONLY; local UE 5.8 build/playtest still required")
