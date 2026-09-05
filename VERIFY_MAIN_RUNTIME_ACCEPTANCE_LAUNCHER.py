from pathlib import Path

ROOT = Path(__file__).resolve().parent
MAIN = ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd"
ENTRY = ROOT / "START_HERE.cmd"
BATCH_CMD = ROOT / "RUN_PASS45_BATCH_RUNTIME_TEST.cmd"
BATCH_PS1 = ROOT / "OsterConflict" / "Scripts" / "run_pass45_batch_runtime_test.ps1"
MATERIAL = ROOT / "OsterConflict" / "RUN_PASS45_STRICT_MATERIAL_GATE.cmd"
EVIDENCE = ROOT / "VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py"
PASS8 = ROOT / "VERIFY_RUNTIME_RECONCILE_PASS_8.py"

for path in (MAIN, ENTRY, BATCH_CMD, BATCH_PS1, MATERIAL, EVIDENCE, PASS8):
    if not path.is_file():
        raise SystemExit(f"MAIN RUNTIME ACCEPTANCE LAUNCHER FAIL: missing {path.relative_to(ROOT)}")

FORBIDDEN_MANUAL_WRAPPERS = (
    "RUN_CLEAN_FULL_TEST.cmd", "RUN_PC_TEST.cmd", "RUN_R11_LISTEN_TEST.cmd",
    "RUN_LOCAL_GAME_AFTER_BUILD.cmd", "RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd",
    "RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd", "RUN_R14_FOLIAGE_RUNTIME_ACCEPTANCE.cmd",
    "RUN_R14_MAIN_SANDBOX_TEST.cmd", "RUN_R14_ROAD_PROFILE_RUNTIME_ACCEPTANCE.cmd",
    "RUN_R14_STADION_RUNTIME_ACCEPTANCE.cmd", "RUN_R14_WORLD_STABILITY_RUNTIME_ACCEPTANCE.cmd",
    "RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd", "RUN_R17_RUNTIME_PERFORMANCE_ACCEPTANCE.cmd",
    "RUN_R21_LANDMARK_OWNERSHIP_RUNTIME_ACCEPTANCE.cmd", "START_MUSEUM_OSTER.cmd",
    "VALIDATE_SILPO_UE58.cmd", "OsterConflict/TRY_PRODUCTION_VEHICLES_UE58.cmd",
    "OsterConflict/INGEST_UPLOADED_MODELS_AND_IMPORT.cmd", "FINALIZE_ASSET_ACCEPTANCE_AND_CLEANUP.cmd",
)
for rel in FORBIDDEN_MANUAL_WRAPPERS:
    if (ROOT / rel).exists():
        raise SystemExit(f"MAIN RUNTIME ACCEPTANCE LAUNCHER FAIL: obsolete/redundant manual wrapper returned: {rel}")

main = MAIN.read_text(encoding="utf-8")
entry = ENTRY.read_text(encoding="utf-8")
batch_cmd = BATCH_CMD.read_text(encoding="utf-8")
batch = BATCH_PS1.read_text(encoding="utf-8")
material = MATERIAL.read_text(encoding="utf-8")
evidence = EVIDENCE.read_text(encoding="utf-8")

for marker in (
    'if /I "%OC_FORCE_ACCEPTANCE%"=="1" set "IS_ACCEPTANCE=1"',
    'VERIFY_RUNTIME_RECONCILE_PASS_8.py', 'PASS7_PRODUCTION_VEHICLE_RUNTIME_FAIL',
    'PASS7_PRODUCTION_WEAPON_RUNTIME_FAIL', 'PASS7_PRODUCTION_VEHICLES_READY',
    'PASS7_PRODUCTION_WEAPONS_READY', 'PASS7_MUSEUM_BASES_READY',
    'git fetch origin "%FETCH_BRANCH%"', 'git rev-parse "%REMOTE_REF%"', '-fullscreen', 't.MaxFPS 60',
):
    if marker not in main:
        raise SystemExit(f"MAIN RUNTIME ACCEPTANCE LAUNCHER FAIL: missing gameplay marker {marker!r}")

