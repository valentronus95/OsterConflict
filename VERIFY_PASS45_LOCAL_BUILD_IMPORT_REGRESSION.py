#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
TACTICAL = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCTacticalMapVisual.cpp"
IMPORTER = ROOT / "OsterConflict" / "Scripts" / "import_production_vehicle_assets.py"
AGGREGATE_IMPORTER = ROOT / "OsterConflict" / "Scripts" / "import_all_project_assets.py"
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
print("- factual local build rejection remains recorded; fix is CODED_UNTESTED")
