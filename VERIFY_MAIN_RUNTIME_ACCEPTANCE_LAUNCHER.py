from pathlib import Path

ROOT = Path(__file__).resolve().parent
MAIN = ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd"
ENTRY = ROOT / "START_HERE.cmd"
MATERIAL = ROOT / "OsterConflict" / "RUN_PASS45_STRICT_MATERIAL_GATE.cmd"
EVIDENCE = ROOT / "VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py"
PASS8 = ROOT / "VERIFY_RUNTIME_RECONCILE_PASS_8.py"

for path in (MAIN, ENTRY, MATERIAL, EVIDENCE, PASS8):
    if not path.is_file():
        raise SystemExit(f"MAIN RUNTIME ACCEPTANCE LAUNCHER FAIL: missing {path.relative_to(ROOT)}")

FORBIDDEN_MANUAL_WRAPPERS = (
    "RUN_CLEAN_FULL_TEST.cmd",
    "RUN_PC_TEST.cmd",
    "RUN_R11_LISTEN_TEST.cmd",
    "RUN_LOCAL_GAME_AFTER_BUILD.cmd",
    "RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd",
    "RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd",
    "RUN_R14_FOLIAGE_RUNTIME_ACCEPTANCE.cmd",
    "RUN_R14_MAIN_SANDBOX_TEST.cmd",
    "RUN_R14_ROAD_PROFILE_RUNTIME_ACCEPTANCE.cmd",
    "RUN_R14_STADION_RUNTIME_ACCEPTANCE.cmd",
    "RUN_R14_WORLD_STABILITY_RUNTIME_ACCEPTANCE.cmd",
    "RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd",
    "RUN_R17_RUNTIME_PERFORMANCE_ACCEPTANCE.cmd",
    "RUN_R21_LANDMARK_OWNERSHIP_RUNTIME_ACCEPTANCE.cmd",
    "START_MUSEUM_OSTER.cmd",
    "VALIDATE_SILPO_UE58.cmd",
    "OsterConflict/TRY_PRODUCTION_VEHICLES_UE58.cmd",
    "OsterConflict/INGEST_UPLOADED_MODELS_AND_IMPORT.cmd",
    "FINALIZE_ASSET_ACCEPTANCE_AND_CLEANUP.cmd",
)
for rel in FORBIDDEN_MANUAL_WRAPPERS:
    if (ROOT / rel).exists():
        raise SystemExit(f"MAIN RUNTIME ACCEPTANCE LAUNCHER FAIL: obsolete/redundant manual wrapper returned: {rel}")

main = MAIN.read_text(encoding="utf-8")
entry = ENTRY.read_text(encoding="utf-8")
material = MATERIAL.read_text(encoding="utf-8")
evidence = EVIDENCE.read_text(encoding="utf-8")

for marker in (
    'if /I "%OC_FORCE_ACCEPTANCE%"=="1" set "IS_ACCEPTANCE=1"',
    'VERIFY_RUNTIME_RECONCILE_PASS_8.py',
    'PASS7_PRODUCTION_VEHICLE_RUNTIME_FAIL',
    'PASS7_PRODUCTION_WEAPON_RUNTIME_FAIL',
    'PASS7_PRODUCTION_VEHICLES_READY',
    'PASS7_PRODUCTION_WEAPONS_READY',
    'PASS7_MUSEUM_BASES_READY',
    'git fetch origin "%FETCH_BRANCH%"',
    'git rev-parse "%REMOTE_REF%"',
    '-fullscreen',
    't.MaxFPS 60',
):
    if marker not in main:
        raise SystemExit(f"MAIN RUNTIME ACCEPTANCE LAUNCHER FAIL: missing gameplay marker {marker!r}")

for marker in (
    'Єдиний користувацький launcher/test entrypoint: START_HERE.cmd.',
    'set "OC_FORCE_ACCEPTANCE=1"',
    'call "%CURRENT_GAMEPLAY%"',
    'call "%MATERIAL_GATE%"',
    'VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py',
    'PASS45 AUTOMATED RUNTIME EVIDENCE GATES PASSED',
    'set "ASSET_FINALIZER=%~dp0OsterConflict\\Scripts\\finalize_asset_acceptance.py"',
    '"%ASSET_FINALIZER%" --preflight',
    'FINALIZE PENDING',
    'Visual acceptance не записано',
    'choice /C YN',
    '"%ASSET_FINALIZER%" --accept-visual',
):
    if marker not in entry:
        raise SystemExit(f"MAIN RUNTIME ACCEPTANCE LAUNCHER FAIL: missing START_HERE marker {marker!r}")

preflight_pos = entry.find('"%ASSET_FINALIZER%" --preflight')
choice_pos = entry.find('choice /C YN', preflight_pos)
manual_pos = entry.find('"%ASSET_FINALIZER%" --accept-visual', choice_pos)
if -1 in (preflight_pos, choice_pos, manual_pos) or not preflight_pos < choice_pos < manual_pos:
    raise SystemExit("MAIN RUNTIME ACCEPTANCE LAUNCHER FAIL: manual visual acceptance is not gated after automated preflight")

if 'TRY_PRODUCTION_VEHICLES_UE58.cmd' in entry:
    raise SystemExit("MAIN RUNTIME ACCEPTANCE LAUNCHER FAIL: normal launcher still runs obsolete TRY vehicle wrapper")
if 'RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd' in entry or 'RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd' in entry:
    raise SystemExit("MAIN RUNTIME ACCEPTANCE LAUNCHER FAIL: START_HERE still delegates to redundant acceptance wrapper")

for marker in (
    'PASS45_AUTHORED_WEAPON_MATERIALS=PASS',
    'PASS45_WEAPON_DEPENDENCY_REPORT=PASS',
    'PASS45_PRODUCTION_VEHICLE_VISUALS_VALIDATED_READY',
):
    if marker not in material:
        raise SystemExit(f"MAIN RUNTIME ACCEPTANCE LAUNCHER FAIL: missing material gate marker {marker!r}")

for marker in (
    'PASS45_VEHICLE_ENTER_TRANSFORM_READY',
    'PASS45_VEHICLE_EXIT_TRANSFORM_READY',
    'PASS45_M2_GUNNER_PITCH_CONTRACT_READY',
    'PASS45_GUNNER_EXIT_TRANSFORM_READY',
    'PASS14_PERF_30FPS_READY',
    'VISUAL_ACCEPTANCE=PENDING_MANUAL_OBSERVATION',
):
    if marker not in evidence:
        raise SystemExit(f"MAIN RUNTIME ACCEPTANCE LAUNCHER FAIL: missing evidence requirement {marker!r}")

print("MAIN RUNTIME ACCEPTANCE LAUNCHER SOURCE CONTRACT PASS")
print("- START_HERE.cmd is the only user-facing launcher/test entrypoint")
print("- obsolete manual runtime/test wrappers are physically absent")
print("- RUN_R14_CURRENT_GAMEPLAY.cmd remains the single internal gameplay execution route")
print("- automated evidence preserves visual=PENDING; manual visual PASS is reachable only after non-destructive finalization preflight and explicit Y/N confirmation")
print("- strict material, interaction and performance evidence is verified without extra acceptance wrappers")
print("STATUS: SOURCE VERIFIED ONLY; local Windows UE 5.8 execution is still required")
