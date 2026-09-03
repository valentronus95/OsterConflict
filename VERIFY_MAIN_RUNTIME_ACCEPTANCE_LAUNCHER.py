from pathlib import Path

ROOT = Path(__file__).resolve().parent
MAIN = ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd"
PLAYFLOW = ROOT / "RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd"
STRICT = ROOT / "RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd"
MATERIAL = ROOT / "OsterConflict" / "RUN_PASS45_STRICT_MATERIAL_GATE.cmd"
EVIDENCE = ROOT / "VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py"
PASS8 = ROOT / "VERIFY_RUNTIME_RECONCILE_PASS_8.py"

for path in (MAIN, PLAYFLOW, STRICT, MATERIAL, EVIDENCE, PASS8):
    if not path.is_file():
        raise SystemExit(f"MAIN RUNTIME ACCEPTANCE LAUNCHER FAIL: missing {path.relative_to(ROOT)}")

main = MAIN.read_text(encoding="utf-8")
playflow = PLAYFLOW.read_text(encoding="utf-8")
strict = STRICT.read_text(encoding="utf-8")
material = MATERIAL.read_text(encoding="utf-8")
evidence = EVIDENCE.read_text(encoding="utf-8")

required_main = (
    'if /I "%OC_FORCE_ACCEPTANCE%"=="1" set "IS_ACCEPTANCE=1"',
    'if "%IS_ACCEPTANCE%"=="1" if /I "%CURRENT_BRANCH%"=="main"',
    'VERIFY_RUNTIME_RECONCILE_PASS_8.py',
    'PASS7_PRODUCTION_VEHICLE_RUNTIME_FAIL',
    'PASS45_REQUIRED_AVAILABLE_WEAPON_RUNTIME_FAIL',
    'PASS44_WEAPON_RACK_AUTHORED_MATERIAL_GAP',
    'PASS7_PRODUCTION_VEHICLES_READY',
    'PASS45_REQUIRED_AVAILABLE_WEAPONS_READY',
    'PASS36_WEAPON_MATERIAL_AUDIT_READY',
    'PASS7_MUSEUM_BASES_READY',
    'git fetch origin "%FETCH_BRANCH%"',
    'git rev-parse "%REMOTE_REF%"',
    '-fullscreen',
    't.MaxFPS 60',
)
for marker in required_main:
    if marker not in main:
        raise SystemExit(f"MAIN RUNTIME ACCEPTANCE LAUNCHER FAIL: missing main marker {marker!r}")
for stale in ('PASS7_PRODUCTION_WEAPONS_READY', 'PASS7_PRODUCTION_WEAPON_RUNTIME_FAIL'):
    if stale in main:
        raise SystemExit(f"MAIN RUNTIME ACCEPTANCE LAUNCHER FAIL: obsolete exact-only weapon marker returned {stale!r}")

# The strict wrapper must add post-playtest material/evidence gates without creating a second game launch.
required_strict = (
    'set "OC_FORCE_ACCEPTANCE=1"',
    'set "PLAYFLOW=%~dp0RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd"',
    'where git >nul 2>nul',
    "git rev-parse --verify HEAD",
    'set "PASS45_SOURCE_SHA_AFTER="',
    'if /I not "%PASS45_SOURCE_SHA_AFTER%"=="%PASS45_SOURCE_SHA%"',
    'source HEAD changed during runtime acceptance',
    'call "%PLAYFLOW%"',
    'call "%MATERIAL_GATE%"',
    'VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py',
    'if not "%RC%"=="0"',
    'PASS45 AUTOMATED RUNTIME EVIDENCE GATES PASSED',
    'VISUAL ACCEPTANCE IS STILL PENDING',
)
for marker in required_strict:
    if marker not in strict:
        raise SystemExit(f"MAIN RUNTIME ACCEPTANCE LAUNCHER FAIL: missing strict wrapper marker {marker!r}")

if 'PASS45_SOURCE_SHA=unknown' in strict:
    raise SystemExit("MAIN RUNTIME ACCEPTANCE LAUNCHER FAIL: strict wrapper still permits unbound SOURCE_SHA=unknown evidence")

if 'call "%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"' not in playflow:
    raise SystemExit("MAIN RUNTIME ACCEPTANCE LAUNCHER FAIL: playflow wrapper no longer owns the single CURRENT_GAMEPLAY delegation")

for marker in (
    'PASS45_REQUIRED_AVAILABLE_WEAPONS=PASS',
    'PASS45_AUTHORED_WEAPON_MATERIALS=PASS',
    'PASS45_WEAPON_DEPENDENCY_REPORT=PASS',
    'PASS45_EXACT_PRODUCTION_CONTENT_GAPS=',
    'PASS45_REQUIRED_AVAILABLE_WEAPON_VISUALS_VALIDATED_READY',
    'PASS45_PRODUCTION_VEHICLE_VISUALS_VALIDATED_READY',
):
    if marker not in material:
        raise SystemExit(f"MAIN RUNTIME ACCEPTANCE LAUNCHER FAIL: missing material gate marker {marker!r}")
if 'R14_PRODUCTION_WEAPONS=PASS' in material:
    raise SystemExit("MAIN RUNTIME ACCEPTANCE LAUNCHER FAIL: strict material gate resurrected impossible all-exact weapon readiness")

for marker in (
    'PASS45_REQUIRED_AVAILABLE_WEAPONS_READY',
    'PASS36_WEAPON_MATERIAL_AUDIT_READY',
    'PASS45_VEHICLE_ENTER_TRANSFORM_READY',
    'PASS45_VEHICLE_EXIT_TRANSFORM_READY',
    'PASS45_M2_GUNNER_PITCH_CONTRACT_READY',
    'PASS45_GUNNER_EXIT_TRANSFORM_READY',
    'textureDependency=PASS',
    'VISUAL_ACCEPTANCE=PENDING_MANUAL_OBSERVATION',
    'SOURCE_SHA=',
):
    if marker not in evidence:
        raise SystemExit(f"MAIN RUNTIME ACCEPTANCE LAUNCHER FAIL: missing evidence requirement {marker!r}")

if 'set "OC_FORCE_ACCEPTANCE=0"' in strict:
    raise SystemExit("MAIN RUNTIME ACCEPTANCE LAUNCHER FAIL: strict wrapper disables its own acceptance flag")
if 'start /wait' in strict or 'start /wait' in playflow:
    raise SystemExit("MAIN RUNTIME ACCEPTANCE LAUNCHER FAIL: wrapper became a second gameplay launcher")
if 'start /wait' not in main:
    raise SystemExit("MAIN RUNTIME ACCEPTANCE LAUNCHER FAIL: CURRENT_GAMEPLAY lost ownership of the one gameplay process")

print("MAIN RUNTIME ACCEPTANCE LAUNCHER + PASS45 REQUIRED-AVAILABLE CONTRACT PASS")
print("- CURRENT_GAMEPLAY remains the single gameplay process owner")
print("- strict main wrapper pins Git HEAD before runtime and rejects a source-head change before evidence verification")
print("- strict main wrapper delegates through playflow, then runs material/dependency and interaction evidence gates")
print("- exact weapon payload gaps stay CONTENT GAP; required available visuals and materials remain mandatory")
print("- driver enter/exit and M2 gunner pitch/exit are mandatory Pass45 regression evidence")
print("- automated evidence cannot promote visual acceptance beyond PENDING")
print("STATUS: SOURCE VERIFIED ONLY; local Windows UE 5.8 execution is still required")
