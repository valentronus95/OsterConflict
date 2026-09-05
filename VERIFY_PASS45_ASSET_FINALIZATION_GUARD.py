#!/usr/bin/env python3
from pass45_runtime_route_contract import ROOT, read, require, forbid, validate_runtime_route

route = validate_runtime_route()
finalizer = route["finalizer"]
batch = route["batch"]
prepare = read("OsterConflict/Scripts/prepare_all_local_inbox_assets.ps1")
base_importer = read("OsterConflict/Scripts/import_all_local_inbox_assets.py")
normalizer = read("OsterConflict/Scripts/normalize_local_weapon_categories.py")
collector = read("COLLECT_LOCAL_ASSET_STATUS.py")
import_cmd = route["all_assets"]

for marker in (
    'preflight_only = "--preflight" in args', 'accept_visual = "--accept-visual" in args',
    'preflight_only == accept_visual', 'git_output("rev-parse", "HEAD")', "verify_exact_remote_head(head)",
    "verify_clean_acceptance_source()", "verify_current_automated_status(current, head)",
    'status.get("schema") != "oster-conflict-local-asset-status-v4"', 'status.get("import_result_code") != 0',
    'status.get("runtime_result_code") != 0', 'status.get("runtime_scope") != "CURRENT_RUN_COMPLETED"',
    'production.get("vehicles_status") != "PASS"', 'production.get("weapons_status") != "PASS"',
    'bindings.get("all_models_bound")', 'source_status_counts.get("UNBOUND")', 'category_counts.get("M16_M4")',
    "VISUAL_CHECKLIST", "write_manual_acceptance(head)", "path.unlink()", 'write_cleanup_report(head, "PASS", deleted)',
):
    require(finalizer, marker, "finalizer fail-closed contract")
for marker in ('if ($item.depth -gt 4)', "status='NESTED_DEPTH_LIMIT'", "error='nested_zip_depth_limit_exceeded'", "UNSAFE_ARCHIVE_PRESENT", "exit 40"):
    require(prepare, marker, "nested ZIP guard")
for marker in ("Fail closed on every source row", 'bindings["unbound_models"].append(status)', 'bindings["all_models_bound"] = len(bindings["unbound_models"]) == 0'):
    require(base_importer, marker, "explicit UNBOUND guard")
for marker in ("A factual import/load failure", 'if row_status == "UNBOUND":', "Independent reconciliation: every source_status row still marked UNBOUND", 'data["all_models_bound"] = len(retained) == 0'):
    require(normalizer, marker, "weapon UNBOUND guard")
for marker in ('"oster-conflict-local-asset-status-v4"', '"direct_visual_acceptance": visual_stage', '"source_zip_cleanup": cleanup_stage', 'explicit_unbound_count = int(source_status_counts.get("UNBOUND", 0))', 'evidence_source_match = source_sha_known'):
    require(collector, marker, "collector finalization truth")
for marker in ("MANUAL_VISUAL_ACCEPTANCE.json", "ACCEPTED_ZIP_CLEANUP.json", "Any fresh ingest invalidates earlier manual visual acceptance"):
    require(import_cmd, marker, "fresh ingest invalidation")
for marker in ('@($Finalizer, "--preflight")', 'Read-Host "Ви реально оглянули assets і приймаєте їх візуальний стан? [Y/N]"', '@($Finalizer, "--accept-visual")', "PASS45 FULL ASSET LIFECYCLE ACCEPTED."):
    require(batch, marker, "packet finalization boundary")
pre = batch.find('@($Finalizer, "--preflight")')
ask = batch.find('Read-Host "Ви реально оглянули assets і приймаєте їх візуальний стан? [Y/N]"', pre)
accept = batch.find('@($Finalizer, "--accept-visual")', ask)
if min(pre, ask, accept) < 0 or not pre < ask < accept:
    raise SystemExit("PASS45 ASSET FINALIZATION GUARD: FAIL - manual acceptance order regressed")
forbidden = ROOT / "FINALIZE_ASSET_ACCEPTANCE_AND_CLEANUP.cmd"
if forbidden.exists():
    raise SystemExit("PASS45 ASSET FINALIZATION GUARD: FAIL - second user-facing finalizer returned")
forbid(finalizer, "shutil.rmtree", "finalizer recursive cleanup")

print("PASS45 ASSET FINALIZATION GUARD: PASS")
print("- packet runner owns preflight -> explicit human observation -> accept-visual order")
print("- UNBOUND/schema/source/result/hash cleanup gates remain fail-closed")
print("- fresh ingest invalidates stale visual/cleanup evidence")
