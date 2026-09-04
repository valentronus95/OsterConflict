#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
IMPORT_CMD = ROOT / "OsterConflict" / "IMPORT_ALL_LOCAL_INBOX_UE58.cmd"
COLLECTOR = ROOT / "COLLECT_LOCAL_ASSET_STATUS.py"

errors = []

if not IMPORT_CMD.is_file():
    errors.append("missing OsterConflict/IMPORT_ALL_LOCAL_INBOX_UE58.cmd")
    text = ""
else:
    text = IMPORT_CMD.read_text(encoding="utf-8", errors="replace")

if not COLLECTOR.is_file():
    errors.append("missing COLLECT_LOCAL_ASSET_STATUS.py")
    collector = ""
else:
    collector = COLLECTOR.read_text(encoding="utf-8", errors="replace")

required = (
    'set "DIRTY_ACCEPTANCE_SOURCE="',
    'git -C "%REPO_ROOT%" status --porcelain --untracked-files=no --',
    'START_HERE.cmd',
    'RUN_R14_CURRENT_GAMEPLAY.cmd',
    'COLLECT_LOCAL_ASSET_STATUS.py',
    'VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py',
    'OsterConflict/IMPORT_ALL_LOCAL_INBOX_UE58.cmd',
    'OsterConflict/IMPORT_PRODUCTION_VEHICLES_UE58.cmd',
    'OsterConflict/RUN_PASS45_STRICT_MATERIAL_GATE.cmd',
    'OsterConflict/Scripts',
    'OsterConflict/Source',
    '[LOCAL SOURCE CHANGE]',
    'if defined DIRTY_ACCEPTANCE_SOURCE',
    'exit /b 59',
)
for marker in required:
    if marker not in text:
        errors.append(f"dirty-source guard lost marker: {marker}")

if text:
    try:
        guard_pos = text.index('git -C "%REPO_ROOT%" status --porcelain --untracked-files=no --')
        evidence_delete_pos = text.index('if exist "%SUCCESS%" del /q "%SUCCESS%"')
        lfs_pos = text.index('git -C "%REPO_ROOT%" lfs pull origin "%CURRENT_BRANCH%"')
        if not guard_pos < evidence_delete_pos < lfs_pos:
            errors.append("dirty-source guard must run before evidence cleanup and before Git LFS/UE ingest")
    except ValueError:
        pass

    if '--untracked-files=no' not in text:
        errors.append("guard must not block untracked/local model payloads")
    if 'OsterConflict/Content' in text[text.find('status --porcelain'):text.find('status --porcelain') + 800]:
        errors.append("guard must not treat Content payloads as dirty runtime source")

if collector:
    for marker in (
        'if import_result is None:',
        'import_stage = "PENDING_CURRENT_RUN"',
        'elif import_result != 0:',
        'import_stage = "FAIL"',
        'import_stage = "PASS"',
    ):
        if marker not in collector:
            errors.append(f"collector import freshness gate lost marker: {marker}")
    try:
        unknown_pos = collector.index('if import_result is None:')
        pending_pos = collector.index('import_stage = "PENDING_CURRENT_RUN"', unknown_pos)
        fail_pos = collector.index('elif import_result != 0:', pending_pos)
        pass_pos = collector.index('import_stage = "PASS"', fail_pos)
        if not unknown_pos < pending_pos < fail_pos < pass_pos:
            errors.append("collector may promote import PASS without an explicit current import result")
    except ValueError:
        pass

if errors:
    print("PASS45 ASSET SOURCE/FRESHNESS GUARD: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 ASSET SOURCE/FRESHNESS GUARD: PASS")
print("- tracked runtime/source edits are rejected before evidence cleanup, LFS hydration, build, or UE import")
print("- untracked/local model payloads and Content remain allowed for asset intake")
print("- acceptance evidence cannot attribute locally edited runtime source to a clean GitHub HEAD")
print("- LOCAL_UE_IMPORT cannot become PASS without an explicit current import result code of zero")
