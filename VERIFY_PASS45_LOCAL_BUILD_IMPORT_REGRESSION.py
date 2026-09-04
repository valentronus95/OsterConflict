#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
TACTICAL = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCTacticalMapVisual.cpp"
IMPORTER = ROOT / "OsterConflict" / "Scripts" / "import_production_vehicle_assets.py"
AGGREGATE_IMPORTER = ROOT / "OsterConflict" / "Scripts" / "import_all_project_assets.py"
ASSET_STATUS_COLLECTOR = ROOT / "COLLECT_LOCAL_ASSET_STATUS.py"
RUNTIME_EVIDENCE = ROOT / "VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py"
START_HERE = ROOT / "START_HERE.cmd"
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
aggregate_importer = read(AGGREGATE_IMPORTER)
asset_status_collector = read(ASSET_STATUS_COLLECTOR)
runtime_evidence = read(RUNTIME_EVIDENCE)
start_here = read(START_HERE)
ledger = read(LEDGER)

# Local UE 5.8.1 / MSVC 14.51 factual build rejected the FVector2D table when it was constexpr.
require(
    "const FPass45ReferenceRoadSegment Pass45ReferenceRoads[]" in tactical,
    "Pass45 road table is not a normal const table",
)
require(
    "constexpr FPass45ReferenceRoadSegment Pass45ReferenceRoads[]" not in tactical,
    "UE 5.8 C2131 regression returned: FVector2D road table must not be constexpr",
)
require(
    "UE_ARRAY_COUNT(Pass45ReferenceRoads)" in tactical,
    "road-table count contract was lost while fixing C2131",
)

# Local UE 5.8 Interchange rejected the deprecated bAutoDetectMeshType property for GLB HMMWV/M2 intake.
require(
    'common_meshes.set_editor_property("convert_statics_with_animated_transform_to_skeletals", False)' in importer,
    "current UE 5.8 static-mesh animated-transform policy is missing",
)
require(
    'set_editor_property("auto_detect_mesh_type"' not in importer,
    "deprecated UE 5.8 auto_detect_mesh_type property returned",
)
require(
    'common_meshes.set_editor_property("force_all_mesh_as_type", force_mesh_type.IFMT_STATIC_MESH)' in importer,
    "HMMWV/M2 GLB intake no longer explicitly forces StaticMesh",
)
require(
    'mesh_pipeline.set_editor_property("import_skeletal_meshes", False)' in importer,
    "HMMWV/M2 GLB intake no longer explicitly disables skeletal import",
)

# Aggregate asset intake may keep cataloging unrelated packs after a production gap, but it must
# never mint the global success sentinel unless vehicle + exact-weapon sub-importers reported PASS.
require(
    "def _run_required_ingest(" in aggregate_importer,
    "aggregate importer no longer records required production ingest results",
)
require(
    'if SUCCESS.exists():' in aggregate_importer and 'SUCCESS.unlink()' in aggregate_importer,
    "aggregate importer does not clear the previous global PASS marker before a fresh run",
)
require(
    "production.SUCCESS_SENTINEL" in aggregate_importer,
    "aggregate importer does not gate on production vehicle STATUS sentinel",
)
require(
    "if sentinel.exists():" in aggregate_importer and "sentinel.unlink()" in aggregate_importer,
    "aggregate importer does not clear stale production PASS sentinels before a fresh ingest attempt",
)
require(
    "production_weapons.SUCCESS_SENTINEL" in aggregate_importer,
    "aggregate importer does not gate on exact production weapon STATUS sentinel",
)
require(
    'bindings.setdefault("unbound_models", []).extend(required_ingest_failures)' in aggregate_importer,
    "required production GAPs are not preserved as aggregate unbound failures",
)
require(
    '"required_production_ingest_failures": len(required_ingest_failures)' in aggregate_importer,
    "aggregate binding summary lost required production failure accounting",
)
require(
    "aggregate asset PASS is blocked" in aggregate_importer,
    "aggregate importer no longer makes production GAP status fail-visible",
)

