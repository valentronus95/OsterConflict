#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
P = ROOT / "OsterConflict"
SRC = P / "Source" / "OsterConflict"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS44 VERIFY FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        raise SystemExit(f"PASS44 VERIFY FAIL: {label}: missing {needle!r}")


def forbid(text: str, needle: str, label: str) -> None:
    if needle in text:
        raise SystemExit(f"PASS44 VERIFY FAIL: {label}: forbidden {needle!r}")


agents = read(ROOT / "AGENTS.md")
ledger = read(ROOT / "OSTER_CONFLICT_WORK_LEDGER.md")
engine = read(P / "Config" / "DefaultEngine.ini")
game_h = read(SRC / "Public" / "OCGameMode.h")
runtime_h = read(SRC / "Public" / "OCGameModeRuntimeSafe.h")
runtime = read(SRC / "Private" / "OCGameModeRuntimeSafe.cpp")
central_h = read(SRC / "Public" / "OCCentralPlayableAreaSubsystem.h")
central = read(SRC / "Private" / "OCCentralPlayableAreaSubsystem.cpp")
tactical_h = read(SRC / "Public" / "OCTacticalMapSubsystem.h")
weapon_h = read(SRC / "Public" / "OCRealWeaponFallbackSubsystem.h")
weapon = read(SRC / "Private" / "OCRealWeaponFallbackSubsystem.cpp")
palette_h = read(SRC / "Public" / "OCWeaponPalettePass37Subsystem.h")
palette = read(SRC / "Private" / "OCWeaponPalettePass37Subsystem.cpp")
try_vehicle = read(P / "TRY_PRODUCTION_VEHICLES_UE58.cmd")
vehicle_import_cmd = read(P / "IMPORT_PRODUCTION_VEHICLES_UE58.cmd")
vehicle_import_py = read(P / "Scripts" / "import_production_vehicle_assets.py")
source_recovery = read(P / "Scripts" / "prepare_local_production_sources.ps1")
vehicle_fresh = read(P / "Scripts" / "verify_production_vehicle_fresh_load.py")
weapon_preflight = read(P / "Scripts" / "verify_required_weapon_assets.py")
acceptance = read(ROOT / "RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd")

reference = ROOT / "REFERENCE_PHOTOS" / "map_extent" / "oster_central_playable_area_20260824.jpg"
manifest = ROOT / "REFERENCE_PHOTOS" / "map_extent" / "README.md"
if not reference.is_file() or reference.stat().st_size <= 0:
    raise SystemExit("PASS44 VERIFY FAIL: compact central Oster reference image is missing/empty")
if not manifest.is_file():
    raise SystemExit("PASS44 VERIFY FAIL: map extent reference manifest is missing")

for needle in (
    "## Authority / conflict resolution",
    "Mandatory stale-rule retirement",
    "No compatibility resurrection",
    "Playable-map size is user-authoritative",
    "Museum BASE means actual pawn placement",
    "Runtime content truth is fail-visible",
    "Normal local game must not silently auto-fill",
    "Verifier truth follows current behavior",
):
    require(agents, needle, "root conflict policy")

for needle in (
    "Active correction branch: `fix/runtime-map-spawn-fps-assets-pass-44-20260824`",
    "Latest authoritative user runtime",
    "120 FPS",
    "about 4",
    "Pass 44 stale-rule retirement",
):
    require(ledger, needle, "current work ledger")

for needle in (
    "GlobalDefaultGameMode=/Script/OsterConflict.OCGameModeRuntimeSafe",
    "GlobalDefaultServerGameMode=/Script/OsterConflict.OCGameModeRuntimeSafe",
):
    require(engine, needle, "runtime-safe GameMode route")

for needle in (
    "int32 TargetPopulation = 0",
    "bool bAutoFillBots = false",
):
    require(game_h, needle, "safe local population defaults")

for needle in (
    "AOCGameModeRuntimeSafe",
    "PASS44_LOCAL_BOT_AUTOFILL_DISABLED_READY",
    "background_ai_load=0",
    "GetRequestedDeploymentSpawn() != FName(TEXT(\"BASE\"))",
    "MaxMuseumBaseDistanceCm = 4500.0f",
    "RestartPlayerAtTransform(NewPlayer, SpawnTransform)",
    "PASS44_ACTUAL_PAWN_MUSEUM_BASE_READY",
    "PASS44_ACTUAL_PAWN_MUSEUM_BASE_FAIL",
):
    require(runtime_h + runtime, needle, "actual Museum pawn + bot suppression")

for needle in (
    "MinPlayableX = -78000.0f",
    "MaxPlayableX =  18000.0f",
    "MinPlayableY = -12000.0f",
    "MaxPlayableY =  82000.0f",
    "RemoveInstance(Index)",
    "PASS44_COMPACT_PLAYABLE_AREA_READY",
    "legacy_2400m_ground=0",
    "reference=oster_central_playable_area_20260824",
):
    require(central_h + central, needle, "compact playable area")

