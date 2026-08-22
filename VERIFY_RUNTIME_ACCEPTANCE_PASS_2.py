from pathlib import Path

ROOT = Path(__file__).resolve().parent

FILES = {
    "spawn_cpp": ROOT / "OsterConflict/Source/OsterConflict/Private/OCTeamSpawnPoint.cpp",
    "spawn_h": ROOT / "OsterConflict/Source/OsterConflict/Public/OCTeamSpawnPoint.h",
    "foliage_cpp": ROOT / "OsterConflict/Source/OsterConflict/Private/OCDenseGroundFoliageSubsystem.cpp",
    "foliage_h": ROOT / "OsterConflict/Source/OsterConflict/Public/OCDenseGroundFoliageSubsystem.h",
    "fx": ROOT / "OsterConflict/Source/OsterConflict/Private/OCTransientVisualFX.cpp",
    "stabilizer": ROOT / "OsterConflict/Source/OsterConflict/Private/OCR13UIViewportStabilizerSubsystem.cpp",
    "frontend": ROOT / "OsterConflict/Source/OsterConflict/Private/OCR13FrontendMenuSubsystem.cpp",
    "launcher": ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd",
    "production_import": ROOT / "OsterConflict/IMPORT_PRODUCTION_VEHICLES_UE58.cmd",
    "source_recovery": ROOT / "OsterConflict/Scripts/prepare_local_production_sources.ps1",
}


def read(name):
    path = FILES[name]
    if not path.is_file():
        raise SystemExit(f"RUNTIME ACCEPTANCE PASS 2 FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8")


def require(text, needle, where):
    if needle not in text:
        raise SystemExit(f"RUNTIME ACCEPTANCE PASS 2 FAIL: {where}: missing {needle!r}")


spawn_cpp = read("spawn_cpp")
spawn_h = read("spawn_h")
foliage_cpp = read("foliage_cpp")
foliage_h = read("foliage_h")
fx = read("fx")
stabilizer = read("stabilizer")
frontend = read("frontend")
launcher = read("launcher")
production_import = read("production_import")
source_recovery = read("source_recovery")

require(spawn_h, "virtual void BeginPlay() override;", "spawn header")
for needle in (
    "void AOCTeamSpawnPoint::BeginPlay()",
    "AOCWorldSectorOster::MuseumAnchor()",
    "FVector::DistSquared2D(GetActorLocation(), Museum)",
    "ConfigureServer(TeamId, true, NAME_None);",
    "SpawnRuntimeBaseWeaponRack",
):
    require(spawn_cpp, needle, "runtime museum spawn")

for needle in (
    "TryPopulateWhenGameplayReady",
    "GameplayReadyTimer",
):
    require(foliage_h, needle, "foliage header")
for needle in (
    "SetTimer(",
    "TryPopulateWhenGameplayReady",
    "if (GameMode->IsFrontendOnlySession()) return;",
    "Populate(*World);",
):
    require(foliage_cpp, needle, "frontend-to-gameplay foliage")

for needle in (
    "ResolveLocalWeaponMuzzle",
    "OC_ProductionWeaponVisual",
    "const FVector VisualStart = ResolveLocalWeaponMuzzle",
    "const FVector VisualMuzzle = ResolveLocalWeaponMuzzle",
):
    require(fx, needle, "muzzle/tracer presentation")

# The stabilizer may isolate layers, but geometry belongs only to the frontend owner.
require(frontend, "PanelSlot->SetPosition(FVector2D(112.0f, 92.0f));", "frontend canonical menu geometry")
require(frontend, "PanelSlot->SetSize(FVector2D(440.0f, 760.0f));", "frontend canonical menu geometry")
for forbidden in (
    "Slot->SetPosition(FVector2D(90.0f, 60.0f));",
    "Slot->SetSize(FVector2D(470.0f, 780.0f));",
):
    if forbidden in stabilizer:
        raise SystemExit(f"RUNTIME ACCEPTANCE PASS 2 FAIL: stabilizer still overrides menu geometry: {forbidden}")

for needle in (
    "IMPORT_PRODUCTION_VEHICLES_UE58.cmd",
    "Importing and validating REAL production HMMWV + M2 Browning + BTR-4 assets",
    "The game will not launch with civilian pickup/proxy turret/proxy BTR geometry pretending to be final assets.",
):
    require(launcher, needle, "normal gameplay production gate")

for needle in (
    "import_production_vehicle_assets.py",
    "prepare_local_production_sources.ps1",
    "production_import_success.txt",
    "-run=pythonscript",
    '-script="%PY_SCRIPT%"',
    "ProductionVehicleImport.log",
):
    require(production_import, needle, "full production importer")
if "-ExecutePythonScript=" in production_import:
    raise SystemExit("RUNTIME ACCEPTANCE PASS 2 FAIL: production importer still uses full-editor ExecutePythonScript route")

for needle in (
    "ukrainian_hmmwv_mk_19.glb",
    "m2_50cal_machinegun_cc0.glb",
    "BTR4_Bucephalus.fbx",
    "OsterConflict_vehicle_assets_ready.zip",
    "моделі.zip",
    "Nothing is uploaded or committed.",
):
    require(source_recovery, needle, "local production source recovery")

print("RUNTIME ACCEPTANCE PASS 2 SOURCE CONTRACT PASS")
print("- menu geometry has one owner")
print("- serialized base spawn is reasserted near Museum at runtime")
print("- foliage survives same-world frontend -> gameplay transition")
print("- local tracer/muzzle presentation is rebased to visible weapon geometry")
print("- normal gameplay is gated on full HMMWV + M2 + BTR production ingest")
print("- missing local production sources are recovered from prior user downloads/ZIPs without branch mutation")
print("- UE 5.8 production import uses the PythonScript commandlet and writes a dedicated import log")
print("STATUS: CODED_UNTESTED; local UE 5.8 build/playtest still required")