# One local snapshot must consolidate the otherwise scattered Saved/ and Logs evidence. Import and
# every early full-runtime failure must refresh the same snapshot with exact exit codes. Import-only
# snapshots must never reuse stale runtime/material/evidence PASS markers from an older launch.
for marker in (
    "def collect_snapshot(",
    "runtime_bindings.json",
    "production_import_success.txt",
    "production_weapon_import_result.txt",
    "local_inbox_runtime_validation.txt",
    "local_world_runtime_validation.txt",
    "LOCAL_ASSET_STATUS.json",
    "LOCAL_ASSET_STATUS.txt",
    "IMPORT_RESULT_CODE=",
    "RUNTIME_RESULT_CODE=",
    "RUNTIME_SCOPE=",
    '"import_result_code": import_result',
    '"runtime_result_code": runtime_result',
    '"runtime_scope": runtime_scope',
    'import_stage = "FAIL"',
    'runtime_stage = "FAIL"',
    'runtime_scope = "IMPORT_ONLY"',
    'runtime_scope = "CURRENT_RUN_FAILED"',
    'runtime_scope = "CURRENT_RUN_COMPLETED"',
    'runtime_stage = "PENDING_CURRENT_RUN"',
    'material_stage = "PENDING_CURRENT_RUN"',
    'evidence_stage = "PENDING_CURRENT_RUN"',
    "PASS45_ASSET_IMPORT_RC",
    "PASS45_RUNTIME_RC",
    "PENDING_MANUAL_OBSERVATION",
):
    require(marker in asset_status_collector, f"local asset status collector lost {marker}")
require(
    "if runtime_result is None:" in asset_status_collector,
    "collector no longer gates runtime interpretation on a current explicit runtime result",
)
require(
    asset_status_collector.index('if runtime_result is None:') < asset_status_collector.index('runtime_scope = "IMPORT_ONLY"'),
    "import-only runtime freshness gate is malformed",
)
require(
    "import COLLECT_LOCAL_ASSET_STATUS as asset_status" in runtime_evidence,
    "canonical runtime evidence verifier no longer imports the local asset status collector",
)
require(
    "asset_status.collect_snapshot" in runtime_evidence,
    "canonical runtime evidence verifier no longer writes the consolidated local asset snapshot",
)
require(
    "import_result=0" in runtime_evidence,
    "final runtime evidence snapshot no longer preserves the already-proven successful import result",
)
require(
    "def write_asset_snapshot(source_sha: str, runtime_result: int) -> bool:" in runtime_evidence,
    "runtime evidence verifier no longer reports whether final LOCAL_ASSET_STATUS was actually written",
)
require(
    "if not write_asset_snapshot(source_sha, 0):" in runtime_evidence,
    "runtime evidence PASS is no longer fail-closed on a missing final LOCAL_ASSET_STATUS snapshot",
)
require(
    "FAIL=LOCAL_ASSET_STATUS snapshot write failed" in runtime_evidence,
    "runtime evidence verifier no longer records the snapshot-write failure in canonical evidence",
)
require(
    "return 2" in runtime_evidence,
    "runtime evidence verifier no longer returns nonzero when final LOCAL_ASSET_STATUS cannot be written",
)

# Asset import itself must be tied to the exact current remote head before Unreal is allowed to mutate
# Content/Saved. Otherwise a stale local branch could produce a fresh-looking snapshot for old code.
for marker in (
    ":verify_current_asset_source",
    'set "CURRENT_ASSET_BRANCH="',
    'set "LOCAL_ASSET_HEAD="',
    'set "REMOTE_ASSET_HEAD="',
    'git -C "%~dp0" fetch origin "%CURRENT_ASSET_BRANCH%"',
    'git -C "%~dp0" rev-parse HEAD',
    'git -C "%~dp0" rev-parse "origin/%CURRENT_ASSET_BRANCH%"',
    'if /I not "%LOCAL_ASSET_HEAD%"=="%REMOTE_ASSET_HEAD%"',
    "UE import на застарілому HEAD не запускається",
    "call :verify_current_asset_source",
):
    require(marker in start_here, f"START_HERE exact-head asset precheck lost {marker}")
