from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent

FILES = {
    "spawn_cpp": ROOT / "OsterConflict/Source/OsterConflict/Private/OCTeamSpawnPoint.cpp",
    "spawn_h": ROOT / "OsterConflict/Source/OsterConflict/Public/OCTeamSpawnPoint.h",
    "foliage_cpp": ROOT / "OsterConflict/Source/OsterConflict/Private/OCDenseGroundFoliageSubsystem.cpp",
    "foliage_h": ROOT / "OsterConflict/Source/OsterConflict/Public/OCDenseGroundFoliageSubsystem.h",
    "fx": ROOT / "OsterConflict/Source/OsterConflict/Private/OCTransientVisualFX.cpp",
    "fallback": ROOT / "OsterConflict/Source/OsterConflict/Private/OCRealWeaponFallbackSubsystem.cpp",
    "stabilizer": ROOT / "OsterConflict/Source/OsterConflict/Private/OCR13UIViewportStabilizerSubsystem.cpp",
    "frontend": ROOT / "OsterConflict/Source/OsterConflict/Private/OCR13FrontendMenuSubsystem.cpp",
    "launcher": ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd",
    "lfs_verify": ROOT / "OsterConflict/Scripts/verify_playtest_lfs_payloads.ps1",
    "production_import": ROOT / "OsterConflict/IMPORT_PRODUCTION_VEHICLES_UE58.cmd",
    "fresh_vehicle_verify": ROOT / "OsterConflict/Scripts/verify_production_vehicle_fresh_load.py",
    "source_recovery": ROOT / "OsterConflict/Scripts/prepare_local_production_sources.ps1",
}


def read(name):
    path = FILES[name]
    if not path.is_file():
        raise SystemExit(f"RUNTIME ACCEPTANCE PASS 3 FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8")


def require(text, needle, where):
    if needle not in text:
        raise SystemExit(f"RUNTIME ACCEPTANCE PASS 3 FAIL: {where}: missing {needle!r}")


spawn_cpp = read("spawn_cpp")
spawn_h = read("spawn_h")
foliage_cpp = read("foliage_cpp")
foliage_h = read("foliage_h")
fx = read("fx")
fallback = read("fallback")
stabilizer = read("stabilizer")
frontend = read("frontend")
launcher = read("launcher")
lfs_verify = read("lfs_verify")
production_import = read("production_import")
fresh_vehicle_verify = read("fresh_vehicle_verify")
source_recovery = read("source_recovery")

require(spawn_h, "virtual void BeginPlay() override;", "spawn header")
for needle in (
    "void AOCTeamSpawnPoint::BeginPlay()",
    "AOCWorldSectorOster::MuseumAnchor()",
    "FVector::DistSquared2D(GetActorLocation(), Museum)",
    "ConfigureServer(TeamId, true, NAME_None);",
    "SpawnRuntimeBaseWeaponRack",
    "FVector(-1450.0f, -900.0f, 120.0f)",
    "FVector(1450.0f, 900.0f, 120.0f)",
):
    require(spawn_cpp, needle, "runtime museum spawn")

for needle in (
    "TryPopulateWhenGameplayReady",
    "PopulationBatchTimer",
    "PopulateBatch",
    "bPopulationStarted",
):
    require(foliage_h, needle, "foliage header")
for needle in (
    "TryPopulateWhenGameplayReady",
    "if (GameMode->IsFrontendOnlySession()) return;",
    "BeginPopulation(*World)",
    "void UOCDenseGroundFoliageSubsystem::PopulateBatch()",
    "World->GetTimerManager().SetTimer(",
):
    require(foliage_cpp, needle, "non-blocking frontend-to-gameplay foliage")
batch_match = re.search(r"constexpr\s+int32\s+CellsPerBatch\s*=\s*(\d+)\s*;", foliage_cpp)
if not batch_match:
    raise SystemExit("RUNTIME ACCEPTANCE PASS 3 FAIL: foliage batch-size contract is missing")
batch_size = int(batch_match.group(1))
if not 1 <= batch_size <= 96:
    raise SystemExit(f"RUNTIME ACCEPTANCE PASS 3 FAIL: foliage batch size {batch_size} exceeds accepted non-blocking ceiling 96")
if "Populate(*World);" in foliage_cpp:
    raise SystemExit("RUNTIME ACCEPTANCE PASS 3 FAIL: foliage still performs synchronous whole-map population")

for needle in (
    "ResolveFiringWeapon",
    "ResolveWeaponMuzzle",
    "Character->GetCurrentWeapon()",
    "OC_ProductionWeaponVisual",
    "TryResolveSocketMuzzle",
    "TryResolveBoundsMuzzle",
    "GetLocalBounds",
    "const FVector VisualStart = ResolveWeaponMuzzle",
    "const FVector VisualMuzzle = ResolveWeaponMuzzle",
):
    require(fx, needle, "muzzle/tracer presentation")
