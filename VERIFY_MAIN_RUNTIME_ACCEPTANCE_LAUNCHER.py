from pathlib import Path

ROOT = Path(__file__).resolve().parent
MAIN = ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd"
STRICT = ROOT / "RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd"
MATERIAL = ROOT / "OsterConflict" / "RUN_PASS45_STRICT_MATERIAL_GATE.cmd"
EVIDENCE = ROOT / "VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py"
PASS8 = ROOT / "VERIFY_RUNTIME_RECONCILE_PASS_8.py"

for path in (MAIN, STRICT, MATERIAL, EVIDENCE, PASS8):
    if not path.is_file():
        raise SystemExit(f"MAIN RUNTIME ACCEPTANCE LAUNCHER FAIL: missing {path.relative_to(ROOT)}")

main = MAIN.read_text(encoding="utf-8")
strict = STRICT.read_text(encoding="utf-8")
material = MATERIAL.read_text(encoding="utf-8")
evidence = EVIDENCE.read_text(encoding="utf-8")

required_main = (
    'if /I "%OC_FORCE_ACCEPTANCE%"=="1" set "IS_ACCEPTANCE=1"',
    'if "%IS_ACCEPTANCE%"=="1" if /I "%CURRENT_BRANCH%"=="main"',
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
)
for marker in required_main:
    if marker not in main:
        raise SystemExit(f"MAIN RUNTIME ACCEPTANCE LAUNCHER FAIL: missing main marker {marker!r}")

required_strict = (
    'set "OC_FORCE_ACCEPTANCE=1"',
    'set "CURRENT_GAMEPLAY=%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"',
    'call "%CURRENT_GAMEPLAY%"',
    'call "%MATERIAL_GATE%"',
    'VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py',
    'if not "%RC%"=="0"',
    'PASS45 AUTOMATED RUNTIME EVIDENCE GATES PASSED',
    'VISUAL ACCEPTANCE IS STILL PENDING',
)
for marker in required_strict:
    if marker not in strict:
        raise SystemExit(f"MAIN RUNTIME ACCEPTANCE LAUNCHER FAIL: missing strict wrapper marker {marker!r}")

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
    'VISUAL_ACCEPTANCE=PENDING_MANUAL_OBSERVATION',
):
    if marker not in evidence:
        raise SystemExit(f"MAIN RUNTIME ACCEPTANCE LAUNCHER FAIL: missing evidence requirement {marker!r}")

if 'set "OC_FORCE_ACCEPTANCE=0"' in strict:
    raise SystemExit("MAIN RUNTIME ACCEPTANCE LAUNCHER FAIL: strict wrapper disables its own acceptance flag")
if 'start /wait' in strict:
    raise SystemExit("MAIN RUNTIME ACCEPTANCE LAUNCHER FAIL: strict wrapper became a second gameplay launcher")

print("MAIN RUNTIME ACCEPTANCE LAUNCHER SOURCE CONTRACT PASS")
print("- RUN_R14_CURRENT_GAMEPLAY.cmd remains the single normal gameplay launcher")
print("- strict wrapper forces current-source gameplay, then Pass45 material/dependency and interaction evidence gates")
print("- Museum BASE + production weapons + production vehicles remain mandatory baseline runtime markers")
print("- driver enter/exit and M2 gunner pitch/exit are mandatory Pass45 regression evidence")
print("- automated evidence cannot promote visual acceptance beyond PENDING")
print("STATUS: SOURCE VERIFIED ONLY; local Windows UE 5.8 execution is still required")