for exit_code in ("exit /b 66", "exit /b 67", "exit /b 68", "exit /b 69", "exit /b 70"):
    require(exit_code in start_here, f"START_HERE lost exact-head asset precheck code {exit_code}")
require(
    start_here.index("call :verify_current_asset_source") < start_here.index('if not exist "%ALL_ASSET_IMPORT%"') < start_here.index('call "%ALL_ASSET_IMPORT%"'),
    "START_HERE must verify exact remote head before any asset importer execution",
)

for marker in (
    'set "ASSET_STATUS_COLLECTOR=%~dp0COLLECT_LOCAL_ASSET_STATUS.py"',
    'set "ASSET_STATUS_TEXT=%~dp0OsterConflict\\Saved\\AssetStatus\\LOCAL_ASSET_STATUS.txt"',
    'set "ASSET_STATUS_JSON=%~dp0OsterConflict\\Saved\\AssetStatus\\LOCAL_ASSET_STATUS.json"',
    'set "ASSET_RC=%ERRORLEVEL%"',
    "call :write_asset_snapshot",
    'set "SNAPSHOT_RC=%ERRORLEVEL%"',
    ":write_asset_snapshot",
    'if defined ASSET_RC set "PASS45_ASSET_IMPORT_RC=%ASSET_RC%"',
    'if defined RUNTIME_RC set "PASS45_RUNTIME_RC=%RUNTIME_RC%"',
    'if exist "%ASSET_STATUS_TEXT%" del /q "%ASSET_STATUS_TEXT%"',
    'if exist "%ASSET_STATUS_JSON%" del /q "%ASSET_STATUS_JSON%"',
    '%ASSET_PY_CMD% "%ASSET_STATUS_COLLECTOR%"',
    'set "PASS45_ASSET_IMPORT_RC="',
    'set "PASS45_RUNTIME_RC="',
    'if not "%SNAPSHOT_RC%"=="0"',
    "[ASSET STATUS] Import snapshot:",
    "[ASSET STATUS] Runtime snapshot:",
):
    require(marker in start_here, f"START_HERE asset snapshot route lost {marker}")
require(
    start_here.index('set "ASSET_RC=%ERRORLEVEL%"') < start_here.index("call :write_asset_snapshot") < start_here.index('set "SNAPSHOT_RC=%ERRORLEVEL%"') < start_here.index('if not "%ASSET_RC%"=="0"'),
    "START_HERE must snapshot the import result and capture snapshot status before branching on ASSET_RC",
)
require(
    start_here.index('if defined ASSET_RC set "PASS45_ASSET_IMPORT_RC=%ASSET_RC%"') < start_here.index('%ASSET_PY_CMD% "%ASSET_STATUS_COLLECTOR%"') < start_here.index('set "PASS45_ASSET_IMPORT_RC="'),
    "START_HERE must expose the exact import exit code only while the collector runs",
)
require(
    start_here.index('if defined RUNTIME_RC set "PASS45_RUNTIME_RC=%RUNTIME_RC%"') < start_here.index('%ASSET_PY_CMD% "%ASSET_STATUS_COLLECTOR%"') < start_here.index('set "PASS45_RUNTIME_RC="'),
    "START_HERE must expose the exact runtime exit code only while the collector runs",
)
require(
    start_here.index('if exist "%ASSET_STATUS_TEXT%" del /q "%ASSET_STATUS_TEXT%"') < start_here.index('%ASSET_PY_CMD% "%ASSET_STATUS_COLLECTOR%"'),
    "START_HERE must delete the old text snapshot before collecting a fresh one",
)
require(
    start_here.index('if exist "%ASSET_STATUS_JSON%" del /q "%ASSET_STATUS_JSON%"') < start_here.index('%ASSET_PY_CMD% "%ASSET_STATUS_COLLECTOR%"'),
    "START_HERE must delete the old JSON snapshot before collecting a fresh one",
)
for exit_code in ("exit /b 62", "exit /b 63", "exit /b 64", "exit /b 65"):
    require(exit_code in start_here, f"START_HERE lost fail-closed asset snapshot code {exit_code}")