if "Bounds.Origin + SafeDirection * Support" in fx:
    raise SystemExit("RUNTIME ACCEPTANCE PASS 3 FAIL: muzzle resolver still uses world-AABB centre projection")

for needle in (
    "RefreshWeaponFallbacks();",
    "SetTimer(",
    "GenericPistol",
    "OC_ProductionWeaponVisual",
):
    require(fallback, needle, "real weapon fallback runtime")
if "if (GameMode->IsFrontendOnlySession()) return;" in fallback:
    raise SystemExit("RUNTIME ACCEPTANCE PASS 3 FAIL: real weapon fallback still dies permanently in frontend world")

# The stabilizer may isolate layers, but geometry belongs only to the frontend owner.
require(frontend, "PanelSlot->SetPosition(FVector2D(112.0f, 92.0f));", "frontend canonical menu geometry")
require(frontend, "PanelSlot->SetSize(FVector2D(440.0f, 760.0f));", "frontend canonical menu geometry")
for forbidden in (
    "Slot->SetPosition(FVector2D(90.0f, 60.0f));",
    "Slot->SetSize(FVector2D(470.0f, 780.0f));",
):
    if forbidden in stabilizer:
        raise SystemExit(f"RUNTIME ACCEPTANCE PASS 3 FAIL: stabilizer still overrides menu geometry: {forbidden}")

for needle in (
    "IMPORT_PRODUCTION_VEHICLES_UE58.cmd",
    "Importing and validating REAL production HMMWV + M2 Browning + BTR-4 assets",
    "The game will not launch with civilian pickup/proxy turret/proxy BTR geometry pretending to be final assets.",
    "git lfs pull origin",
    "git lfs checkout >nul",
    "verify_playtest_lfs_payloads.ps1",
):
    require(launcher, needle, "normal gameplay production/LFS gate")
if "--include=" in launcher:
    raise SystemExit("RUNTIME ACCEPTANCE PASS 3 FAIL: unsupported Git LFS --include flag returned")
if "^|" in launcher:
    raise SystemExit("RUNTIME ACCEPTANCE PASS 3 FAIL: invalid cmd caret-pipe returned to PowerShell path")

for needle in (
    "Content\\AK-47",
    "Content\\R13\\Weapons",
    "Content\\PN_FoliageCollection",
    "Unhydrated Git LFS model files remain",
    "version https://git-lfs.github.com/spec/v1",
):
    require(lfs_verify, needle, "standalone LFS payload verification")

for needle in (
    "import_production_vehicle_assets.py",
    "verify_production_vehicle_fresh_load.py",
    "prepare_local_production_sources.ps1",
    "production_import_success.txt",
    "production_fresh_load_success.txt",
    "-run=pythonscript",
    '-script="%PY_SCRIPT%"',
    '-script="%VERIFY_SCRIPT%"',
    "ProductionVehicleImport.log",
    "ProductionVehicleFreshLoad.log",
):
    require(production_import, needle, "full production importer")
if "-ExecutePythonScript=" in production_import:
    raise SystemExit("RUNTIME ACCEPTANCE PASS 3 FAIL: production importer still uses full-editor ExecutePythonScript route")

for needle in (
    "/Game/Production/Vehicles/HMMWV/SM_HMMWV_UA",
    "/Game/Production/Weapons/M2/SM_M2_Browning",
    "/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus",
    "unreal.load_asset",
    "production_fresh_load_success.txt",
):
    require(fresh_vehicle_verify, needle, "fresh-process production vehicle verification")

for needle in (
    "ukrainian_hmmwv_mk_19.glb",
    "m2_50cal_machinegun_cc0.glb",
    "BTR4_Bucephalus.fbx",
    "OsterConflict_vehicle_assets_ready.zip",
    "моделі.zip",
    "Nothing is uploaded or committed.",
):
    require(source_recovery, needle, "local production source recovery")

print("RUNTIME ACCEPTANCE PASS 3 SOURCE CONTRACT PASS")
print("- BASE is placed directly beside the Museum test hub")
print("- dense foliage is generated incrementally with a bounded per-frame batch instead of freezing deployment")
print("- restored weapon and foliage LFS payloads are hydrated before playtest using Windows-compatible commands")
print("- M1911/M249/MAC10/Rem870 real-mesh fallbacks remain active after frontend")
print("- tracer/muzzle presentation resolves the actual firing CurrentWeapon and socket/local-mesh barrel geometry")
print("- HMMWV + M2 + BTR are reopened in a fresh UE process before gameplay starts")
print("STATUS: CODED_UNTESTED; local UE 5.8 build/playtest still required")
