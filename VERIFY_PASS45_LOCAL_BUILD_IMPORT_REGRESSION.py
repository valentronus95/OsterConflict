#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
TACTICAL = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCTacticalMapVisual.cpp"
IMPORTER = ROOT / "OsterConflict" / "Scripts" / "import_production_vehicle_assets.py"
AGGREGATE_IMPORTER = ROOT / "OsterConflict" / "Scripts" / "import_all_project_assets.py"
ASSET_STATUS_COLLECTOR = ROOT / "COLLECT_LOCAL_ASSET_STATUS.py"
RUNTIME_EVIDENCE = ROOT / "VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py"
START_HERE = ROOT / "START_HERE.cmd"
BATCH_CMD = ROOT / "RUN_PASS45_BATCH_RUNTIME_TEST.cmd"
BATCH_PS1 = ROOT / "OsterConflict" / "Scripts" / "run_pass45_batch_runtime_test.ps1"
LEDGER = ROOT / "OSTER_CONFLICT_WORK_LEDGER.md"

errors = []


def read(path: Path) -> str:
    if not path.is_file():
        errors.append(f"missing file: {path.relative_to(ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def require(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


tactical = read(TACTICAL)
importer = read(IMPORTER)
aggregate = read(AGGREGATE_IMPORTER)
collector = read(ASSET_STATUS_COLLECTOR)
evidence = read(RUNTIME_EVIDENCE)
start = read(START_HERE)
batch_cmd = read(BATCH_CMD)
batch = read(BATCH_PS1)
ledger = read(LEDGER)

require("const FPass45ReferenceRoadSegment Pass45ReferenceRoads[]" in tactical, "Pass45 road table is not a normal const table")
require("constexpr FPass45ReferenceRoadSegment Pass45ReferenceRoads[]" not in tactical, "UE 5.8 C2131 constexpr FVector2D regression returned")
require("UE_ARRAY_COUNT(Pass45ReferenceRoads)" in tactical, "road-table count contract was lost")

require('common_meshes.set_editor_property("convert_statics_with_animated_transform_to_skeletals", False)' in importer, "current UE 5.8 static-mesh animated-transform policy is missing")
require('set_editor_property("auto_detect_mesh_type"' not in importer, "deprecated UE 5.8 auto_detect_mesh_type property returned")
require('common_meshes.set_editor_property("force_all_mesh_as_type", force_mesh_type.IFMT_STATIC_MESH)' in importer, "HMMWV/M2 intake no longer forces StaticMesh")
require('mesh_pipeline.set_editor_property("import_skeletal_meshes", False)' in importer, "HMMWV/M2 intake no longer disables skeletal import")

for marker in (
    "def _run_required_ingest(", "production.SUCCESS_SENTINEL", "production_weapons.SUCCESS_SENTINEL",
    'bindings.setdefault("unbound_models", []).extend(required_ingest_failures)',
    '"required_production_ingest_failures": len(required_ingest_failures)', "aggregate asset PASS is blocked",
):
    require(marker in aggregate, f"aggregate importer lost fail-closed marker {marker}")

for marker in (
    "def collect_snapshot(", "runtime_bindings.json", "LOCAL_ASSET_STATUS.json", "LOCAL_ASSET_STATUS.txt",
    "IMPORT_RESULT_CODE=", "RUNTIME_RESULT_CODE=", "RUNTIME_SCOPE=", 'runtime_scope = "IMPORT_ONLY"',
    'runtime_scope = "CURRENT_RUN_FAILED"', 'runtime_scope = "CURRENT_RUN_COMPLETED"', "PENDING_MANUAL_OBSERVATION",
):
    require(marker in collector, f"asset status collector lost {marker}")
require("asset_status.collect_snapshot" in evidence, "runtime evidence no longer refreshes consolidated asset snapshot")
require("FAIL=LOCAL_ASSET_STATUS snapshot write failed" in evidence, "runtime evidence no longer fails closed on snapshot write failure")

for marker in (
    'set "BATCH_RUNTIME=%~dp0RUN_PASS45_BATCH_RUNTIME_TEST.cmd"',
    '2. ПОВНИЙ RUNTIME-ТЕСТ ^(ПАКЕТНИЙ^)', 'call "%BATCH_RUNTIME%"',
    'Пункт 1: тільки incremental C++ build + запуск гри. Без strict reimport/fresh-load підготовки.',
):
    require(marker in start, f"START_HERE packet route lost {marker}")
require(":full_runtime_test" not in start, "stale in-file full runtime owner returned")
require('call :ingest_all_assets' not in start, "normal launcher again forces strict asset ingest before ordinary game/editor")

require('powershell -NoProfile -ExecutionPolicy Bypass -File "%RUNNER%"' in batch_cmd, "batch wrapper no longer calls PowerShell runner directly")
for marker in (
    'IMPORT_ALL_LOCAL_INBOX_UE58.cmd', 'RUN_PASS45_STRICT_MATERIAL_GATE.cmd', 'RUN_R14_CURRENT_GAMEPLAY.cmd',
    'Final OsterConflictEditor C++ build', 'PASS45_BATCH_RUNTIME_REPORT.txt', 'Tracked Changes:',
    'FORMAL_ACCEPTANCE=BLOCKED_DIRTY_OR_NONEXACT_SOURCE', '$env:OC_FORCE_ACCEPTANCE = "0"',
    '$env:OC_FORCE_ACCEPTANCE = "1"', 'Single gameplay runtime', 'FAILURE TAILS:',
):
    require(marker in batch, f"packet runtime lost {marker}")
require('& $Command @Arguments' in batch, "packet runner no longer executes paths as direct PowerShell command arguments")
require('("-Project=" + $UProject)' in batch, "Build.bat project argument is not passed as one argument")

for stale in (
    'PASS45_REIMPORT_STEIN_WEAPON_MATERIALS_UE58.cmd', 'PASS45_IMPORT_MANUAL_ACTION_AUDIO_UE58.cmd',
    'PASS45_IMPORT_REMINGTON870_PRODUCTION_UE58.cmd',
):
    require(stale not in batch, f"stale per-weapon packet stage returned: {stale}")
require('\\"C:' not in batch, "literal backslash-escaped Windows command quote returned")
require('cmd /c' not in batch.lower(), "packet runner returned to fragile cmd /c command-string dispatch")

require("LOCAL UE BUILD REJECTED" in ledger, "ledger lost factual local UE build rejection")
require("C2131" in ledger and "auto_detect_mesh_type" in ledger, "ledger lost observed C2131/Interchange regressions")
require("CODED_UNTESTED" in ledger, "source correction was promoted before local UE runtime")

if errors:
    print("PASS45 LOCAL BUILD/IMPORT REGRESSION: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 LOCAL BUILD/IMPORT REGRESSION: PASS")
print("- packet runtime dispatch uses direct PowerShell invocation instead of literal escaped quote command strings")
print("- normal game/editor no longer force strict asset reimport")
print("- one aggregate asset ingest replaces stale Stein/audio/Remington wrapper fan-out")
print("- dirty/non-exact source remains diagnostic-only and cannot become formal acceptance")
print("- UE 5.8 C2131 and Interchange regressions remain guarded")
print("- aggregate import, status snapshot and runtime evidence remain fail-closed")
print("STATUS: SOURCE VERIFIED ONLY; local Windows UE 5.8 execution is still required")