for needle in (
    "Pass 44: the old projection was resolved before the central-playable-area trim",
    "return ResolveWorldMapSource() && CaptureWorldMap();",
):
    require(tactical_h, needle, "tactical map reframe after trim")

for needle in (
    'AuthoredMaterialGapTag(TEXT("OC_WeaponAuthoredMaterialGap"))',
    "PASS44_WEAPON_AUTHORED_MATERIAL_GAP",
    "PASS44_WEAPON_AUTHORED_MATERIAL_READY",
    "PASS44_WEAPON_RACK_AUTHORED_MATERIAL_GAP",
    "basicshape_repair=0",
    "reason=material_gap_audited",
):
    require(weapon_h + weapon, needle, "truth-only authored material audit")
for forbidden in (
    "MaterialRecoveryBase",
    "UMaterialInstanceDynamic::Create",
    "Component->SetMaterial(Slot",
):
    forbid(weapon_h + weapon, forbidden, "old grey material repair")

for needle in (
    "compatibility shell",
    "PASS44_WEAPON_PALETTE_MUTATION_DISABLED",
    "runtime_material_creation=0",
    "set_material_calls=0",
    "polling=0",
):
    require(palette_h + palette, needle, "retired palette mutation")
for forbidden in (
    "BasicShapeMaterial.BasicShapeMaterial",
    "UMaterialInstanceDynamic::Create",
    "SetMaterial(",
    "SetTimer(",
):
    forbid(palette_h + palette, forbidden, "palette subsystem must be inert")

for needle in (
    "підключаю кожну доступну модель незалежно",
    "HMMWV_READY=0",
    "M2_READY=0",
    "BTR_READY=0",
    "Partial production intake only",
    "missing models are NOT production-ready",
):
    require(try_vehicle, needle, "independent vehicle intake reporting")

for needle in (
    'set "HMMWV_IMPORTED=0"',
    'set "M2_IMPORTED=0"',
    'set "BTR_IMPORTED=0"',
    "Continuing independent intake for any available source files",
    "CONTENT GAP: BTR-4 production source/import is still unavailable",
):
    require(vehicle_import_cmd, needle, "independent production vehicle import command")

for needle in (
    'attempt("HMMWV"',
    'attempt("M2"',
    'attempt("BTR4"',
    "other independent assets will continue",
    "CONTENT_GAP=",
):
    require(vehicle_import_py, needle, "independent production import implementation")

for needle in (
    "Find-BtrFbxInNamedArchive",
    "BTR-labelled archive contains generic FBX",
    "source.fbx/model.fbx/untitled.fbx",
    "CONTENT GAP",
):
    require(source_recovery, needle, "broader BTR archive recovery")

for needle in (
    "AUTHORED_MATERIALS_READY",
    "authored_materials_ready",
    "defaultmaterial",
    "basicshapematerial",
    "placeholder_slots",
):
    require(vehicle_fresh, needle, "vehicle fresh-load authored material truth")

for needle in (
    "MESH_RESULT=",
    "MATERIAL_RESULT=",
    "AUTHORED_MATERIAL_SUMMARY=",
    "AUTHORED MATERIAL GAP",
    "grey/default slots are NOT production-ready",
):
    require(weapon_preflight, needle, "weapon preflight material truth")

for marker in (
    "PASS44_LOCAL_BOT_AUTOFILL_DISABLED_READY",
    "PASS44_COMPACT_PLAYABLE_AREA_READY",
    "PASS44_ACTUAL_PAWN_MUSEUM_BASE_READY",
    "PASS44_WEAPON_PALETTE_MUTATION_DISABLED",
    "PASS44_WEAPON_RACK_AUTHORED_MATERIAL_GAP",
    "PASS14_PERF_30FPS_READY",
):
    require(acceptance, marker, f"Pass 44 runtime acceptance marker {marker}")

print("RUNTIME MAP / SPAWN / FPS / ASSETS PASS 44 SOURCE CONTRACT PASS")
print("- root authority now retires stale conflicting rules/verifiers instead of resurrecting regressions")
print("- actual human BASE pawn is verified within 45 m of MuseumAnchor")
print("- implicit local bot fill is disabled; bots remain explicit opt-in")
print("- central Oster playable/tactical bounds follow the saved user map reference")
print("- weapon BasicShape/palette mutation is retired and authored-material gaps remain fail-visible")
print("- HMMWV/M2/BTR intake is independent and missing BTR no longer blocks available models")
print("STATUS: CODED_UNTESTED; UE 5.8 runtime and user visual acceptance remain authoritative")