for marker in (
    'Єдиний користувацький launcher/test entrypoint: START_HERE.cmd.',
    '2. ПОВНИЙ RUNTIME-ТЕСТ ^(ПАКЕТНИЙ^)',
    'set "BATCH_RUNTIME=%~dp0RUN_PASS45_BATCH_RUNTIME_TEST.cmd"',
    'call "%BATCH_RUNTIME%"',
    'set "OC_FORCE_ACCEPTANCE=0"',
    'call "%CURRENT_GAMEPLAY%"',
):
    if marker not in entry:
        raise SystemExit(f"MAIN RUNTIME ACCEPTANCE LAUNCHER FAIL: missing START_HERE marker {marker!r}")

if ':full_runtime_test' in entry:
    raise SystemExit("MAIN RUNTIME ACCEPTANCE LAUNCHER FAIL: stale in-file full-runtime owner still exists")
if 'PASS45_REIMPORT_STEIN_WEAPON_MATERIALS_UE58.cmd' in entry or 'PASS45_IMPORT_REMINGTON870_PRODUCTION_UE58.cmd' in entry:
    raise SystemExit("MAIN RUNTIME ACCEPTANCE LAUNCHER FAIL: stale per-weapon preflight leaked into START_HERE")

for marker in (
    'powershell -NoProfile -ExecutionPolicy Bypass -File "%RUNNER%"',
    'IMPORT_ALL_LOCAL_INBOX_UE58.cmd', 'RUN_PASS45_STRICT_MATERIAL_GATE.cmd',
    'RUN_R14_CURRENT_GAMEPLAY.cmd', 'VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py',
    'finalize_asset_acceptance.py', 'PASS45_BATCH_RUNTIME_REPORT.txt',
    'Tracked Changes:', 'FORMAL_ACCEPTANCE=BLOCKED_DIRTY_OR_NONEXACT_SOURCE',
    '$env:OC_FORCE_ACCEPTANCE = "0"', '$env:OC_FORCE_ACCEPTANCE = "1"',
    'Read-Host "Ви реально оглянули assets і приймаєте їх візуальний стан? [Y/N]"',
):
    haystack = batch_cmd + "\n" + batch
    if marker not in haystack:
        raise SystemExit(f"MAIN RUNTIME ACCEPTANCE LAUNCHER FAIL: batch runtime lost {marker!r}")

for stale in (
    'PASS45_REIMPORT_STEIN_WEAPON_MATERIALS_UE58.cmd',
    'PASS45_IMPORT_MANUAL_ACTION_AUDIO_UE58.cmd',
    'PASS45_IMPORT_REMINGTON870_PRODUCTION_UE58.cmd',
):
    if stale in batch:
        raise SystemExit(f"MAIN RUNTIME ACCEPTANCE LAUNCHER FAIL: stale batch stage returned: {stale}")

for marker in ('PASS45_AUTHORED_WEAPON_MATERIALS=PASS', 'PASS45_WEAPON_DEPENDENCY_REPORT=PASS', 'PASS45_PRODUCTION_VEHICLE_VISUALS_VALIDATED_READY'):
    if marker not in material:
        raise SystemExit(f"MAIN RUNTIME ACCEPTANCE LAUNCHER FAIL: missing material gate marker {marker!r}")
for marker in ('PASS45_VEHICLE_ENTER_TRANSFORM_READY', 'PASS45_VEHICLE_EXIT_TRANSFORM_READY', 'PASS45_M2_GUNNER_PITCH_CONTRACT_READY', 'PASS45_GUNNER_EXIT_TRANSFORM_READY', 'PASS14_PERF_30FPS_READY', 'VISUAL_ACCEPTANCE=PENDING_MANUAL_OBSERVATION'):
    if marker not in evidence:
        raise SystemExit(f"MAIN RUNTIME ACCEPTANCE LAUNCHER FAIL: missing evidence requirement {marker!r}")

print("MAIN RUNTIME ACCEPTANCE LAUNCHER SOURCE CONTRACT PASS")
print("- START_HERE.cmd remains the only user-facing entrypoint")
print("- option 2 delegates to one internal batch orchestrator with one report")
print("- dirty/non-exact local source can run diagnostic gameplay but can never become formal acceptance")
print("- obsolete per-weapon preflight wrappers are not part of the canonical runtime path")
print("STATUS: SOURCE VERIFIED ONLY; local Windows UE 5.8 execution is still required")
