#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
FINALIZER = ROOT / "OsterConflict" / "Scripts" / "finalize_asset_acceptance.py"
BASE_IMPORTER = ROOT / "OsterConflict" / "Scripts" / "import_all_local_inbox_assets.py"
WEAPON_NORMALIZER = ROOT / "OsterConflict" / "Scripts" / "normalize_local_weapon_categories.py"
ENTRYPOINT = ROOT / "START_HERE.cmd"
COLLECTOR = ROOT / "COLLECT_LOCAL_ASSET_STATUS.py"
IMPORT_CMD = ROOT / "OsterConflict" / "IMPORT_ALL_LOCAL_INBOX_UE58.cmd"
LEGACY_FINALIZE_CMD = ROOT / "FINALIZE_ASSET_ACCEPTANCE_AND_CLEANUP.cmd"

errors: list[str] = []


def read(path: Path) -> str:
    if not path.is_file():
        errors.append(f"missing file: {path.relative_to(ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def require(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


finalizer = read(FINALIZER)
base_importer = read(BASE_IMPORTER)
weapon_normalizer = read(WEAPON_NORMALIZER)
entrypoint = read(ENTRYPOINT)
collector = read(COLLECTOR)
import_cmd = read(IMPORT_CMD)

for marker in (
    'preflight_only = "--preflight" in args',
    'accept_visual = "--accept-visual" in args',
    'preflight_only == accept_visual',
    'git_output("rev-parse", "HEAD")',
    "verify_exact_remote_head(head)",
    'git_output("fetch", "origin", branch)',
    'git_output("rev-parse", f"origin/{branch}")',
    "verify_clean_acceptance_source()",
    "verify_current_automated_status(current, head)",
    'status.get("schema") != "oster-conflict-local-asset-status-v4"',
    'status.get("import_result_code") != 0',
    'status.get("runtime_result_code") != 0',
    'status.get("runtime_scope") != "CURRENT_RUN_COMPLETED"',
    '"local_ue_import"',
    '"live_runtime_hookup"',
    '"strict_material_gate"',
    '"automated_runtime_evidence"',
    'production.get("vehicles_status") != "PASS"',
    'production.get("weapons_status") != "PASS"',
    'bindings.get("all_models_bound")',
    'bindings.get("unbound")',
    'summary.get("unbound_models")',
    'source_status_counts.get("UNBOUND")',
    "source_status_counts still contains explicit UNBOUND rows",
    'category_counts.get("M16_M4")',
    "M16/M4 production content gap is still open",
    'prepared_status not in {"PASS", "NO_INBOX"}',
    'status != "EXTRACTED"',
    'p.suffix.lower() == ".zip"',
    "sha256(path)",
    "digest not in accepted_hashes",
    "source ZIP cleanup refused",
    "VISUAL_CHECKLIST",
    "write_manual_acceptance(head)",
    "path.unlink()",
    'write_cleanup_report(head, "PASS", deleted)',
    "refresh_consolidated_status(head)",
):
    require(marker in finalizer, f"finalizer lost fail-closed marker: {marker}")

for marker in (
    "Fail closed on every source row the importer itself explicitly classified as UNBOUND",
    'if str(status.get("status") or "").upper() != "UNBOUND":',
    'bindings["unbound_models"].append(status)',
    'bindings["all_models_bound"] = len(bindings["unbound_models"]) == 0',
):
    require(marker in base_importer, f"base importer lost explicit-UNBOUND fail-closed marker: {marker}")

unbound_guard_pos = base_importer.find("Fail closed on every source row")
all_bound_pos = base_importer.find('bindings["all_models_bound"] = len(bindings["unbound_models"]) == 0')
require(
    -1 not in (unbound_guard_pos, all_bound_pos) and unbound_guard_pos < all_bound_pos,
    "base importer may compute all_models_bound before explicit source_status UNBOUND rows are promoted",
)

for marker in (
    "A factual import/load failure",
    'row_status = str(row.get("status") or "").upper()',
    'if row_status == "UNBOUND":',
    "Independent reconciliation: every source_status row still marked UNBOUND after normalization",
    'if str(row.get("status") or "").upper() != "UNBOUND":',
    'data["all_models_bound"] = len(retained) == 0',
):
    require(marker in weapon_normalizer, f"weapon normalizer lost factual-UNBOUND guard: {marker}")

status_guard_pos = weapon_normalizer.find('if row_status == "UNBOUND":')
source_promote_pos = weapon_normalizer.find("if source in resolved_sources:", status_guard_pos)
reconcile_pos = weapon_normalizer.find("Independent reconciliation: every source_status row still marked UNBOUND")
normalizer_all_bound_pos = weapon_normalizer.find('data["all_models_bound"] = len(retained) == 0')
require(
    -1 not in (status_guard_pos, source_promote_pos) and status_guard_pos < source_promote_pos,
    "weapon normalizer may upgrade factual UNBOUND rows based only on filename/category mapping",
)
require(
    -1 not in (reconcile_pos, normalizer_all_bound_pos) and reconcile_pos < normalizer_all_bound_pos,
    "weapon normalizer may compute all_models_bound before reconciling explicit UNBOUND source rows",
)

main_pos = finalizer.find("def main() -> int:")
if main_pos < 0:
    errors.append("finalizer main() is missing")
else:
    call_positions = [
        finalizer.find("run_preflight()", main_pos),
        finalizer.find("if preflight_only:", main_pos),
        finalizer.find("write_manual_acceptance(head)", main_pos),
        finalizer.find("path.unlink()", main_pos),
    ]
    require(
        all(pos >= 0 for pos in call_positions) and call_positions == sorted(call_positions),
        "finalizer can mutate acceptance/ZIP state before preflight completes",
    )

schema_pos = finalizer.find('status.get("schema") != "oster-conflict-local-asset-status-v4"')
source_pos = finalizer.find('source_sha = str(status.get("source_sha") or "")')
import_code_pos = finalizer.find('status.get("import_result_code") != 0')
runtime_code_pos = finalizer.find('status.get("runtime_result_code") != 0')
runtime_scope_pos = finalizer.find('status.get("runtime_scope") != "CURRENT_RUN_COMPLETED"')
source_status_pos = finalizer.find('source_status_counts.get("UNBOUND")')
m16_pos = finalizer.find('category_counts.get("M16_M4")')
require(
    -1 not in (schema_pos, source_pos, import_code_pos, runtime_code_pos, runtime_scope_pos, source_status_pos, m16_pos)
    and schema_pos < source_pos < import_code_pos < runtime_code_pos < runtime_scope_pos < source_status_pos < m16_pos,
    "finalizer lost strict current-schema/result/unbound ordering before content finalization",
)

require(
    "shutil.rmtree" not in finalizer and "os.remove(INBOX" not in finalizer,
    "finalizer must not recursively delete the inbox",
)
require(
    'relative(path)' in finalizer and 'sha256' in finalizer,
    "cleanup report must preserve exact path/hash evidence",
)

for marker in (
    "Єдиний користувацький launcher/test entrypoint: START_HERE.cmd",
    'set "ASSET_FINALIZER=%~dp0OsterConflict\\Scripts\\finalize_asset_acceptance.py"',
    '"%ASSET_FINALIZER%" --preflight',
    "FINALIZE PENDING",
    "ZIP не видалялись",
    "choice /C YN",
    '"%ASSET_FINALIZER%" --accept-visual',
    "PASS45 FULL ASSET LIFECYCLE ACCEPTED",
    "SOURCE ZIP CLEANUP: PASS",
):
    require(marker in entrypoint, f"START_HERE finalization route lost marker: {marker}")

preflight_pos = entrypoint.find('"%ASSET_FINALIZER%" --preflight')
choice_pos = entrypoint.find('choice /C YN', preflight_pos)
accept_pos = entrypoint.find('"%ASSET_FINALIZER%" --accept-visual', choice_pos)
require(
    -1 not in (preflight_pos, choice_pos, accept_pos) and preflight_pos < choice_pos < accept_pos,
    "START_HERE must pass non-destructive preflight before asking for manual visual acceptance or cleanup",
)
require(
    not LEGACY_FINALIZE_CMD.exists(),
    "second user-facing FINALIZE_ASSET_ACCEPTANCE_AND_CLEANUP.cmd returned; START_HERE must stay the single entrypoint",
)

for marker in (
    '"oster-conflict-local-asset-status-v4"',
    '"direct_visual_acceptance": visual_stage',
    '"source_zip_cleanup": cleanup_stage',
    'MANUAL_VISUAL_JSON',
    'ZIP_CLEANUP_JSON',
    'visual_stage = "PENDING_MANUAL_OBSERVATION"',
    'visual_stage = "STALE_SOURCE"',
    'visual_stage = "PASS"',
    'cleanup_stage = "PENDING_VISUAL_ACCEPTANCE"',
    'cleanup_stage = "PASS"',
    'f"DIRECT_VISUAL_ACCEPTANCE={visual_stage}"',
    'f"SOURCE_ZIP_CLEANUP={cleanup_stage}"',
    'explicit_unbound_count = int(source_status_counts.get("UNBOUND", 0))',
    'not unbound',
    'explicit_unbound_count == 0',
    "inconsistent manifest where all_models_bound/success says green but explicit UNBOUND evidence remains",
    'source_sha_known = bool(source_sha and source_sha.lower() != "unknown")',
    'evidence_source_match = source_sha_known and _marker(texts["runtime_evidence"], f"SOURCE_SHA={source_sha}")',
    '"STALE_SOURCE" if evidence_pass',
):
    require(marker in collector, f"collector lost finalization/import/runtime fail-closed marker: {marker}")

collector_unbound_pos = collector.find('explicit_unbound_count = int(source_status_counts.get("UNBOUND", 0))')
collector_import_pass_pos = collector.find('import_stage = "PASS"')
require(
    -1 not in (collector_unbound_pos, collector_import_pass_pos) and collector_unbound_pos < collector_import_pass_pos,
    "collector may mint LOCAL_UE_IMPORT=PASS before checking explicit UNBOUND evidence",
)
evidence_pass_pos = collector.find('evidence_pass = _marker(texts["runtime_evidence"], "PASS45_RUNTIME_AUTOMATED_EVIDENCE=PASS")')
evidence_source_pos = collector.find('evidence_source_match = source_sha_known and _marker(texts["runtime_evidence"], f"SOURCE_SHA={source_sha}")')
evidence_stage_pos = collector.find('evidence_stage = (', evidence_source_pos)
require(
    -1 not in (evidence_pass_pos, evidence_source_pos, evidence_stage_pos)
    and evidence_pass_pos < evidence_source_pos < evidence_stage_pos,
    "collector may accept runtime evidence PASS without exact source-SHA attribution",
)
require(
    collector.index("automated_ready = all(") < collector.index('visual_stage = "PASS"'),
    "collector may mint manual visual PASS without current automated PASS",
)
require(
    collector.index('manual_sha.lower() != source_sha.lower()') < collector.index('visual_stage = "PASS"'),
    "collector may accept a manual record from another source SHA",
)
require(
    collector.index('cleanup_sha.lower() != source_sha.lower()') < collector.index('cleanup_stage = "PASS"'),
    "collector may accept ZIP cleanup from another source SHA",
)

for marker in (
    'MANUAL_VISUAL_ACCEPTANCE.json',
    'MANUAL_VISUAL_ACCEPTANCE.txt',
    'ACCEPTED_ZIP_CLEANUP.json',
    'ACCEPTED_ZIP_CLEANUP.txt',
    "Any fresh ingest invalidates earlier manual visual acceptance",
):
    require(marker in import_cmd, f"fresh ingest no longer invalidates finalization evidence: {marker}")

manual_delete = import_cmd.find('if exist "%MANUAL_VISUAL_JSON%" del /q')
cleanup_delete = import_cmd.find('if exist "%ZIP_CLEANUP_JSON%" del /q')
success_delete = import_cmd.find('if exist "%SUCCESS%" del /q')
require(
    -1 not in (manual_delete, cleanup_delete, success_delete)
    and manual_delete < success_delete
    and cleanup_delete < success_delete,
    "old finalization evidence must be cleared before fresh import evidence is reset/generated",
)

if errors:
    print("PASS45 ASSET FINALIZATION GUARD: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 ASSET FINALIZATION GUARD: PASS")
print("- START_HERE remains the single user-facing launcher and owns finalization after full runtime")
print("- base importer promotes every explicit source_status UNBOUND row before computing all_models_bound")
print("- weapon normalization cannot upgrade factual import/load UNBOUND rows from filename matching")
print("- weapon normalization independently reconciles all explicit UNBOUND rows before all_models_bound")
print("- collector independently blocks import PASS on unbound_models or explicit source_status UNBOUND")
print("- collector accepts automated runtime evidence only when its SOURCE_SHA matches the current snapshot")
print("- finalizer requires exact v4 schema plus explicit import/runtime result code zero")
print("- finalizer independently rejects summary/source-status UNBOUND counts")
print("- preflight is non-destructive and runs before the human visual confirmation")
print("- direct visual PASS requires explicit confirmation after exact-remote current automated PASS")
print("- M16/M4 content gap must be closed by a fresh bound payload before 100% finalization")
print("- Fab-only/no-inbox runs can finalize with zero source ZIPs after the automated gates pass")
print("- manual/cleanup records are exact-source scoped and fresh ingest invalidates them")
print("- source ZIP deletion is limited to models_game_OC archives whose SHA-256 is proven by prepared_sources")
print("- unknown/unproven ZIP blocks cleanup before any archive is deleted")
print("- consolidated LOCAL_ASSET_STATUS exposes visual acceptance and source ZIP cleanup separately")