require(
    'if not exist "%ASSET_STATUS_TEXT%" (' in start_here and 'if not exist "%ASSET_STATUS_JSON%" (' in start_here,
    "START_HERE no longer verifies that both fresh LOCAL_ASSET_STATUS outputs exist",
)
require(
    start_here.index('if not "%ASSET_RC%"=="0"') < start_here.index('if not "%SNAPSHOT_RC%"=="0"') < start_here.index("[ASSET STATUS] Import snapshot:"),
    "successful import may bypass the fail-closed snapshot result",
)
require(
    start_here.count("call :write_asset_snapshot") >= 9,
    "START_HERE no longer snapshots all early full-runtime failure exits",
)
for runtime_code in ('set "RUNTIME_RC=35"', 'set "RUNTIME_RC=36"', 'set "RUNTIME_RC=37"', 'set "RUNTIME_RC=38"', 'set "RUNTIME_RC=30"'):
    require(runtime_code in start_here, f"START_HERE lost explicit runtime failure code {runtime_code}")
require(
    'set "RUNTIME_RC=%GAME_RC%"' in start_here and 'set "RUNTIME_RC=%MATERIAL_RC%"' in start_here and 'set "RUNTIME_RC=%EVIDENCE_RC%"' in start_here,
    "START_HERE lost dynamic gameplay/material/evidence runtime failure codes",
)

# Status must remain factual: source fix exists, but a later local build/import must verify it.
require(
    "LOCAL UE BUILD REJECTED" in ledger,
    "ledger does not preserve the factual 2026-08-25 local UE build rejection",
)
require(
    "C2131" in ledger and "auto_detect_mesh_type" in ledger,
    "ledger does not name both observed regressions",
)
require(
    "CODED_UNTESTED" in ledger,
    "ledger must not promote the new build/import fix to VERIFIED before a later local UE run",
)

if errors:
    print("PASS45 LOCAL BUILD/IMPORT REGRESSION: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 LOCAL BUILD/IMPORT REGRESSION: PASS")
print("- UE 5.8 FVector2D tactical-road table is no longer constexpr")
print("- HMMWV/M2 Interchange intake uses the current UE 5.8 static-mesh policy")
print("- deprecated auto_detect_mesh_type cannot silently return")
print("- aggregate asset PASS is blocked by fresh vehicle/exact-weapon GAP sentinels")
print("- stale aggregate PASS is cleared before a fresh import run")
print("- asset ingest is blocked unless local HEAD exactly matches fetched origin/current-branch")
print("- import-only LOCAL_ASSET_STATUS cannot reuse stale runtime/material/evidence PASS")
print("- stale LOCAL_ASSET_STATUS files are deleted before every fresh collection")
print("- successful import fails closed unless fresh LOCAL_ASSET_STATUS txt/json both exist")
print("- final runtime snapshot preserves IMPORT_RESULT_CODE=0 plus exact RUNTIME_RESULT_CODE")
print("- final runtime PASS fails closed if LOCAL_ASSET_STATUS cannot be written")
print("- every early full-runtime failure refreshes LOCAL_ASSET_STATUS with exact RUNTIME_RESULT_CODE")
print("- canonical runtime evidence still refreshes the consolidated LOCAL_ASSET_STATUS snapshot")
print("- factual local build rejection remains recorded; fix is CODED_UNTESTED")
