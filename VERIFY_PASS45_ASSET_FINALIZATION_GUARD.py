#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
FINALIZER = ROOT / "OsterConflict" / "Scripts" / "finalize_asset_acceptance.py"
ENTRYPOINT = ROOT / "FINALIZE_ASSET_ACCEPTANCE_AND_CLEANUP.cmd"
COLLECTOR = ROOT / "COLLECT_LOCAL_ASSET_STATUS.py"
IMPORT_CMD = ROOT / "OsterConflict" / "IMPORT_ALL_LOCAL_INBOX_UE58.cmd"

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
entrypoint = read(ENTRYPOINT)
collector = read(COLLECTOR)
import_cmd = read(IMPORT_CMD)

for marker in (
    '"--accept-visual" not in sys.argv[1:]',
    'git_output("rev-parse", "HEAD")',
    "verify_exact_remote_head(head)",
    'git_output("fetch", "origin", branch)',
    'git_output("rev-parse", f"origin/{branch}")',
    "verify_clean_acceptance_source()",
    "verify_current_automated_status(current, head)",
    'status.get("runtime_scope") != "CURRENT_RUN_COMPLETED"',
    '"local_ue_import"',
    '"live_runtime_hookup"',
    '"strict_material_gate"',
    '"automated_runtime_evidence"',
    'production.get("vehicles_status") != "PASS"',
    'production.get("weapons_status") != "PASS"',
    'bindings.get("all_models_bound")',
    'bindings.get("unbound")',
    'category_counts.get("M16_M4")',
    "M16/M4 production content gap is still open",
    'prepared_status not in {"PASS", "NO_INBOX"}',
    'status != "EXTRACTED"',
    'INBOX.rglob("*.zip")',
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

require(
    finalizer.index("verify_exact_remote_head(head)")
    < finalizer.index("verify_clean_acceptance_source()")
    < finalizer.index("verify_current_automated_status(current, head)")
    < finalizer.index("preflight_source_zips(accepted_hashes)")
    < finalizer.index("write_manual_acceptance(head)")
    < finalizer.index("path.unlink()"),
    "final acceptance ordering no longer proves remote/source/runtime/ZIP state before manual PASS or deletion",
)
require(
    "shutil.rmtree" not in finalizer and "os.remove(INBOX" not in finalizer,
    "finalizer must not recursively delete the inbox",
)
require(
    'relative(path)' in finalizer and 'sha256' in finalizer,
    "cleanup report must preserve exact path/hash evidence",
)
require(
    'prepared_status not in {"PASS", "NO_INBOX"}' in finalizer,
    "Fab-only/no-inbox projects must not be blocked from zero-ZIP cleanup after full acceptance",
)

for marker in (
    "FINAL ASSET ACCEPTANCE",
    "успішного ПОВНОГО RUNTIME-ТЕСТУ",
    "власними очима перевірили assets",
    "choice /C YN",
    "--accept-visual",
    "Непідтверджені ZIP не видаляються",
):
    require(marker in entrypoint, f"manual finalization entrypoint lost marker: {marker}")

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
):
    require(marker in collector, f"collector lost finalization marker: {marker}")

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
print("- direct visual PASS requires explicit human confirmation after exact-remote current automated PASS")
print("- M16/M4 content gap must be closed by a fresh bound payload before 100% finalization")
print("- Fab-only/no-inbox runs can finalize with zero source ZIPs after the automated gates pass")
print("- manual/cleanup records are exact-source scoped and fresh ingest invalidates them")
print("- source ZIP deletion is limited to models_game_OC archives whose SHA-256 is proven by prepared_sources")
print("- unknown/unproven ZIP blocks cleanup before any archive is deleted")
print("- consolidated LOCAL_ASSET_STATUS exposes visual acceptance and source ZIP cleanup separately")
